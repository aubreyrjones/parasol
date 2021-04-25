
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