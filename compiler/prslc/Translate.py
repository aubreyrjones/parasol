from AST import *
import Types
from Scope import LexicalScope
from Util import _map
import traceback


def push_scopes(root, parent_scope = None):
    '''
    Push scope objects onto all nodes, adding new
    ones at scope boundaries.
    '''
    if isinstance(root, TU):
        root.scope = LexicalScope('__root__', root)
    elif root.defines_scope():
        root.scope = LexicalScope(root.name, root, parent_scope, root.defines_scope()[0])
    else:
        root.scope = parent_scope
    _map(lambda g: push_scopes(g, root.scope), root.subs())


def push_decls(root):
    '''
    Push declarations into enclosing scopes.
    '''
    if isinstance(root, VarDecl):
        root.scope.declare_var(root)
    elif isinstance(root, FnDef):
        root.scope.parent.define_fn(root)
    _map(push_decls, root.subs())


def link_vardecl_types(root):
    '''
    Back-link from Variable to VarDecl.
    '''
    if root['T']:
        return
    root['T'] = root.follow().type


def synth_call_type(fn_call):
    '''
    Realize a function with the given parameters, and get its type.
    '''
    realfunc = fn_call.follow().realize(fn_call.arg_typecodes())
    _map(synthesize_types, realfunc.params)
    synthesize_types(realfunc.body)
    realfunc['T'] = realfunc.body['T']
    fn_call['T'] = realfunc['T']


def synthesize_types(root):
    '''
    Depth-first deduction of types.
    '''
    try:
        if isinstance(root,  TypeRef): # a type root
            root['T'] = root.typecode()
        elif isinstance(root,  FnCall):
            _map(synthesize_types, root.args)
            synth_call_type(root)
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
                root['T'] = root.left['T']
            elif root.left['T'] and root.right['T']:
                root['T'] = Types.combine_binary(opname, root.left['T'], root.right['T'])
            else:
                root['T'] = None
        elif isinstance(root,  VarDecl):
            link_vardecl_types(root)
        elif isinstance(root,  VarRef):
            root['T'] = root.follow().type
        else:
            _map(synthesize_types, root.subs())
    except Exception as ex:
        raise RuntimeError(f"Error deducing types for {type(root)} on line {root.line}. {str(ex)}.")


def postorder(root):
    for c in root.subs():
        for i in postorder(c):
            yield i
    yield root