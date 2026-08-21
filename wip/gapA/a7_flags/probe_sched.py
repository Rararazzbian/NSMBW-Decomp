"""Focused: under the variants that changed the key lines at all, is the
mBaseSpeed/const LOAD PAIR in retail order, or is the whole body just reshuffled?
"""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
from flagsweep import parse, TARGET, FN, compile_with, BASE, solo

CASES = [('BASELINE', list(BASE)),
         ('-schedule off', BASE + ['-schedule', 'off']),
         ('-O4,p -schedule off', solo(BASE, '-O4', '-O4,p') + ['-schedule', 'off']),
         ('-fp fmadd', [f if f != 'hard' else 'fmadd' for f in BASE])]

LOADS = re.compile(r'^lfs\s+(f\d+),\s*(0x60\(r\d+\)|.*sda21.*)$')

tgt = harness.canonicalise([t for _, t in parse(TARGET)[FN]])
print('=== TARGET load pairs ===')
for i, l in enumerate(tgt):
    m = LOADS.match(l)
    if m:
        print('  %3d  %s' % (i, l))

for label, flags in CASES:
    obj, txt = os.path.join(HERE, 'p.o'), os.path.join(HERE, 'p.txt')
    ok, log = compile_with(flags, obj)
    if not ok or not harness.disasm(obj, txt)[0]:
        print('\n=== %s: compile/disasm failed' % label)
        continue
    got = harness.canonicalise([t for _, t in parse(txt)[FN]])
    ndiff = sum(1 for i in range(max(len(got), len(tgt)))
                if (got[i] if i < len(got) else None) != (tgt[i] if i < len(tgt) else None))
    print('\n=== %s : %d insns, %d lines differ from target ===' % (label, len(got), ndiff))
    for i, l in enumerate(got):
        m = LOADS.match(l)
        if m:
            print('  %3d  %s' % (i, l))
