"""Build/diff driver for the d_lift_allhit_draw2 unit.

Runs the project's own tools unmodified by pointing their Windows-exe
constants at wibo/native shims for this Linux container.

    python -c "import sys; sys.path.insert(0,'scratch/auto_lift2'); import build; \
               print(build.build('d_lift_allhit_draw2.cpp','draft','draw__18dLiftAllhitDraw2_cFv'))"
"""
import os
import re
import subprocess
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_lift2')
SHIM_DTK = os.path.join(BASE, 'shims', 'dtk')
SHIM_MWCC = os.path.join(BASE, 'shims', 'mwcceppc')

sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

# Point the harness at this container's toolchain before anything uses it.
harness.MWCC = SHIM_MWCC
harness.DTK = SHIM_DTK

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_lift_allhit_draw2', 'target.txt')


def build(source, label='draft'):
    """Compile+disassemble a draft in BASE. Returns (ok, txt_path_or_log)."""
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


def shape(txt_path, function):
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


def diff(function):
    ok_t, want = harness.diff_fn(TARGET, os.path.join(BASE, 'draft.txt'), function)[:2]
    return ok_t, want


def report():
    """Score every function in the target against the draft."""
    out = []
    for name in harness.list_functions(TARGET):
        matched, msg = harness.diff_fn(TARGET, os.path.join(BASE, 'draft.txt'), name)
        n = len(harness.extract(os.path.join(BASE, 'draft.txt'), name) or [])
        t = len(harness.extract(TARGET, name) or [])
        out.append('%-40s %s (%dw target / %dw draft)' %
                   (name, 'MATCH' if matched else 'DIFFS', t, n))
    return '\n'.join(out)


if __name__ == '__main__':
    print(report())
