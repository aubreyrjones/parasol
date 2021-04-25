
NativeTypes = ['int', 'float', 'vec2', 'vec3', 'vec4', 'mat4']

def same(t):
    return (t, t)

BINOP_TYPE_MAP = {}

BINOP_TYPE_MAP['Mult'] = {
    same('int') : 'int',
    same('float') : 'float',
    
    same('mat4') : 'mat4',
    ('mat4', 'vec4') : 'vec4'
}


def combine_binary(op: str, t1: tuple, t2: tuple) -> tuple:
    if t1[1] != t2[1]: # array spec must match, no wildcard for now
        return None
    
    if op not in BINOP_TYPE_MAP: raise RuntimeError('Type result not defined for operation: ' + op)

    opmap = BINOP_TYPE_MAP[op]

    combo = (t1[0], t2[0])
    if combo not in opmap: raise RuntimeError(f'Operator {op} result not defined for operands {repr(combo)}')
    return (opmap[combo], t1[1])