"""Build/diff driver for the KokoopaSpFumiCheck_c unit (Linux container).

    python3 scratch/auto_kokoopa_fumi/build.py            # build + report
    python3 -c "import sys; sys.path.insert(0,'scratch/auto_kokoopa_fumi'); \
                import build; print(build.build('d_kokoopa_sp_fumi_check.cpp','draft'))"
"""
import os
import re
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_kokoopa_fumi')
SHIM_DTK = os.path.join(BASE, 'dtk')
SHIM_MWCC = os.path.join(BASE, 'shims', 'mwcceppc')

sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

# Point the harness at this container's toolchain before anything uses it.
harness.MWCC = SHIM_MWCC
harness.DTK = SHIM_DTK

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_kokoopa_sp_fumi_check', 'target.txt')


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


def score(draft_txt):
    out = []
    for name in ['operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c',
                 '__dt__20KokoopaSpFumiCheck_cFv']:
        matched, msg = harness.diff_fn(TARGET, draft_txt, name)
        n = len(harness.extract(draft_txt, name) or [])
        t = len(harness.extract(TARGET, name) or [])
        out.append('%-52s %s (%dw/%dw)' %
                   (name, 'MATCH' if matched else
                    (msg.splitlines()[0] if msg else 'DIFFS?'), t, n))
    return '\n'.join(out)


if __name__ == '__main__':
    src = sys.argv[1] if len(sys.argv) > 1 else 'd_kokoopa_sp_fumi_check.cpp'
    lbl = sys.argv[2] if len(sys.argv) > 2 else 'draft'
    ok, log = build(src, lbl)
    if not ok:
        print(log)
        sys.exit(1)
    print(score(os.path.join(BASE, lbl + '.txt')))
