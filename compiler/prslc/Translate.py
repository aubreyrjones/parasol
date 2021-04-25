from AST import *
import Types
from Scope import LexicalScope
from Util import _map


def push_scopes(root, parent_scope = None):
    if isinstance(root, TU):
        root.scope = LexicalScope('__root__', root)
    elif isinstance(root, FnDef) or type(root) in (Pipeline, Component):
        root.scope = LexicalScope(root.name, root, parent_scope)
    else:
        root.scope = parent_scope
    _map(lambda g: push_scopes(g, root.scope), root.subs())


def push_decls(root):
    if isinstance(root, VarDecl):
        root.scope.declare_var(root)
    if isinstance(root, FnDef):
        root.scope.parent.define_fn(root)
    _map(push_decls, root.subs())


def link_vardecl_types(root):
    if root['T']:
        return
    root['T'] = root.follow().type


def synth_call_type(fn_call):
    realfunc = fn_call.follow().realize(fn_call.arg_typecodes())
    _map(synthesize_types, realfunc.params)
    synthesize_types(realfunc.body)
    realfunc['T'] = realfunc.body['T']
    fn_call['T'] = realfunc['T']

def synthesize_types(root):
    if isinstance(root,  TU): # has no type
        _map(synthesize_types, root.globals)
    elif type(root) in (Component, Pipeline): # has no type
        _map(synthesize_types, root.items)
    elif isinstance(root,  TypeRef): # a type root
        root['T'] = root.typecode()
    elif isinstance(root,  FnCall):
        _map(synthesize_types, root.args)
        synth_call_type(root)
        # if root['ref']:
        #     root['T'] = root['ref'].realized(root.arg_typecodes()).type()
    elif isinstance(root,  UnaryOp):
        synthesize_types(root.operand)
        root['T'] = root.operand['T']
    elif isinstance(root,  BinaryOp):
        synthesize_types(root.left)
        synthesize_types(root.right)

        opname = type(root).NAME
        # set our type
        if isinstance(root, Gets):
            root.left.follow().refine_type(root.right['T'])
            root.left['T'] = root.right['T']
            root['T'] = root.right['T']
        elif root.left['T'] and root.right['T']:
            root['T'] = Types.combine_binary(opname, root.left['T'], root.right['T'])
        else:
            root['T'] = None
    elif isinstance(root,  VarDecl):
        link_vardecl_types(root)
    elif isinstance(root,  VarRef):
        root['T'] = root.follow().type