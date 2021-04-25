
class FunctionGenerator:
    def __init__(self, definition, enclosing_scope):
        self.definition = definition
        self.parent_scope = enclosing_scope

    def realize(self, arg_type_list):
        pass










class RealizedFunction:
    def __init__(self, defnode, param_type_list):
        self.defnode = defnode
        self.name = None
        self.param_type_list = param_type_list
        self.ret_type = None

    def copy_body_and_synthesize(self):
        pass

    def return_type(self):
        return self.ret_type


class BuiltinFunction(RealizedFunction):
    def __init__(self, name, param_type_list, ret_type):
        self.name = None
        self.param_type_list = param_type_list
        self.ret_type = ret_type


BUILTIN_FUNCTIONS = {}

def make_builtin(name, params, ret):
    pass