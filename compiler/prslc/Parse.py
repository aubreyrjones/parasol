import parasol_parser as parser
from AST import *

def convert_global_list(n):
    r = TU(n[0].line if len(n) else -1)
    for c in n:
        r.push(convert_global(c))
    return r

def convert_global(n):
    rval = None
    if n.name == 'component':
        rval = Component(cast_ident(n[0]), n.line)
    elif n.name == 'pipeline':
        rval = Pipeline(cast_ident(n[0]), n.line)
    else:
        raise RuntimeError("Parse conversion error. Expected component or pipeline, got: " + n.name)

    for c in n[1]:
        rval.push(convert_component_item(c))

    return rval

def convert_component_item(n):
    r = None
    if n.name == 'Gets':
        return Gets(convert_expr(n[0]), convert_expr(n[1]), n.line)
    else:
        raise RuntimeError("Parse conversion error. Expected Gets, got: " + n.name)
    
def convert_expr(n):
    if n is None: return None

    # complex expressions
    if n.name == 'staged_vardecl':
        return convert_vardecl(n[1], cast_ident(n[0]))
    elif n.name == 'vardecl':
        return convert_vardecl(n, None)
    elif n.name == 'fncall':
        return FnCall(cast_ident(n[0]), list(map(convert_expr, n[1])), n.line)
    elif n.name == 'varref':
        return VarRef(cast_ident(n[0]), n[0].line)

    # operations
    if len(n) == 2: # binop if 2 children
        return Binops[n.name](convert_expr(n[0]), convert_expr(n[1]), n.line)
    elif len(n) == 1: # unop if 1 child
        return Unops[n.name](convert_expr(n[0]), n.line)
    
    if n.type: # terminal
        return convert_terminal_value(n)
    
    raise RuntimeError("Unknown expression type: " + n.name or n.type)

def convert_terminal_value(n):
    if n.type == 'INTEGER':
        return Integer(cast_integer(n), n.line)

def convert_vardecl(n, stage: str):
    return VarDecl(cast_ident(n[0]), stage, convert_typeref(n[1][0]), cast_integer(n[2][0]), n.line)

def convert_typeref(n):
    if n is None: return None
    return TypeRef(cast_ident(n[0]), cast_integer(n[1]), n.line)

def cast_ident(n) -> str:
    if n is None: return None
    if n.name is not None: raise RuntimeError('Parse conversion error. Converting ident, but non-terminal found.')

    if n.type in ('STAGEREF', 'VARDECL', 'FNCALL'):
        return n.value[:-1]
    if n.type == 'IDENT':
        return n.value
    
def cast_integer(n) -> int:
    if n is None: return None
    if n.type != 'INTEGER': raise RuntimeError('Parse conversion error. Expected INTEGER type, got' + repr(n.type))
    
    return int(n.value)


def parse_and_convert(source: str) -> TU:
    return convert_global_list(parser.parse(source))
    pass



if __name__ == '__main__' :
    import sys
    ast = parse_and_convert(open(sys.argv[1]).read())
    visualize_ast(ast)
