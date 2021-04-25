from typing import *
from graphviz import Digraph, nohtml
import tempfile
import os.path as path
import subprocess
from itertools import chain
from Types import Variable, Function

_id_counter = -1

def _next_id():
    global _id_counter
    _id_counter += 1
    return _id_counter


def _add_type_node(g: Digraph, this):
    if 'T' not in this: return
    typecode = this['T'] or ('?', None)
    nodeid = str(this.id) + "_T"
    g.node(nodeid, f'{typecode[0]}{"@" + str(typecode[1]) if typecode[1] else ""}', shape='ellipse', color='#e6eeff', fontsize='8')
    g.edge(nodeid, str(this.id))

def typestr(typecode: tuple) -> str:
    if typecode is None: return '?'
    if typecode[1]:
        return f'{typecode[0]}@{typecode[1]}'
    return typecode[0]

class ASTNode:
    def __init__(self, line: int = -1):
        self.id = _next_id()
        self.line = line
        self.attr = {}
        self.parent = None
        self.scope = None

    def dot(self, g: Digraph):
        return g
    
    def akeys(self):
        return list(self.attr.keys())

    def __setitem__(self, k, v):
        self.attr[k] = v

    def __getitem__(self, k):
        if k not in self.attr: return None
        return self.attr[k]

    def __contains__(self, k):
        return k in self.attr
        
    def re_id(self):
        self.id = _next_id()

    def subs(self):
        return []


class TU(ASTNode):
    def __init__(self, line: int):
        super().__init__(line)
        self.globals = []

    def push(self, child: ASTNode):
        self.globals.append(child)
        child.parent = self

    def dot(self, g):
        g.node(str(self.id), f'{self.line}) globals')
        for c in self.globals:
            c.dot(g)
            g.edge(str(self.id), str(c.id))

    def re_id(self):
        super().re_id()
        for g in self.globals:
            g.re_id()
    
    def subs(self):
        return self.globals[:]

class Component(ASTNode):
    '''
    A non-realized set of definitions, declarations, and assignments.
    '''
    def __init__(self, name: str, line):
        super().__init__(line)
        self.name = name
        self.items = []
    
    def push(self, child: ASTNode):
        self.items.append(child)
        child.parent = self

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) component | {self.name}'))
        self.dot_items(g)

    def dot_items(self, g):
        #with g.subgraph(name = "cluster_" + self.scope.name, graph_attr = {'label' : self.scope.name}) as sg:
        for c in self.items:
            c.dot(g)
            g.edge(str(self.id), str(c.id))

    def re_id(self):
        super().re_id()
        for c in self.items:
            c.re_id()

    def subs(self):
        return self.items[:]

class Pipeline(Component):
    '''
    A Component that the user has declared is:
      * meant for export, and thus realization
      * complete for whatever interface contract they intend
    '''
    def __init__(self, name: str, line):
        super().__init__(name, line)

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) pipeline | {self.name}'))
        self.dot_items(g)


class TypeRef(ASTNode):
    '''
    A reference to a type, with optional array spec.
    '''
    def __init__(self, name: str, array_spec: Optional[int], line):
        super().__init__(line)
        self.name = name
        self.array_spec = array_spec
        pass

    def dot(self, g):
        if self.array_spec:
            g.node(str(self.id), nohtml(f'{self.line}) typeref | {self.name} @ {str(self.array_spec)}'))
        else:
            g.node(str(self.id), nohtml(f'{self.line}) typeref | {self.name}'))
        _add_type_node(g, self)
    
    def typecode(self):
        return (self.name, self.array_spec)

    def re_id(self):
        super().re_id()


class FnDef(ASTNode):
    def __init__(self, name: str, params, body, line):
        super().__init__(line)
        self.name = name
        self.params = params
        self.body = body

    def re_id(self):
        super().re_id()
        for p in self.params:
            p.re_id()
        if self.body: self.body.re_id()
    
    def param_names(self):
        return [p.name for p in self.params]

    def param_rep(self):
        return " | ".join(map(lambda p: typestr(p.typecode()), self.params))

    def dot(self, g, dot_reals = True):
        g.node(str(self.id), nohtml(f'{{{self.line}) {self.name}| {{{self.param_rep()}}} | <f0>}}'))
        self.body.dot(g)
        g.edge(str(self.id) + ":<f0>", str(self.body.id))
        if dot_reals:
            with g.subgraph(name = "Cluster_" + self.name, graph_attr = {'label' : self.name}) as sg:
                self.dot_reals(sg)

    def dot_reals(self, g):
        real = self.follow()
        for r in real.reals.values():
            r.dot(g, False)

    def subs(self):
        return chain(self.params, [self.body])

    def follow(self):
        if self.name not in self.scope or not isinstance(self.scope[self.name], Function):
            raise RuntimeError("Cannot find declaration for function: " + self.name)
        return self.scope[self.name]

class Expression(ASTNode):
    def __init__(self, line):
        super().__init__(line)


