"""Focused probe for the two e1 residuals.

Each variant is a list of (old, new) literal substitutions on a copy of
wip/fix_bigtwo/d_line_mng.cpp.  Reports, per variant:
  * matched-function count (byte-equal, name-keyed) and any LOST/GAIN
  * differing-word count for the watchlist
  * the offending instruction pairs for the two study functions

Usage:  python probe.py --list
        python probe.py <name> ...        (add --show for instruction diffs)
        python probe.py --group T1|T2
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
TGT = parse(TARGET)

WATCH = ['initializeState_Right45', 'executeState_Right45',
         'executeState_Left30Right', 'executeState_Left30Left',
         'executeState_Right30Right', 'executeState_Right60Up',
         'executeState_Left45', 'executeState_Right30Left']
STUDY = ['initializeState_Right45__10dLineMng_cFv',
         'executeState_Right45__10dLineMng_cFv',
         'executeState_Left30Right__10dLineMng_cFv']


def bl(fns, k):
    return [b for _, b, _ in fns.get(k, [])]


def mset(fns):
    return {k for k in TGT if TGT[k] and bl(fns, k) == bl(TGT, k)}


BASE_M = None


def build(name, subs):
    src = open(BASE, encoding='utf-8').read()
    for old, new in subs:
        n = src.count(old)
        if n == 0:
            print('=== %-26s  !! pattern not found: %r' % (name, old[:60]))
            return None
        src = src.replace(old, new)
    out = os.path.join(HERE, 'v_%s.cpp' % name)
    open(out, 'w', encoding='utf-8').write(src)
    obj = os.path.join(HERE, 'v_%s.o' % name)
    ok, log = harness.compile_draft(out, obj, extra_inc=[INC])
    if not ok:
        print('=== %-26s  COMPILE FAILED\n%s' % (name, log[-900:]))
        return None
    txt = os.path.join(HERE, 'v_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print('=== %-26s  DISASM FAILED' % name)
        return None
    return parse(txt)


def nd(fns, sub):
    k = [x for x in TGT if x.startswith(sub + '__')]
    if not k:
        return '?'
    k = k[0]
    tb, db = bl(TGT, k), bl(fns, k)
    if len(tb) != len(db):
        return 'L%+d' % (len(db) - len(tb))
    return str(sum(1 for a, b in zip(tb, db) if a != b))


def report(name, fns, show=False):
    global BASE_M
    ms = mset(fns)
    line = '  '.join('%s=%s' % (w.replace('State_', ''), nd(fns, w)) for w in WATCH)
    print('=== %-26s m=%-3d  %s' % (name, len(ms), line))
    if BASE_M is not None:
        lost = sorted(BASE_M - ms)
        gain = sorted(ms - BASE_M)
        if lost:
            print('    LOST: ' + ', '.join(x.split('__')[0] for x in lost))
        if gain:
            print('    GAIN: ' + ', '.join(x.split('__')[0] for x in gain))
    if show:
        for k in STUDY:
            tb, db = TGT[k], fns.get(k, [])
            out = []
            for i in range(max(len(tb), len(db))):
                t = tb[i] if i < len(tb) else ('', '--', '--')
                d = db[i] if i < len(db) else ('', '--', '--')
                if t[1] != d[1]:
                    out.append('      %3d  T %-32s | D %s' % (i, t[2], d[2]))
            if out:
                print('    %s' % k.split('__')[0])
                print('\n'.join(out))
    return ms


# --------------------------------------------------------------------------
# anchors
R45I = '''    mAngle = 0xA000;
    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);
}'''
R45E = '''    mPos.x += mSpeed.x;
    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);
    if (check_term()) {'''

# the Left30Right branch-site half-product (unique: 8.0f variant of the file)
L30R_HEAD = '''    mSpeed.x = mBaseSpeed;
    mSpeed.x *= 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;
    mPos.y = (mUnitBasePos.y - 8.0f) + 0.5f * (mPos.x - mUnitBasePos.x);
    if (check_term()) {
        mPos = old;
        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;
    } else if (mPos.x < mUnitBasePos.x) {
        mVec2_c newBase = mUnitBasePos;
        newBase.x -= 16.0f;'''


def l30r(head_half, branch_half):
    return (L30R_HEAD, L30R_HEAD
            .replace('    mSpeed.y = 0.5f * mSpeed.x;\n    mPos.x',
                     '    %s\n    mPos.x' % head_half)
            .replace('        mSpeed.y = 0.5f * mSpeed.x;\n    } else',
                     '        %s\n    } else' % branch_half))


V = {}
GROUP = {}

# ---------------- T1: Left30Right half-product slot order -----------------
def t1(name, head, branch):
    V[name] = [l30r(head, branch)]
    GROUP.setdefault('T1', []).append(name)


t1('T1_ctl', 'mSpeed.y = 0.5f * mSpeed.x;', 'mSpeed.y = 0.5f * mSpeed.x;')
t1('T1_commute_branch', 'mSpeed.y = 0.5f * mSpeed.x;', 'mSpeed.y = mSpeed.x * 0.5f;')
t1('T1_div_branch', 'mSpeed.y = 0.5f * mSpeed.x;', 'mSpeed.y = mSpeed.x / 2.0f;')
t1('T1_div_both', 'mSpeed.y = mSpeed.x / 2.0f;', 'mSpeed.y = mSpeed.x / 2.0f;')
t1('T1_compound_branch', 'mSpeed.y = 0.5f * mSpeed.x;',
   'mSpeed.y = mSpeed.x;\n        mSpeed.y *= 0.5f;')
t1('T1_halfbase_branch', 'mSpeed.y = 0.5f * mSpeed.x;',
   'mSpeed.y = 0.5f * mSpeed.x;')

# ---------------- T2: Right45 y-expression -------------------------------
def t2(name, initbody, execbody=None):
    subs = [(R45I, '    mAngle = 0xA000;\n%s\n}' % initbody)]
    if execbody is not None:
        subs.append((R45E, '    mPos.x += mSpeed.x;\n%s\n    if (check_term()) {' % execbody))
    V[name] = subs
    GROUP.setdefault('T2', []).append(name)


t2('T2_ctl', '    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);')
t2('T2_neg_first', '    mPos.y = -(mPos.x - mUnitBasePos.x) + mUnitBasePos.y;')
t2('T2_baseref', '    const mVec2_c &b = mUnitBasePos;\n'
                 '    mPos.y = b.y - (mPos.x - b.x);')
t2('T2_ylocal', '    f32 by = mUnitBasePos.y;\n'
                '    mPos.y = by - (mPos.x - mUnitBasePos.x);')
t2('T2_selfassign', '    mPos.y = mUnitBasePos.y;\n'
                    '    mPos.y -= (mPos.x - mUnitBasePos.x);')
t2('T2_dxlocal', '    f32 dx = mPos.x - mUnitBasePos.x;\n'
                 '    mPos.y = mUnitBasePos.y - dx;')
t2('T2_nested', '    mPos.y = mUnitBasePos.y - mPos.x + mUnitBasePos.x;')
t2('T2_addneg', '    mPos.y = mUnitBasePos.y + (mUnitBasePos.x - mPos.x);')

SELF = '    mPos.y = mUnitBasePos.y;\n    mPos.y -= (mPos.x - mUnitBasePos.x);'
SELF_NP = '    mPos.y = mUnitBasePos.y;\n    mPos.y -= mPos.x - mUnitBasePos.x;'
YLOC = '    f32 by = mUnitBasePos.y;\n    mPos.y = by - (mPos.x - mUnitBasePos.x);'
PLAIN = '    mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x);'

t2('T2b_selfassign_both', SELF, SELF)
t2('T2b_ylocal_both', YLOC, YLOC)
t2('T2b_exec_only', PLAIN, SELF)
t2('T2b_noparen_both', SELF_NP, SELF_NP)
t2('T2b_ylocal_exec_only', PLAIN, YLOC)


def main():
    global BASE_M
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    show = '--show' in sys.argv
    if '--list' in sys.argv or (not args and '--group' not in ' '.join(sys.argv)):
        print('\n'.join(V))
        return
    if '--group' in sys.argv:
        g = sys.argv[sys.argv.index('--group') + 1]
        args = GROUP[g]
    ctl = build('control', [])
    BASE_M = mset(ctl)
    report('control', ctl, show)
    for n in args:
        f = build(n, V[n])
        if f is not None:
            report(n, f, show)


if __name__ == '__main__':
    main()
