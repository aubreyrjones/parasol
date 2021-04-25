from Util import copy_subtree

NativeTypes = ['int', 'float', 'vec2', 'vec3', 'vec4', 'mat4']

def same(t):
    return (t, t)

BINOP_TYPE_MAP = {}

BINOP_TYPE_MAP['Mult'] = {
    same('int') : 'int',
    same('float') : 'float'
}

BINOP_TYPE_MAP['MMult'] = {
    same('mat4') : 'mat4',
    
    ('mat4', 'vec4') : 'vec4'
}

def combine_binary(op: str, t1: tuple, t2: tuple) -> tuple:
    if t1[1] != t2[1]: # array spec must match, no wildcard for now
        return None
    
    if op not in BINOP_TYPE_MAP: raise RuntimeError('No type results defined for operation: ' + op)

    opmap = BINOP_TYPE_MAP[op]

    combo = (t1[0], t2[0])
    if combo not in opmap: raise RuntimeError(f'Operator {op} type result not defined for operands {repr(combo)}.')
    return (opmap[combo], t1[1])


def unpack_type(typestring: str) -> tuple:
    if typestring == '?': return None
    s = typestring.split(',')
    if len(s) == 1: return (s[0], None)
    return (s[0], int(s[1]))

def pack_type(typecode: tuple) -> str:
    if typecode is None: return "?"
    return f'{typecode[0]}{"," + str(typecode[1]) if typecode[1] else ""}'

def pack_func_type(param_list):
    return ';'.join(map(pack_type, param_list))


class Variable:
    '''
    Variable metadata and perhaps a constant value.
    '''
    def __init__(self, name, decl = None):
        self.name = name
        self.constant_value = None
        self.stage = None
        self.type = None
        self.index = None
        if decl: 
            self.refine(decl)

    def _ref_attr(self, decl, a):
        oval = getattr(decl, a)
        mval = getattr(self, a)
        if oval:
            if mval and mval != oval:
                raise RuntimeError(f"Error refining variable definition. Mismatched {a} declaration for {self.name}. Declared earlier with {str(mval)}, then with {str(oval)} at line {decl.line}.")
            setattr(self, a, oval)

    def refine_type(self, typecode):
        if typecode:
            if self.type and typecode != self.type:
                raise RuntimeError(f"Error refining variable definition. Mismatched type declaration for `{self.name}`. Declared earlier with {str(self.type)}, then refined with {str(typecode)}.")
            self.type = typecode
        

    def refine(self, decl):
        self._ref_attr(decl, 'stage')
        self._ref_attr(decl, 'index')
        self.refine_type(decl.typecode())

    def push_inferred_type(self, typecode):
        if self.type and self.type != typecode:
            raise RuntimeError(f"Inferred type {str(typecode)} is incompatible with explicitly declared type {str(self.type)} for variable {self.name}.")
        self.type = typecode


class Function:
    '''
    Holds all realizations of a particular functions.
    '''
    def __init__(self, definition):
        self.name = definition.name
        self.definition = definition
        self.reals = {}
    
    def realize(self, param_type_list):
        packed_param_type = pack_func_type(param_type_list)
        if packed_param_type in self.reals:
            return self.reals[packed_param_type]
        
        newreal = copy_subtree(self.definition)

        for p, t in zip(self.definition.param_names(), param_type_list):
            newreal.scope[p].refine_type(t)
        
        self.reals[packed_param_type] = newreal
        return newreal
    

class BuiltinFunction(Function):
    '''
    Represents a function whose actual definition is provided by some aspect
    of the compiler or environment.
    '''
    def __init__(self, name, sigs):
        pass