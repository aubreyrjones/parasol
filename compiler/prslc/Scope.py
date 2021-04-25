from Types import Variable, Function


class LexicalScope:
    def __init__(self, name, owner, parent: 'LexicalScope' = None, shadowing = False):
        self.parent = parent
        self.name = name
        self.owner = owner
        self.locals = {}
        self.shadowing = shadowing
    
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
        var = None

        if self.shadowing:
            if decl.name in self.locals:
                var = self.locals[decl.name]
        else:
            var = self[decl.name]
        
        if not var:
            self[decl.name] = Variable(decl.name, decl)
        else:
            var.refine(decl)

    def define_fn(self, fndef):
        func = self[fndef.name]
        if func:
            raise RuntimeError(f"Scope error attempting to define function with name {fndef.name}, but that name is already defined.")
        self[fndef.name] = Function(fndef)