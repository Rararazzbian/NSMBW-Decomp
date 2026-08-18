"""Show instruction-level diff between draft and target for named draft functions."""
import os, sys, re

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'wip/wm_units'))
from verify_anon import functions, norm  # noqa: E402

DRAFT = os.path.join(ROOT, 'scratch/round15/draft.txt')
OBJDIR = os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj')

# target address -> draft function name
PAIRS = [
    (0x163800, 'create__11daWmGhost_cFv'),
    (0x163940, 'createModel__11daWmGhost_cFv'),
]

def load_targets():
    sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
    import harness as H
    cache = os.path.join(ROOT, 'wip/wm_units/_dis')
    os.makedirs(cache, exist_ok=True)
    t = []
    for obj in ['auto_00_00163620_text.o', 'auto_fn_2_164180_text.o', 'auto_00_00164204_text.o']:
        out = os.path.join(cache, obj + '.txt')
        if not os.path.exists(out):
            H.disasm(os.path.join(OBJDIR, obj), out)
        t += functions(out, with_addr=True)
    return dict((a, (n, ins)) for a, n, ins in t)

targets = load_targets()
drafts = dict((n, ins) for n, ins in functions(DRAFT))

for taddr, dname in PAIRS:
    if taddr not in targets:
        print('target %#x not found' % taddr); continue
    tname, tins = targets[taddr]
    dins = drafts.get(dname)
    if dins is None:
        print('draft %s not emitted' % dname); continue
    want, got = norm(tins), norm(dins)
    print('\n' + '='*70)
    print('TARGET %#x (%s, %d ins)  vs  DRAFT %s (%d ins)' % (taddr, tname, len(tins), dname, len(dins)))
    print('='*70)
    n = 0
    for i in range(max(len(want), len(got))):
        a = want[i] if i < len(want) else '<none>'
        b = got[i] if i < len(got) else '<none>'
        if a != b:
            n += 1
            print('  %3d | want: %-42s got: %s' % (i, a, b))
    print('  -> %d differing' % n)
