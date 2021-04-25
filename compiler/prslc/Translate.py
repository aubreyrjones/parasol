from collections import deque
from AST import *
import Types
from Scope import LexicalScope

def _map(f, s):
    deque(map(f, s), 0)


def push_scopes(root, parent_scope = None):
    if isinstance(root, TU):
        root.scope = LexicalScope('__root__', root)
    elif type(root) in (Pipeline, Component) or isinstance(root, FnDef):
        root.scope = LexicalScope(root.name, root, parent_scope)
    else:
        root.scope = parent_scope
    _map(lambda g: push_scopes(g, root.scope), root.subs())


def push_decls(root):
    if isinstance(root, VarDecl):
        root.scope.declare_var(root)
    _map(push_decls, root.subs())


def close_refs(root):
    if isinstance(root,  TypeRef):
        pass
    elif isinstance(root,  FnCall):
        pass
    _map(close_refs, root.subs())


def link_vardecl_types(root):
    if root['T']:
        return

    if root.typeref:
        synthesize_types(root.typeref)
        root['T'] = root.typeref['T']
        if root['ref']:
            if root['ref']['T']:
                if root['ref']['T'] != root['T']:
                    raise RuntimeError('Multiple VarDecl with incompatible types.')
                else:
                    root['ref']['T'] = root['T']
    else:
        if root['ref']:
            synthesize_types(root['ref'])
            root['T'] = root['ref']['T']


def synthesize_types(root):
    if isinstance(root,  TU): # has no type
        _map(synthesize_types, root.globals)
    elif type(root) in (Component, Pipeline): # has no type
        _map(synthesize_types, root.items)
    elif isinstance(root,  TypeRef): # a type root
        root['T'] = root.typecode()
    elif isinstance(root,  FnCall):
        _map(synthesize_types, root.args)
        if root['ref']:
            root['T'] = root['ref'].realized(root.arg_typecodes()).type()
    elif isinstance(root,  UnaryOp):
        synthesize_types(root.operand)
        root['T'] = root.operand['T']
    elif isinstance(root,  BinaryOp):
        synthesize_types(root.left)
        synthesize_types(root.right)
        if root.left['T'] and root.right['T'] and (root.left['T'] != root.right['T']):
            raise RuntimeError('Type error: operands are of different types. ' + str(root.left['T']) + " " + str(root.right['T']))
        
        opname = type(root).NAME
        # set our type
        if isinstance(root, Gets):
            root.left['T'] = root.right['T']
            root['T'] = root.right['T']
        elif root.left['T'] and root.right['T']:
            root['T'] = Types.combine_binary(opname, root.left['T'], root.right['T'])
        else:
            root['T'] = None
    elif isinstance(root,  VarDecl):
        link_vardecl_types(root)
    elif isinstance(root,  VarRef):
        pass
        #root['T'] = root['ref']['T']
        #root['T'] = ('?', None)