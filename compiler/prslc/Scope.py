class Variable:
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

    def refine(self, decl):
        self._ref_attr(decl, 'stage')
        self._ref_attr(decl, 'index')
        decltype = decl.typecode()
        if decltype:
            if self.type and decltype != self.type:
                raise RuntimeError(f"Error refining variable definition. Mismatched type declaration for {self.name}. Declared earlier with {str(self.type)}, then with {str(decltype)} at line {decl.line}.")
            self.type = decltype

    def push_inferred_type(self, typecode):
        if self.type and self.type != typecode:
            raise RuntimeError(f"Inferred type {str(typecode)} is incompatible with explicitly declared type {str(self.type)} for variable {self.name}.")
        self.type = typecode
        

class LexicalScope:
    def __init__(self, name, owner, parent: 'LexicalScope' = None):
        self.parent = parent
        self.name = name
        self.owner = owner
        self.locals = {}
    
    def __getitem__(self, k):
        if k in self.locals:
            return self.locals[k]
        if self.parent:
            return self.parent[k]
        return None

    def __setitem__(self, k, v):
        self.locals[k] = v
    
    def __contains__(self, k):
        return self[k] is not None

    def declare_var(self, decl):
        var = self[decl.name]
        if not var:
            self[decl.name] = Variable(decl.name, decl)
        else:
            var.refine(decl)
        