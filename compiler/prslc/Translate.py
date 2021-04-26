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
    #_map(push_decls, root.subs())


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
    types_pass(realfunc)
    fn_call['T'] = realfunc['T']


def synthesize_types(root):
    '''
    Depth-first deduction of types.
    '''
    if isinstance(root,  TypeRef): # a type root
        root['T'] = root.typecode()
    elif isinstance(root, FnDef):
        if 'T' in root.body:
            root['T'] = root.body['T']
    elif isinstance(root,  FnCall):
        synth_call_type(root)
    elif isinstance(root,  UnaryOp):
        root['T'] = root.operand['T']
    elif isinstance(root,  BinaryOp):
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


def postorder(root):
    for c in root.subs():
        for i in postorder(c):
            yield i
    yield root


def preorder(root):
    yield root
    for c in root.subs():
        for i in preorder(c):
            yield i

def scopes_pass(ast):
    push_scopes(ast)

def decls_pass(ast):
    _map(push_decls, preorder(ast))

def types_pass(ast):
    _map(synthesize_types, postorder(ast))


def do_passes(ast):
    scopes_pass(ast)
    decls_pass(ast)
    types_pass(ast)
