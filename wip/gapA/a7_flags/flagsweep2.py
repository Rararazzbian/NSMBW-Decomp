"""A7 round 2: only flags this compiler (Freescale EPPC 4.3 b151) documents.

Classifies each variant's lines 2/10/33/34 as FLIPPED (retail order),
SAME (identical to the baseline draft residual), or OTHER (printed in full).
"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
from flagsweep import parse, SRC, INC, TARGET, FN, KEY, compile_with, BASE, sub, solo

VARIANTS = []
def V(l, f): VARIANTS.append((l, f))

V('BASELINE', list(BASE))
# documented -opt keywords not covered in round 1
for k in ('speed', 'space', 'nospeed', 'nospace', 'nodeadstore', 'nodead',
          'level=0', 'level=1', 'level=2', 'level=3', 'level=4',
          'on', 'all', 'full', 'off',
          'level=4,peephole', 'level=4,peephole,schedule',
          'level=4,peephole,schedule,autoinline',
          'level=4,peephole,noschedule', 'speed,level=4,peephole,schedule',
          'space,level=4,peephole,schedule'):
    V('-opt %s' % k, BASE + ['-opt', k])
# scheduling, standalone flag (documented: default off; -O4 turns it on)
V('-schedule off', BASE + ['-schedule', 'off'])
V('-schedule on', BASE + ['-schedule', 'on'])
# ieee / fsel / fp compare semantics
V('-strict_ieee', BASE + ['-strict_ieee'])
V('-relax_ieee', BASE + ['-relax_ieee'])
V('-norelax_ieee', BASE + ['-norelax_ieee'])
V('-use_fsel on', BASE + ['-use_fsel', 'on'])
V('-use_fsel off', BASE + ['-use_fsel', 'off'])
V('-gen-fsel', BASE + ['-gen-fsel'])
V('-no-gen-fsel', BASE + ['-no-gen-fsel'])
V('-ordered-fp-compares', BASE + ['-ordered-fp-compares'])
V('-no-ordered-fp-compares', BASE + ['-no-ordered-fp-compares'])
# data pooling / placement
V('-pool on', BASE + ['-pool', 'on'])
V('-pool off', BASE + ['-pool', 'off'])
V('-pooldata off', BASE + ['-pooldata', 'off'])
V('-sdatathreshold 0', BASE + ['-sdatathreshold', '0'])
V('-sdata2 4', BASE + ['-sdata2', '4'])
V('-sdata2 16', BASE + ['-sdata2', '16'])
V('-model absolute', BASE + ['-model', 'absolute'])
V('-abi eabi', BASE + ['-abi', 'eabi'])
# inline depth / direction
for k in ('level=0', 'level=1', 'level=2', 'level=8', 'bottomup', 'nobottomup',
          'smart', 'noauto,nobottomup', 'noauto,bottomup'):
    V('-inline %s' % k, sub(BASE, '-inline', k))
# ipa remaining
V('-ipa program-final', sub(BASE, '-ipa', 'program-final'))
# other codegen knobs
V('-profile on', BASE + ['-profile', 'on'])
V('-align mac68k', BASE + ['-align', 'mac68k'])
V('-align powerpc,array', BASE + ['-align', 'powerpc,array'])
V('-bool on', BASE + ['-bool', 'on'])
V('-bool off', BASE + ['-bool', 'off'])
V('-nolonglong', BASE + ['-nolonglong'])
V('-str readonly', BASE + ['-str', 'readonly'])
V('-str pool', BASE + ['-str', 'pool'])
V('-noaltivec_move_block', BASE + ['-noaltivec_move_block'])
V('-g (debug info)', BASE + ['-g'])
V('-sym on', BASE + ['-sym', 'on'])
V('-proc generic', solo(BASE, 'gekko', 'generic'))
V('-proc 750cl', solo(BASE, 'gekko', '750cl'))
V('-proc 603e', solo(BASE, 'gekko', '603e'))
V('-enc UTF8', sub(BASE, '-enc', 'UTF8'))
V('-dialect cplus', BASE + ['-dialect', 'cplus'])
V('-relax_pointers', BASE + ['-relax_pointers'])
# combined: the two that are most plausibly "this TU was built differently"
V('-O4,p -schedule off', solo(BASE, '-O4', '-O4,p') + ['-schedule', 'off'])
V('-O4,p -fp fmadd', sub(solo(BASE, '-O4', '-O4,p'), '-fp', 'fmadd'))
V('-O4 -opt nocse,nopropagation', BASE + ['-opt', 'nocse,nopropagation'])


def main():
    tgt_all = parse(TARGET)
    tgt = [t for _, t in tgt_all[FN]]
    tgt_can = harness.canonicalise(tgt)
    obj, txt = os.path.join(HERE, 'w.o'), os.path.join(HERE, 'w.txt')

    base_key = None
    rows = []
    for label, flags in VARIANTS:
        ok, log = compile_with(flags, obj)
        if not ok:
            err = next((l.strip() for l in log.splitlines() if 'rror' in l),
                       (log.strip().splitlines() or ['?'])[0])
            rows.append((label, 'REJECTED', '-', '-', '-', err[:60]))
            continue
        if not harness.disasm(obj, txt)[0]:
            rows.append((label, 'yes', 'disasm-fail', '-', '-', ''))
            continue
        got_all = parse(txt)
        if FN not in got_all:
            rows.append((label, 'yes', 'fn-missing', '-', '-', ''))
            continue
        got = [t for _, t in got_all[FN]]
        gcan = harness.canonicalise(got)
        n = len(got)
        if n != len(tgt):
            state = 'n/a len %d' % n
        else:
            k = tuple(gcan[i] for i in KEY)
            if base_key is None:
                base_key = k
            if all(gcan[i] == tgt_can[i] for i in KEY):
                state = 'FLIPPED->retail'
            elif k == base_key:
                state = 'same as draft'
            else:
                state = 'OTHER'
                print('  OTHER key lines under %s:' % label)
                for i in KEY:
                    print('    %3d  %s' % (i, gcan[i]))
        full = 'EXACT' if gcan == tgt_can else 'differs'
        nm = tot = 0
        for name, body in tgt_all.items():
            if name.startswith('gap_'):
                continue
            tot += 1
            g = got_all.get(name)
            if g and harness.canonicalise([t for _, t in g]) == \
                     harness.canonicalise([t for _, t in body]):
                nm += 1
        rows.append((label, 'yes', str(n), state, full, '%d/%d unit' % (nm, tot)))

    w = max(len(r[0]) for r in rows)
    print('\n%-*s  %-9s %-11s %-16s %-8s %s' % (w, 'FLAG VARIED', 'ACCEPTED', 'INSNS',
                                                'KEY 2/10/33/34', 'FN', 'NOTE'))
    for r in rows:
        print('%-*s  %-9s %-11s %-16s %-8s %s' % (w, r[0], r[1], r[2], r[3], r[4], r[5]))


main()
