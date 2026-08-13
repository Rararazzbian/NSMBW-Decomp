import os, sys, shutil, subprocess
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
S = r'C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\a82a73ff-4c16-4614-ab34-6dd919c467b3\scratchpad'
W = os.path.join(S, 'b1n')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

TGT = os.path.join(S, 'b1', 'target.txt')
SRC = os.environ.get('B1SRC', os.path.join(S, 'hb-b1.cpp'))
# CodeWarrior derives the __sinit symbol from the source file name, so the
# draft MUST be compiled from a file called d_a_en_hatena_balloon.cpp or the
# __sinit body cannot be looked up at all (it silently reads DRAFT MISSING).
CSRC = os.path.join(W, 'd_a_en_hatena_balloon.cpp')
OBJ = os.path.join(W, 'b1.o')
DIS = os.path.join(W, 'b1.txt')

SIZES = {
    'daEnHatenaBalloon_c_classInit__Fv': 0x160,
    'create__19daEnHatenaBalloon_cFv': 0x308,
    'doDelete__19daEnHatenaBalloon_cFv': 0x8,
    'block_hit_init__19daEnHatenaBalloon_cFv': 0x4,
    'isQuakeDamage__19daEnHatenaBalloon_cFv': 0x8,
    '__dt__19daEnHatenaBalloon_cFv': 0xF8,
    '__sinit_\\d_a_en_hatena_balloon_cpp': 0x4F8,
    '__dt__34sFStateID_c<19daEnHatenaBalloon_c>Fv': 0x58,
    'isSameName__34sFStateID_c<19daEnHatenaBalloon_c>CFPCc': 0x88,
    'initializeState__34sFStateID_c<19daEnHatenaBalloon_c>CFR19daEnHatenaBalloon_c': 0x30,
    'executeState__34sFStateID_c<19daEnHatenaBalloon_c>CFR19daEnHatenaBalloon_c': 0x30,
    'finalizeState__34sFStateID_c<19daEnHatenaBalloon_c>CFR19daEnHatenaBalloon_c': 0x30,
}
ORDER = list(SIZES)


def norm(lines):
    # dtk quotes a symbol whose spelling needs it. The target's .bss anchor is
    # `"@77306_803753F8"` (quoted) where a fresh object's is `...bss.0`
    # (unquoted); canonicalise() turns BOTH into SYM0 but leaves the quotes, so
    # an otherwise identical instruction reads as a difference. Quoting is a
    # property of the spelling, which canonicalise has already erased, so
    # dropping it symmetrically cannot hide a real mismatch.
    return [l.replace('"', '') for l in H.canonicalise(lines)]


def cmp_fn(name):
    a, b = H.extract(TGT, name), H.extract(DIS, name)
    if a is None or b is None:
        return None, a, b
    return norm(a) == norm(b), a, b


def build():
    shutil.copyfile(SRC, CSRC)
    ok, log = H.compile_draft(CSRC, OBJ)
    if not ok:
        print('COMPILE FAILED\n' + log[:8000]); return False
    if log.strip():
        print('compiler notes:\n' + log[:2000])
    ok, log = H.disasm(OBJ, DIS)
    if not ok:
        print('DISASM FAILED\n' + log[:3000]); return False
    return True


def main():
    if not build():
        return 1
    only = [a for a in sys.argv[1:] if not a.startswith('-')] or ORDER
    full = '-v' in sys.argv
    nmatch = 0
    for n in only:
        want = H.extract(TGT, n)
        got = H.extract(DIS, n)
        if want is None:
            print('%-72s TARGET MISSING' % n[:72]); continue
        exp = SIZES.get(n)
        if exp is not None and len(want) * 4 != exp:
            print('%-72s !! COMPARATOR BROKEN: %d insns = %d B, map says %d B'
                  % (n[:72], len(want), len(want) * 4, exp))
            continue
        if got is None:
            print('%-72s DRAFT MISSING' % n[:72]); continue
        m, _, _ = cmp_fn(n)
        if m:
            nmatch += 1
            print('%-72s MATCH (%d insns, %d B)' % (n[:72], len(want), len(want) * 4))
        else:
            na, nb = norm(want), norm(got)
            ndiff = sum(1 for i in range(max(len(na), len(nb)))
                        if (na[i] if i < len(na) else None) != (nb[i] if i < len(nb) else None))
            print('%-72s DIFF (target %d, draft %d insns, %d differing)'
                  % (n[:72], len(want), len(got), ndiff))
            if full:
                for i in range(max(len(na), len(nb))):
                    x = na[i] if i < len(na) else ''
                    y = nb[i] if i < len(nb) else ''
                    if x != y:
                        print('  %4d | want: %-50s got: %s' % (i, x[:50], y[:50]))
    print('\n%d/%d matched' % (nmatch, len(only)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
