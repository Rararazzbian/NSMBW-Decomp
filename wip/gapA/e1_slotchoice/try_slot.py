"""Variant driver for the e1 slot-choice / FP-register-assignment residuals.

A variant is a list of (old, new) literal substitutions applied to a copy of
wip/fix_bigtwo/d_line_mng.cpp.  Each is compiled as a whole TU, disassembled,
and reported as:
  * per-function differing-word counts for a watchlist
  * a global regression check against the baseline matched-set
Nothing outside this directory is written.

Usage:
    python try_slot.py --list
    python try_slot.py <name> [<name> ...]
    python try_slot.py --all
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
sys.path.insert(0, HERE)
import harness
from dump import parse

BASE = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'd_line_mng.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')

WATCH = [
    'initializeState_Right45__10dLineMng_cFv',
    'executeState_Right45__10dLineMng_cFv',
    'executeState_Left30Right__10dLineMng_cFv',
    'executeState_Left45__10dLineMng_cFv',
    'initializeState_Left45__10dLineMng_cFv',
    'executeState_Left30Left__10dLineMng_cFv',
    'executeState_Right30Right__10dLineMng_cFv',
    'executeState_Right60Up__10dLineMng_cFv',
    'start_line_move__10dLineMng_cFv',
]

TGT = parse(TARGET)


def bytes_of(fns, k):
    return [b for _, b, _ in fns.get(k, [])]


def matched_set(fns):
    """Byte-equal name-keyed matches (a conservative, stable regression gate)."""
    return {k for k in TGT if bytes_of(fns, k) == bytes_of(TGT, k) and TGT[k]}


BASE_MATCHED = None


def build(name, subs, verbose=True):
    src = open(BASE, encoding='utf-8').read()
    for old, new in subs:
        if old not in src:
            print('  !! substitution target not found: %r' % old[:70])
            return None
        if src.count(old) != subs_count(subs, old):
            pass
        src = src.replace(old, new)
    out = os.path.join(HERE, 'v_%s.cpp' % name)
    open(out, 'w', encoding='utf-8').write(src)
    obj = os.path.join(HERE, 'v_%s.o' % name)
    ok, log = harness.compile_draft(out, obj, extra_inc=[INC])
    if not ok:
        print('=== %s : COMPILE FAILED\n%s' % (name, log[-1200:]))
        return None
    txt = os.path.join(HERE, 'v_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print('=== %s : DISASM FAILED' % name)
        return None
    return parse(txt)


def subs_count(subs, old):
    return 1


def report(name, fns):
    ms = matched_set(fns)
    global BASE_MATCHED
    lost = sorted(BASE_MATCHED - ms) if BASE_MATCHED else []
    gained = sorted(ms - BASE_MATCHED) if BASE_MATCHED else []
    bits = []
    for k in WATCH:
        tb, db = bytes_of(TGT, k), bytes_of(fns, k)
        if len(tb) != len(db):
            bits.append('%s=LEN%+d' % (k.split('__')[0], len(db) - len(tb)))
        else:
            nd = sum(1 for a, b in zip(tb, db) if a != b)
            bits.append('%s=%d' % (k.split('__')[0], nd))
    print('=== %-28s  matched=%d' % (name, len(ms)))
    print('    ' + '  '.join(bits))
    if lost:
        print('    LOST: ' + ', '.join(x.split('__')[0] for x in lost))
    if gained:
        print('    GAIN: ' + ', '.join(x.split('__')[0] for x in gained))
    return ms


def show(name, fns, fnsub):
    ks = [k for k in TGT if fnsub in k]
    for k in ks:
        tb = TGT[k]
        db = fns.get(k, [])
        print('--- %s  T%dw D%dw' % (k, len(tb), len(db)))
        for i in range(max(len(tb), len(db))):
            t = tb[i][2] if i < len(tb) else '--'
            tw = tb[i][1] if i < len(tb) else '--'
            d = db[i][2] if i < len(db) else '--'
            dw = db[i][1] if i < len(db) else '--'
            if tw != dw:
                print('  >>> %3d  T %-34s | D %s' % (i, t, d))


# ---------------------------------------------------------------------------
# The two source statements under study.
R45_INIT_OLD = '''    mAngle = 0xA000;
    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);
}'''

R45_EXEC_OLD = '''    mPos.x += mSpeed.x;
    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);
    if (check_term()) {'''

VARIANTS = {}
VARIANTS['control'] = []

# S1: hoist the inner difference into a named local (init site only)
VARIANTS['S1_init_tmp'] = [(R45_INIT_OLD, '''    mAngle = 0xA000;
    f32 dx = mPos.x - mUnitBasePos.x;
    mPos.y = mUnitBasePos.y - dx;
}''')]

# S2: same, both sites
VARIANTS['S2_both_tmp'] = [
    (R45_INIT_OLD, '''    mAngle = 0xA000;
    f32 dx = mPos.x - mUnitBasePos.x;
    mPos.y = mUnitBasePos.y - dx;
}'''),
    (R45_EXEC_OLD, '''    mPos.x += mSpeed.x;
    f32 dx = mPos.x - mUnitBasePos.x;
    mPos.y = mUnitBasePos.y - dx;
    if (check_term()) {'''),
]

# S3: compound assignment form on the member
VARIANTS['S3_init_compound'] = [(R45_INIT_OLD, '''    mAngle = 0xA000;
    mPos.y = mPos.x - mUnitBasePos.x;
    mPos.y = mUnitBasePos.y - mPos.y;
}''')]

# S4: extra parens (no-op sanity check that spelling alone does nothing)
VARIANTS['S4_init_parens'] = [(R45_INIT_OLD, '''    mAngle = 0xA000;
    mPos.y = (mUnitBasePos.y) - ((mPos.x) - (mUnitBasePos.x));
}''')]


def main():
    global BASE_MATCHED
    args = sys.argv[1:]
    if not args or args[0] == '--list':
        print('\n'.join(VARIANTS))
        return
    ctl = build('control', [])
    BASE_MATCHED = matched_set(ctl)
    report('control', ctl)
    names = list(VARIANTS) if args[0] == '--all' else args
    for n in names:
        if n == 'control':
            continue
        f = build(n, VARIANTS[n])
        if f is None:
            continue
        report(n, f)
        if '--show' in args:
            show(n, f, 'State_Right45')


if __name__ == '__main__':
    main()
