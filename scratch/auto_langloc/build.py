"""Build/diff driver for the dFunsuiAct_c unit (Linux container).

    python3 -c "import sys; sys.path.insert(0,'scratch/auto_langloc'); import build; \
               print(build.build('d_funsui_act.cpp','draft'))"
"""
import os
import re
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_langloc')
SHIM_DTK = os.path.join(ROOT, 'scratch', 'auto_funsui', 'dtk')
SHIM_MWCC = os.path.join(ROOT, 'scratch', 'auto_funsui', 'shims', 'mwcceppc')

sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

# Point the harness at this container's toolchain before anything uses it.
harness.MWCC = SHIM_MWCC
harness.DTK = SHIM_DTK

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_lang_loc_string', 'target.txt')


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


def report():
    """Score every function in the target against draft.txt."""
    draft = os.path.join(BASE, 'draft.txt')
    out = []
    for name in harness.list_functions(TARGET):
        matched, msg = harness.diff_fn(TARGET, draft, name)
        n = len(harness.extract(draft, name) or [])
        t = len(harness.extract(TARGET, name) or [])
        out.append('%-40s %s (%dw target / %dw draft)' %
                   (name, 'MATCH' if matched else msg.splitlines()[0] if msg else 'DIFFS',
                    t, n))
    return '\n'.join(out)


if __name__ == '__main__':
    print(report())
