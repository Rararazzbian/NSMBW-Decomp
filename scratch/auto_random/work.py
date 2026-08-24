"""Fast iterate: compile a source file, diff calcMachineRandom against target.

    python3 scratch/auto_random/work.py K2_base_slot.cpp
"""
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness

harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_random/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_random/shims/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_random'
TARGET = '/opt/NSMBW-Decomp/tools/auto_decomp/work/dol_bases_d_random/target.txt'
FN = 'calcMachineRandom__9dRandom_cFv'


def run(src_path):
    name = os.path.splitext(os.path.basename(src_path))[0]
    obj = os.path.join(BASE, name + '.o')
    txt = os.path.join(BASE, name + '.txt')
    ok, log = harness.compile_draft(src_path, obj,
                                    extra_inc=(os.path.join(BASE, 'shadow'),),
                                    module='wiimj2d')
    if not ok:
        return None, 'BUILD FAIL: ' + str(log)[-500:]
    ok, log = harness.disasm(obj, txt)
    if not ok:
        return None, 'DISASM FAIL: ' + str(log)[-300:]
    matched, msg = harness.diff_fn(TARGET, txt, FN)[:2]
    want = harness.extract(TARGET, FN)
    got = harness.extract(txt, FN) or []
    if matched:
        return 0, 'MATCH (%dw)' % len(want)
    # count differing words ourselves
    ndiff = sum(1 for i in range(max(len(want), len(got)))
                if (want[i] if i < len(want) else '<none>') !=
                   (got[i] if i < len(got) else '<none>'))
    return ndiff, msg


if __name__ == '__main__':
    for p in sys.argv[1:]:
        src = p if os.path.isabs(p) else os.path.join(BASE, p)
        n, msg = run(src)
        print('%-28s %s %s' % (os.path.basename(p), n if n is not None else '-', msg))
