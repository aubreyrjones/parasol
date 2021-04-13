import lark
import os

GRAMMAR_FILENAME = os.path.join(os.path.dirname(__file__), 'parasol.lark')
GRAMMAR = None
with open(GRAMMAR_FILENAME, 'r') as grammar_file:
    GRAMMAR = grammar_file.read()

def make_parser():
    p = lark.Lark(GRAMMAR, start="module", propagate_positions=True)
    return p

def parse(filename):
    with open(filename, 'r') as source:
        return make_parser().parse(source.read())

if __name__ == '__main__':
    import sys
    print(parse(sys.argv[1]).pretty())
    pass
