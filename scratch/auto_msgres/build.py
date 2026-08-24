"""Build/diff driver for the MsgRes_c unit (Linux container).

    python3 scratch/auto_msgres/build.py
    python3 scratch/auto_msgres/build.py vX.cpp   # build a variant
"""
import os
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_msgres')
SHIM_DTK = os.path.join(ROOT, 'scratch', 'auto_funsui', 'dtk')
SHIM_MWCC = os.path.join(ROOT, 'scratch', 'auto_funsui', 'shims', 'mwcceppc')

sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

harness.MWCC = SHIM_MWCC
harness.DTK = SHIM_DTK

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', 'dol_bases_d_message', 'target.txt')

FNS = [
    '__ct__8MsgRes_cFPCvPQ23EGG4Heap',
    '__dt__8MsgRes_cFv',
    'getScale__8MsgRes_cFUlUl',
    'getFont__8MsgRes_cFUlUl',
]

INC = (os.path.join(BASE, 'shadow'), os.path.join(BASE, 'shadow', 'lib'))


def build(source='d_message.cpp', label=None):
    src = source if os.path.isabs(source) else os.path.join(BASE, source)
    if label is None:
        label = os.path.splitext(os.path.basename(src))[0]
    obj = os.path.join(BASE, label + '.o')
    txt = os.path.join(BASE, label + '.txt')
    ok, log = harness.compile_draft(src, obj, extra_inc=INC, module='wiimj2d')
    if not ok:
        print(log)
        return None
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print(log)
        return None
    return txt


def report(txt):
    out = []
    total = 0
    for name in FNS:
        matched, msg = harness.diff_fn(TARGET, txt, name)
        n = len(harness.extract(txt, name) or [])
        t = len(harness.extract(TARGET, name) or [])
        head = msg.splitlines()[0] if msg else ('MATCHING' if matched else '?')
        total += 0 if matched else 1
        out.append('%-32s %s (%dw/%dw)' % (name, 'MATCH' if matched else head, t, n))
    return '\n'.join(out), total


if __name__ == '__main__':
    src = sys.argv[1] if len(sys.argv) > 1 else 'd_message.cpp'
    txt = build(src)
    if txt:
        rep, bad = report(txt)
        print(rep)
        sys.exit(1 if bad else 0)
    sys.exit(2)
