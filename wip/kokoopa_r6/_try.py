"""Variant harness for koopa_castle __sinit.

Usage: python wip/kokoopa_r6/_try.py <variant_file.txt> <label>

<variant_file.txt> replaces the text between the BEGIN/END markers in the
namespace block of d_a_wm_koopa_castle.cpp.  Prints the __sinit diff count and
the full unit tally.
"""
import sys, os, subprocess, re

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
import harness as H
from verify_anon import functions, norm

HERE = os.path.join(ROOT, 'wip', 'kokoopa_r6')
BASE = os.path.join(HERE, 'base.cpp')          # pristine baseline copy
SRC  = os.path.join(HERE, 'd_a_wm_koopa_castle.cpp')
OBJ  = os.path.join(HERE, 'draft.o')
TXT  = os.path.join(HERE, 'draft.txt')
INC  = os.path.join(HERE, 'shadow_include')
CACHE = os.path.join(HERE, '_dis_local')
os.makedirs(CACHE, exist_ok=True)

# ---- target instructions for fn_2_191C30 ----
TOBJ = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj',
                    'auto_fn_2_191C30_text.o')
tout = os.path.join(CACHE, 'auto_fn_2_191C30_text.o.txt')
if not os.path.exists(tout):
    H.disasm(TOBJ, tout)
TARGET = None
for addr, name, ins in functions(tout, with_addr=True):
    if addr == 0x191C30:
        TARGET = ins
assert TARGET is not None


def measure(label, verbose=False, tally=False):
    ok, log = H.compile_draft(SRC, OBJ, extra_inc=[INC], module='d_basesNP')
    if not ok:
        print('%-46s BUILD FAIL' % label)
        print(log[-2500:])
        return None
    ok2, log2 = H.disasm(OBJ, TXT)
    if not ok2:
        print('%-46s DISASM FAIL' % label)
        return None
    draft = None
    for name, ins in functions(TXT):
        if '__sinit' in name:
            draft = ins
    if draft is None:
        print('%-46s NO __sinit' % label)
        return None
    a, b = norm(TARGET), norm(draft)
    n = sum(1 for i in range(max(len(a), len(b)))
            if (a[i] if i < len(a) else '#') != (b[i] if i < len(b) else '#'))
    print('%-46s diffs=%-4d len %d/%d' % (label, n, len(a), len(b)))
    if verbose:
        for i in range(max(len(a), len(b))):
            x = a[i] if i < len(a) else '<none>'
            y = b[i] if i < len(b) else '<none>'
            print('%3d %s want:%-38s got:%-38s' % (i, '  ' if x == y else '<<', x, y))
    if tally:
        r = subprocess.run([sys.executable,
                            os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                            TXT, '0x1910d0', '0x191d40',
                            os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_001910A4_text.o'),
                            TOBJ,
                            os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_00191D18_text.o')],
                           capture_output=True, text=True)
        print(r.stdout)
    return n


BEGIN = '    struct KoopaShipPosGuard_t {'
END = '\nint daWmKoopaCastle_c::execute()'


def apply_variant(text):
    base = open(BASE, encoding='utf-8').read()
    i = base.index(BEGIN)
    j = base.index(END)
    out = base[:i] + text + '\n' + base[j:]
    open(SRC, 'w', encoding='utf-8', newline='\n').write(out)


if __name__ == '__main__':
    if len(sys.argv) == 1:
        measure('baseline (current SRC)', verbose='-v' in sys.argv, tally=True)
    else:
        vf = sys.argv[1]
        label = sys.argv[2] if len(sys.argv) > 2 else os.path.basename(vf)
        if vf == 'RESTORE':
            import shutil
            shutil.copy(BASE, SRC)
            measure('restored baseline', tally=True)
        else:
            apply_variant(open(vf, encoding='utf-8').read())
            measure(label, verbose=('-v' in sys.argv))
