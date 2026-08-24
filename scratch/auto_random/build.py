"""Build/diff driver for the dRandom_c unit (Linux container).

Same pattern as scratch/auto_lift2/build.py: point the project harness at the
container's wibo/native shims, compile drafts from this directory.

    python3 -c "import sys; sys.path.insert(0,'scratch/auto_random'); import build; print(build.report())"
"""
import os
import re
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_random')
SHIM_DTK = os.path.join(BASE, 'shims', 'dtk')
SHIM_MWCC = os.path.join(BASE, 'shims', 'mwcceppc')

sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

harness.MWCC = SHIM_MWCC
harness.DTK = SHIM_DTK

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_random', 'target.txt')

FN = 'calcMachineRandom__9dRandom_cFv'


def build(source, label='draft'):
    src = source if os.path.isabs(source) else os.path.join(BASE, source)
    obj = os.path.join(BASE, label + '.o')
    txt = os.path.join(BASE, label + '.txt')
    inc = (os.path.join(BASE, 'shadow'),) if os.path.isdir(os.path.join(BASE, 'shadow')) else ()
    ok, log = harness.compile_draft(src, obj, extra_inc=inc, module='wiimj2d')
    if not ok:
        return False, log
    ok, log = harness.disasm(obj, txt)
    if not ok:
        return False, log
    return True, txt


def diff(function=FN):
    ok_t, msg = harness.diff_fn(TARGET, os.path.join(BASE, 'draft.txt'), function)[:2]
    return ok_t, msg


def shape(txt_path, function=FN):
    body = harness.extract(txt_path, function)
    if body is None:
        return None
    section = '\n'.join(body)
    m = re.search(r'stwu\s+r1,\s*-0x([0-9A-Fa-f]+)\(r1\)', section)
    frame = ('0x%X' % int(m.group(1), 16)) if m else None
    gpr = sorted({int(x) for x in re.findall(r'\bstw\s+r(\d+),\s*0x[0-9A-Fa-f]+\(r1\)', section)
                  if int(x) >= 13})
    fpr = sorted({int(x) for x in re.findall(r'\bstfd\s+f(\d+),\s*0x[0-9A-Fa-f]+\(r1\)', section)
                  if int(x) >= 14})
    return len(body), frame, gpr, fpr


def report():
    matched, msg = diff()
    t = harness.extract(TARGET, FN) or []
    d = harness.extract(os.path.join(BASE, 'draft.txt'), FN) or []
    return '%s %s (%dw target / %dw draft)' % (
        FN, 'MATCH' if matched else 'DIFFS: ' + msg, len(t), len(d))


if __name__ == '__main__':
    ok, log = build('d_random.cpp')
    print(('BUILD OK' if ok else 'BUILD FAIL') + ': ' + str(log)[-400:])
    if ok:
        print(report())
