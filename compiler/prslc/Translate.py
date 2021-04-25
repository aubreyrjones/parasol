from collections import deque
from AST import *

def _map(f, s):
    deque(map(f, s), 0)



def close_refs(root):
    if isinstance(root,  TU): # has no type
        _map(close_refs, root.globals)
    elif type(root) in (Component, Pipeline): # has no type
        _map(close_refs, root.items)
    elif isinstance(root,  TypeRef): # a type root
        pass
    elif isinstance(root,  FnCall):
        pass
    elif isinstance(root,  UnaryOp):
        close_refs(root.operand)
    elif isinstance(root,  BinaryOp):
        close_refs(root.lef)
        close_refs(root.right)
    elif isinstance(root,  VarDecl):
        pass
    elif isinstance(root,  VarRef):
        pass


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
        if root['ref']:
            root['T'] = root['ref'].realized(root.arg_typecodes()).type()
    elif isinstance(root,  UnaryOp):
        synthesize_types(root.operand)
        root['T'] = root.operand['T']
    elif isinstance(root, Gets):
        if root.left['T'] and root.right['T'] and (root.left['T'] != root.right['T']):
            raise RuntimeError('Type error: assignment operands are of different types. ' + str(root.left['T']) + " " + str(root.right['T']))
        
    elif isinstance(root,  BinaryOp):
        synthesize_types(root.left)
        synthesize_types(root.right)
        if root.left['T'] and root.right['T'] and (root.left['T'] != root.right['T']):
            raise RuntimeError('Type error: operands are of different types. ' + str(root.left['T']) + " " + str(root.right['T']))
        root['T'] = root.left['T'] or root.right['T']
    elif isinstance(root,  VarDecl):
        link_vardecl_types(root)
    elif isinstance(root,  VarRef):
        pass
        #root['T'] = root['ref']['T']
        #root['T'] = ('?', None)