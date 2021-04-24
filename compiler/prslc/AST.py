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


class ASTNode:
    def __init__(self, line: int = -1):
        self.id = _next_id()
        self.line = line
        self.attr = {}
        self.parent = None

    def dot(self, g: Digraph):
        return g


class TU(ASTNode):
    def __init__(self):
        super().__init__(self)
        self.globals = []

    def push(self, child: ASTNode):
        self.globals.append(child)
        child.parent = self

    def dot(self, g):
        g.node(str(self.id), 'globals')
        for c in self.globals:
            c.dot(g)
            g.edge(str(self.id), str(c.id))

class Component(ASTNode):
    '''
    A non-realized set of definitions, declarations, and assignments.
    '''
    def __init__(self, name: str):
        super().__init__(self)
        self.name = name
        self.items = []
    
    def push(self, child: ASTNode):
        self.items.append(child)
        child.parent = self

    def dot(self, g):
        g.node(str(self.id), nohtml(f'component | {self.name}'))
        for c in self.items:
            c.dot(g)
            g.edge(str(self.id), str(c.id))

class Pipeline(Component):
    '''
    A Component that the user has declared is:
      * meant for export, and thus realization
      * complete for whatever interface contract they intend
    '''
    def __init__(self, name: str):
        super().__init__(name)

    def dot(self, g):
        g.node(str(self.id), nohtml(f'pipeline | {self.name}'))
        for c in self.items:
            c.dot(g)
            g.edge(str(self.id), str(c.id))


class TypeRef(ASTNode):
    '''
    A reference to a type, with optional array spec.
    '''
    def __init__(self, name: str, array_spec: Optional[int] = None):
        super().__init__()
        self.name = name
        self.array_spec = array_spec
        pass

    def dot(self, g):
        if self.array_spec:
            g.node(str(self.id), nohtml(f'typeref | {self.name} @ {str(self.array_spec)}'))
        else:
            g.node(str(self.id), nohtml(f'typeref | {self.name}'))
        



class Expression(ASTNode):
    def __init__(self):
        super().__init__()


class FnCall(Expression):
    def __init__(self, ref: str, args: List[Expression] = None):
        super().__init__()
        self.ref = ref
        self.args = args or []
        for a in self.args:
            a.parent = self

    def dot(self, g):
        g.node(str(self.id), nohtml(f'fncall | {self.ref} | <f0> (...)'))
        for a in self.args:
            a.dot(g)
            g.edge(f'{str(self.id)}:<f0>', str(a.id))

class Operation(Expression):
    '''
    An executable operation.
    '''
    NAME = 'unknown'
    def __init__(self):
        super().__init__()


class UnaryOp(Operation):
    '''
    An operation with a single operand.
    '''
    def __init__(self, operand: ASTNode):
        super().__init__()
        self.operand = operand
        self.operand.parent = self

    def dot(self, g):
        g.node(str(self.id), nohtml(f'{type(self).NAME}'))
        g.edge(str(self.id), str(self.operand.id))


class BinaryOp(Operation):
    '''
    An operation with two operands.
    '''
    def __init__(self, left: ASTNode, right: ASTNode):
        super().__init__()
        self.left = left
        left.parent = self

        self.right = right
        right.parent = self
    
    def dot(self, g):
        g.node(str(self.id), nohtml(f'<f0> | {type(self).NAME} | <f1>'))
        self.left.dot(g)
        self.right.dot(g)
        g.edge(f'{str(self.id)}:<f0>', str(self.left.id))
        g.edge(f'{str(self.id)}:<f1>', str(self.right.id))

def make_binop(opname: str):
    class Ret(BinaryOp):
        NAME = opname
        def __init__(self, left: ASTNode, right: ASTNode):
            super().__init__(left, right)
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
        def __init__(self, operand: ASTNode):
            super().__init__(operand)
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
    def __init__(self, value):
        super().__init__()
        self.value = value

    def dot(self, g):
        g.node(str(self.id), nohtml(f"Value | {self.value}"))


class Number(Terminal):
    '''
    A numeric quantity.
    '''
    def __init__(self, value):
        super().__init__(value)


class Integer(Number):
    '''
    An integer quantity.
    '''
    def __init__(self, value: int):
        super().__init__(value)


class Float(Number):
    '''
    A floating-point quantity.
    '''
    def __init__(self, value: float):
        super().__init__(value)


class VarDecl(Expression):
    '''
    A variable declaration, staged or unstaged,
    '''
    def __init__(self, name: str, stageref: str = None, typeref: Optional[TypeRef] = None, index: Optional[int] = None):
        super().__init__()
        self.name: str = name
        self.stage: str = stageref
        self.typeref: Optional[TypeRef] = typeref
        if self.typeref:
            self.typeref.parent = self
        self.index: int = index
        pass

    def dot(self, g):
        g.node(str(self.id), nohtml(f'vardecl | {self.stage or "*"} | {self.name} | <f0> {"T" if self.typeref else "?"} | {str(self.index or "?")}'))
        if self.typeref:
            self.typeref.dot(g)
            g.edge(f'{str(self.id)}:<f0>', str(self.typeref.id))

class VarRef(Expression):
    '''
    A reference to a variable.
    '''
    def __init__(self, name: str):
        super().__init__()
        self.ref = name

    def dot(self, g):
        g.node(str(self.id), nohtml(f'var | {self.ref}'))

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
