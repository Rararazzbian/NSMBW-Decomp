import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(BASE, 'draft.cpp')
OBJ = os.path.join(BASE, 'draft.o')
TXT = os.path.join(BASE, 'draft.txt')
TARGET = os.path.join(BASE, 'target.txt')
SHADOW = os.path.join(BASE, 'shadow')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[SHADOW], module='wiimj2d')
print('COMPILE OK:', ok)
if not ok:
    print(log)
    sys.exit(1)

dok, dlog = harness.disasm(OBJ, TXT)
print('DISASM OK:', dok)
if not dok:
    print(dlog)
    sys.exit(1)

fns = harness.list_functions(TARGET)
print('Functions in target: %d' % len(fns))
print()
n_match = 0
for fn in fns:
    matched, rep = harness.diff_fn(TARGET, TXT, fn)
    want = harness.extract(TARGET, fn) or []
    got = harness.extract(TXT, fn) or []
    if matched:
        n_match += 1
        # check if it's byte-identical or just canonicalised-equal
        print('MATCH  len=%3d  %s' % (len(want), fn))
    else:
        print('DIFFER T=%3d D=%3d  %s' % (len(want), len(got), fn))

print()
print('TOTAL: %d / %d matched' % (n_match, len(fns)))