class FnCall(Expression):
    def __init__(self, ref: str, args: List[Expression], line):
        super().__init__(line)
        self.refname = ref
        self.args = args or []
        for a in self.args:
            a.parent = self

    def re_id(self):
        super().re_id()
        for a in self.args:
            a.re_id()

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) fncall | {self.refname} | <f0> (...)'))
        for a in self.args:
            a.dot(g)
            g.edge(f'{str(self.id)}:<f0>', str(a.id))
        _add_type_node(g, self)

    def subs(self):
        return self.args[:]

    def arg_typecodes(self):
        return [a['T'] for a in self.args]

    def follow(self):
        if self.refname not in self.scope or not isinstance(self.scope[self.refname], Function):
            raise RuntimeError("Cannot find declaration for function: " + self.refname)
        return self.scope[self.refname]


class Operation(Expression):
    '''
    An executable operation.
    '''
    NAME = 'unknown'
    def __init__(self, line):
        super().__init__(line)


class UnaryOp(Operation):
    '''
    An operation with a single operand.
    '''
    def __init__(self, operand: ASTNode, line):
        super().__init__(line)
        self.operand = operand
        self.operand.parent = self

    def re_id(self):
        super().re_id()
        self.operand.re_id()


    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) {type(self).NAME}'))
        g.edge(str(self.id), str(self.operand.id))
        _add_type_node(g, self)

    def subs(self):
        return [self.operand]


class BinaryOp(Operation):
    '''
    An operation with two operands.
    '''
    def __init__(self, left: ASTNode, right: ASTNode, line):
        super().__init__(line)
        self.left = left
        left.parent = self

        self.right = right
        right.parent = self

    def re_id(self):
        super().re_id()
        self.left.re_id()
        self.right.re_id()

    def dot(self, g):
        g.node(str(self.id), nohtml(f'<f0> | {self.line}) {type(self).NAME} | <f1>'))
        self.left.dot(g)
        self.right.dot(g)
        g.edge(f'{str(self.id)}:<f0>', str(self.left.id))
        g.edge(f'{str(self.id)}:<f1>', str(self.right.id))
        _add_type_node(g, self)

    def subs(self):
        return [self.left, self.right]

def make_binop(opname: str):
    class Ret(BinaryOp):
        NAME = opname
        def __init__(self, left: ASTNode, right: ASTNode, line):
            super().__init__(left, right, line)
    return Ret

Binops = {}
_binops = 'Gets Add Sub Mult MMult Div Dot'.split()
for b in _binops:
    newbin = make_binop(b)
    Binops[b] = newbin
    globals()[b] = newbin


def make_unop(opname: str):
    class Ret(UnaryOp):
        NAME = opname
        def __init__(self, operand: ASTNode, line):
            super().__init__(operand, line)
    return Ret

Unops = {}
_unops = ''.split()
for u in _unops:
    newun = make_unop(u)
    Unops[u] = newun
    globals()[u] = newun


class Terminal(Expression):
    '''
    Some kind of terminal operand.
    '''
    def __init__(self, value, line):
        super().__init__(line)
        self.value = value

    def dot(self, g):
        g.node(str(self.id), nohtml(f"{self.line}) Value | {self.value}"))
        _add_type_node(g, self)


class Number(Terminal):
    '''
    A numeric quantity.
    '''
    def __init__(self, value, line):
        super().__init__(value, line)


class Integer(Number):
    '''
    An integer quantity.
    '''
    def __init__(self, value: int, line):
        super().__init__(value, line)
        self.attr['T'] = ('int', None)


class Float(Number):
    '''
    A floating-point quantity.
    '''
    def __init__(self, value: float, line):
        super().__init__(value, line)
        self.attr['T'] = ('float', None)


class VarDecl(Expression):
    '''
    A variable declaration, staged or unstaged,
    '''
    def __init__(self, name: str, stageref: str, typeref: Optional[TypeRef], index: Optional[int], line):
        super().__init__(line)
        self.name: str = name
        self.stage: str = stageref
        self.typeref: Optional[TypeRef] = typeref
        if self.typeref:
            self.typeref.parent = self
        self.index: int = index
        pass

    def re_id(self):
        super().re_id()
        if self.typeref: self.typeref.re_id()

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) vardecl | {self.stage or "*"} | {self.name} | <f0> {"T" if self.typeref else "?"} | {str("?" if self.index is None else self.index)}'))
        if self.typeref:
            self.typeref.dot(g)
            g.edge(f'{str(self.id)}:<f0>', str(self.typeref.id))
        _add_type_node(g, self)

    def typecode(self):
        if 'T' in self: return self['T']
        if not self.typeref: return None
        return self.typeref.typecode()

    def follow(self):
        return self.scope[self.name]

class VarRef(Expression):
    '''
    A reference to a variable.
    '''
    def __init__(self, name: str, line):
        super().__init__(line)
        self.refname = name

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) var | {self.refname}'))
        _add_type_node(g, self)

    def follow(self):
        if self.refname not in self.scope or not isinstance(self.scope[self.refname], Variable):
            raise RuntimeError("Cannot find declaration for variable: " + self.refname)
        return self.scope[self.refname]


Literals = {
    'INTEGER' : Integer,
    'FLOAT'   : Float,
}


def visualize_ast(root: ASTNode):
    '''
    Visualize an AST using DOT.
    '''
    g = Digraph(node_attr = {'shape' : 'record', 'style' : 'filled'})
    
    root.dot(g)
    
    with tempfile.TemporaryDirectory() as workdir:
        fn = path.join(workdir, '_ast.png')
        g.render(filename = fn, format='png')
        subprocess.call(["display", fn])
