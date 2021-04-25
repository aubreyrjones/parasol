from collections import deque
from copy import deepcopy

def _map(f, s):
    deque(map(f, s), 0)

def copy_subtree(copy_root):
    '''
    '''
    parent = copy_root.parent
    scope_parent = copy_root.scope.parent
    
    copy_root.parent = None
    copy_root.scope.parent_scope = None
    
    retval = deepcopy(copy_root)
    retval.re_id()

    retval.parent = parent
    retval.scope.parent = scope_parent

    copy_root.scope.parent = scope_parent
    copy_root.parent = parent
    return retval
