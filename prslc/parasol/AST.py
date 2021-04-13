'''
Defines the AST for Parasol.
'''

from typing import *

class Identifier:
    def __init__(self, name: str, stage: str = None, target: 'Node' = None):
        self.name = name
        self.stage = stage
        self.target = target

    def __repr__(self):
        if self.stage:
            return f"{self.stage}[{self.name}]->{repr(self.target)}"
        else:

class PType:
    def __init__(self, name: str):
        self.name = name

    def __repr__(self):
        return f"<{self.name}>"


class LexicalScope:
    def __init__(self, parent: 'LexicalScope' = None):
        self.locals = {}
        self.parent = parent
    
    def lookup(self, name: str) -> Optional[Identifier]:
        if name in self.locals:
            return self.locals[name]
        elif self.parent:
            return self.parent.lookup(name)
        else:
            return None


class Node:
    _id_counter = 0
    
    def _new_id() -> int:
        nid = Node._id_counter
        Node._id_counter += 1
        return nid

    def __init__(self, line: int):
        global _id_counter
        self.line = line
        self.id = Node._new_id()


class VarDecl(Node):
    def __init__(self, line: int, ident: Identifier, ptype: PType = None, array_spec: int = None):
        super().__init__(self, line)
        self.ident: Identifier = ident
        self.ptype: PType = ptype
        self.array_spec: int = array_spec


class Expression(Node):
    def __init__(self, line: int):
        super().__init__(self, line)


class FunctionDef(Node):
    def __init__(self, line: int, ident: Identifier, params: List[VarDecl], body: Expression):
        super().__init__(self, line)
        self.ident = ident
        self.params = params
        self.body = body

class Pipeline(Node):
    def __init__(self, line: int, name: str):
        super().__init__(self, line)
        self.name = name
        self.definitions = []
        self.scope = LexicalScope()


class Struct(Node):
    def __init__(self, line: int, name: str):
        super().__init__(self, line)
        self.name = name
        self.members: List[VarDecl] = []


class Module(Node):
    def __init__(self, name: str):
        super().__init__(self, 0)
        self.name = name
        self.definitions: List[Union[Struct, Pipeline]] = []
        self.scope = LexicalScope()
