import sys, os

ORIG = os.path.join('include','game','mLib','m_vec.hpp')

with open(ORIG, encoding='utf-8') as fh:
    lines = fh.readlines()

def write_variant(name, remove_line_nums):
    # remove_line_nums: 1-based line numbers to blank out (comment out)
    out = list(lines)
    for ln in remove_line_nums:
        idx = ln - 1
        out[idx] = '// REMOVED: ' + out[idx]
    outdir = os.path.join('wip','vec3pod','variants', name, 'game', 'mLib')
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, 'm_vec.hpp'), 'w', encoding='utf-8') as fh:
        fh.writelines(out)
    print(f'wrote variant {name}: removed lines {remove_line_nums}')

if __name__ == '__main__':
    # variant name and line numbers passed as args
    name = sys.argv[1]
    lns = [int(x) for x in sys.argv[2:]]
    write_variant(name, lns)
