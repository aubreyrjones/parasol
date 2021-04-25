from typing import *
from graphviz import Digraph, nohtml
import tempfile
import os.path as path
import subprocess

_id_counter = -1

def _next_id():
    global _id_counter
    _id_counter += 1
    return _id_counter

def _add_type_node(g: Digraph, this):
    if 'T' not in this: return
    typecode = this['T'] or ('?', None)
    nodeid = str(this.id) + "_T"
    if typecode[1]:
        g.node(nodeid, f'{typecode[0]}@{typecode[1]}', shape='ellipse')
    else:
        g.node(nodeid, f'{typecode[0]}', shape='ellipse')
    g.edge(nodeid, str(this.id))

class ASTNode:
    def __init__(self, line: int = -1):
        self.id = _next_id()
        self.line = line
        self.attr = {}
        self.parent = None

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
        for c in self.items:
            c.dot(g)
            g.edge(str(self.id), str(c.id))

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
        for c in self.items:
            c.dot(g)
            g.edge(str(self.id), str(c.id))


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

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) fncall | {self.refname} | <f0> (...)'))
        for a in self.args:
            a.dot(g)
            g.edge(f'{str(self.id)}:<f0>', str(a.id))
        _add_type_node(g, self)

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

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) {type(self).NAME}'))
        g.edge(str(self.id), str(self.operand.id))
        _add_type_node(g, self)


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
    
    def dot(self, g):
        g.node(str(self.id), nohtml(f'<f0> | {self.line}) {type(self).NAME} | <f1>'))
        self.left.dot(g)
        self.right.dot(g)
        g.edge(f'{str(self.id)}:<f0>', str(self.left.id))
        g.edge(f'{str(self.id)}:<f1>', str(self.right.id))
        _add_type_node(g, self)

def make_binop(opname: str):
    class Ret(BinaryOp):
        NAME = opname
        def __init__(self, left: ASTNode, right: ASTNode, line):
            super().__init__(left, right, line)
    return Ret

Binops = {}
_binops = 'Gets Add Sub Mult Div Dot'.split()
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

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{self.line}) vardecl | {self.stage or "*"} | {self.name} | <f0> {"T" if self.typeref else "?"} | {str(self.index or "?")}'))
        if self.typeref:
            self.typeref.dot(g)
            g.edge(f'{str(self.id)}:<f0>', str(self.typeref.id))
        _add_type_node(g, self)

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

Literals = {
    'INTEGER' : Integer,
    'FLOAT'   : Float,
}


def visualize_ast(root: ASTNode):
    '''
    Visualize an AST using DOT.
    '''
    g = Digraph('g', node_attr = {'shape' : 'record', 'style' : 'filled'})
    root.dot(g)
    with tempfile.TemporaryDirectory() as workdir:
        fn = path.join(workdir, '_ast.png')
        g.render(filename = fn, format='png')
        subprocess.call(["display", fn])
