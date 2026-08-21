"""Angle A3: live ranges / use position for the mBaseSpeed-vs-0.8910065f swap.

Retail:  member -> f1 (emitted first at the branch site), const -> f0.
Draft :  const  -> f1 (emitted first at the branch site), member -> f0.

Two sites: head (0x60(r3), const hoisted to idx 2) and branch (0x60(r30)).

This driver substitutes a WHOLE replacement body for
dLineMng_c::executeState_Left30Left into a copy of wip/gapA/gapA_all.cpp,
slicing between that function and initializeState_Left30Right so the eight
sibling functions are untouched.  Output files live in this directory only.

Usage:  python try_live.py <variant>  |  python try_live.py --list
        python try_live.py --all
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'executeState_Left30Left__10dLineMng_cFv'

START = 'void dLineMng_c::executeState_Left30Left()'
END = 'void dLineMng_c::initializeState_Left30Right()'

# The tail of the function (the >= 16.0f branch) is invariant across every
# variant below, so keep it in one place.
TAIL = '''    } else if (mPos.x < mUnitBasePos.x) {
        mov_frm_leftlower(mUnitBasePos, false);
    } else if (mPos.x >= mUnitBasePos.x + 16.0f) {
        mVec2_c newBase = mUnitBasePos;
        newBase.x += 16.0f;
        u32 lineUnitNo = getLineUnitNo(newBase.x, newBase.y);
        if (lineUnitNo == 8) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Left30Right);
        } else if (lineUnitNo == 0xA) {
            mUnitBasePos.x = newBase.x;
            mStateMgr.changeState(StateID_Right30Right);
        } else {
            mStateMgr.changeState(StateID_FallDown);
        }
    }
}
'''


def fn(head_decls, head_pair, branch_pair, extra_mid='', tail_extra=None):
    """Assemble a full function body from the varying pieces."""
    t = TAIL if tail_extra is None else tail_extra
    return (START + ' {\n'
            + head_decls
            + head_pair
            + extra_mid
            + '    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5f * (mPos.x - mUnitBasePos.x);\n'
            + '    if (check_term()) {\n'
            + '        mPos = old;\n'
            + branch_pair
            + t + '\n\n')


CTL_DECLS = '    mVec2_c old = mPos;\n'
CTL_HEAD = ('    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
            '    mSpeed.y = 0.5f * mSpeed.x;\n'
            '    mPos.x += mSpeed.x;\n')
CTL_BRANCH = ('        mSpeed.x = mBaseSpeed * 0.8910065f;\n'
              '        mSpeed.y = 0.5f * mSpeed.x;\n')

VARIANTS = {}

# --- 0. control -------------------------------------------------------------
VARIANTS['control'] = fn(CTL_DECLS, CTL_HEAD, CTL_BRANCH)

# --- 1. read mBaseSpeed into a local at the very top, use it at both sites ---
VARIANTS['L1_toplocal'] = fn(
    '    f32 base = mBaseSpeed;\n    mVec2_c old = mPos;\n',
    '    mSpeed.x = base * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = base * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1b. local declared after `old`, i.e. later start of live range ---------
VARIANTS['L1b_localafterold'] = fn(
    '    mVec2_c old = mPos;\n    f32 base = mBaseSpeed;\n',
    '    mSpeed.x = base * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = base * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1c. separate local per site (short live ranges, still "a local") -------
VARIANTS['L1c_perstlocal'] = fn(
    '    mVec2_c old = mPos;\n',
    '    f32 base = mBaseSpeed;\n'
    '    mSpeed.x = base * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        f32 base2 = mBaseSpeed;\n'
    '        mSpeed.x = base2 * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1c2: L1c with the product written CONSTANT-FIRST -----------------------
VARIANTS['L1c2_constfirst'] = fn(
    '    mVec2_c old = mPos;\n',
    '    f32 base = mBaseSpeed;\n'
    '    mSpeed.x = 0.8910065f * base;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        f32 base2 = mBaseSpeed;\n'
    '        mSpeed.x = 0.8910065f * base2;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1c3: const-first product AND variable-first half -----------------------
VARIANTS['L1c3_constfirst_halfswap'] = fn(
    '    mVec2_c old = mPos;\n',
    '    f32 base = mBaseSpeed;\n'
    '    mSpeed.x = 0.8910065f * base;\n'
    '    mSpeed.y = mSpeed.x * 0.5f;\n'
    '    mPos.x += mSpeed.x;\n',
    '        f32 base2 = mBaseSpeed;\n'
    '        mSpeed.x = 0.8910065f * base2;\n'
    '        mSpeed.y = mSpeed.x * 0.5f;\n')

# --- 1c4: L1c shape, but the scale applied by compound assignment -----------
VARIANTS['L1c4_compound'] = fn(
    '    mVec2_c old = mPos;\n',
    '    f32 base = mBaseSpeed;\n'
    '    base *= 0.8910065f;\n'
    '    mSpeed.x = base;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        f32 base2 = mBaseSpeed;\n'
    '        base2 *= 0.8910065f;\n'
    '        mSpeed.x = base2;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1c5: L1c shape, compound assignment straight onto the member -----------
VARIANTS['L1c5_member_compound'] = fn(
    '    mVec2_c old = mPos;\n',
    '    mSpeed.x = mBaseSpeed;\n'
    '    mSpeed.x *= 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = mBaseSpeed;\n'
    '        mSpeed.x *= 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1c6: L1c but the local is declared const (does that re-fold it?) -------
VARIANTS['L1c6_constlocal'] = fn(
    '    mVec2_c old = mPos;\n',
    '    const f32 base = mBaseSpeed;\n'
    '    mSpeed.x = base * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        const f32 base2 = mBaseSpeed;\n'
    '        mSpeed.x = base2 * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 1c7: local at head site only; branch site keeps the direct member read -
VARIANTS['L1c7_head_only'] = fn(
    '    mVec2_c old = mPos;\n',
    '    f32 base = mBaseSpeed;\n'
    '    mSpeed.x = base * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    CTL_BRANCH)

# --- 1c8: branch site only --------------------------------------------------
VARIANTS['L1c8_branch_only'] = fn(
    CTL_DECLS, CTL_HEAD,
    '        f32 base2 = mBaseSpeed;\n'
    '        mSpeed.x = base2 * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 2. explicit this-> ------------------------------------------------------
VARIANTS['L2_thisptr'] = fn(
    CTL_DECLS,
    '    mSpeed.x = this->mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = this->mBaseSpeed * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 2b. this-> on both the member and the destination ----------------------
VARIANTS['L2b_thisall'] = fn(
    '    mVec2_c old = this->mPos;\n',
    '    this->mSpeed.x = this->mBaseSpeed * 0.8910065f;\n'
    '    this->mSpeed.y = 0.5f * this->mSpeed.x;\n'
    '    this->mPos.x += this->mSpeed.x;\n',
    '        this->mSpeed.x = this->mBaseSpeed * 0.8910065f;\n'
    '        this->mSpeed.y = 0.5f * this->mSpeed.x;\n')

# --- 3. a later, extra use of mBaseSpeed (live range extended past both) ----
#     PROBE ONLY -- changes semantics (writes mUnitBasePos.y).
_T3 = TAIL.replace('            mStateMgr.changeState(StateID_FallDown);',
                   '            mUnitBasePos.y += mBaseSpeed;\n'
                   '            mStateMgr.changeState(StateID_FallDown);')
VARIANTS['L3_lateruse_base'] = fn(CTL_DECLS, CTL_HEAD, CTL_BRANCH, tail_extra=_T3)

# --- 4. a later, extra use of mSpeed.x --------------------------------------
#     PROBE ONLY -- changes semantics.
_T4 = TAIL.replace('            mStateMgr.changeState(StateID_FallDown);',
                   '            mUnitBasePos.y += mSpeed.x;\n'
                   '            mStateMgr.changeState(StateID_FallDown);')
VARIANTS['L4_lateruse_speedx'] = fn(CTL_DECLS, CTL_HEAD, CTL_BRANCH, tail_extra=_T4)

# --- 5. y computed straight from mBaseSpeed (no dependence on mSpeed.x) -----
VARIANTS['L5_y_from_base'] = fn(
    CTL_DECLS,
    '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * (mBaseSpeed * 0.8910065f);\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '        mSpeed.y = 0.5f * (mBaseSpeed * 0.8910065f);\n')

# --- 5b. y first, then x (reversed store order, same values) ----------------
VARIANTS['L5b_y_first'] = fn(
    CTL_DECLS,
    '    mSpeed.y = 0.5f * (mBaseSpeed * 0.8910065f);\n'
    '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.y = 0.5f * (mBaseSpeed * 0.8910065f);\n'
    '        mSpeed.x = mBaseSpeed * 0.8910065f;\n')

# --- 6. move the pair BEFORE `old = mPos` -----------------------------------
VARIANTS['L6_pair_first'] = fn(
    '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mVec2_c old = mPos;\n',
    '    mPos.x += mSpeed.x;\n',
    CTL_BRANCH)

# --- 6b. move the pair AFTER the mPos.x update (x add reads it, so split) ---
#     mPos.x += mBaseSpeed*k first, then the stores.  Same values.
VARIANTS['L6b_pair_after_posx'] = fn(
    CTL_DECLS,
    '    mPos.x += mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n',
    CTL_BRANCH)

# --- 6c. pair moved after the mPos.y line -----------------------------------
VARIANTS['L6c_pair_after_posy'] = fn(
    CTL_DECLS,
    '    mPos.x += mBaseSpeed * 0.8910065f;\n',
    CTL_BRANCH,
    extra_mid='')
VARIANTS['L6c_pair_after_posy'] = (
    START + ' {\n'
    '    mVec2_c old = mPos;\n'
    '    mPos.x += mBaseSpeed * 0.8910065f;\n'
    '    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5f * (mPos.x - mUnitBasePos.x);\n'
    '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    if (check_term()) {\n'
    '        mPos = old;\n'
    + CTL_BRANCH + TAIL + '\n\n')

# --- 7. duplicate the member read at the head site (two reads, one stmt) ----
VARIANTS['L7_dup_read'] = fn(
    CTL_DECLS,
    '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mBaseSpeed * 0.8910065f;\n',
    CTL_BRANCH)

# --- 7b. const in a named local (extends the CONST's live range) ------------
VARIANTS['L7b_const_local'] = fn(
    '    mVec2_c old = mPos;\n    const f32 k = 0.8910065f;\n',
    '    mSpeed.x = mBaseSpeed * k;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = mBaseSpeed * k;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 7c. both member and const in top locals --------------------------------
VARIANTS['L7c_both_locals'] = fn(
    '    f32 base = mBaseSpeed;\n    const f32 k = 0.8910065f;\n    mVec2_c old = mPos;\n',
    '    mSpeed.x = base * k;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = base * k;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')

# --- 7d. const local declared BEFORE the member local -----------------------
VARIANTS['L7d_k_before_base'] = fn(
    '    const f32 k = 0.8910065f;\n    f32 base = mBaseSpeed;\n    mVec2_c old = mPos;\n',
    '    mSpeed.x = base * k;\n'
    '    mSpeed.y = 0.5f * mSpeed.x;\n'
    '    mPos.x += mSpeed.x;\n',
    '        mSpeed.x = base * k;\n'
    '        mSpeed.y = 0.5f * mSpeed.x;\n')


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


def key_regs(ins):
    """Which FPR holds mBaseSpeed and which holds the 0.8910065f constant, at
    each of the two sites?  Identified structurally: find the `fmuls` whose
    sources are (a load of 0x60(rN)) and (an sda21 const load)."""
    out = []
    for i, t in enumerate(ins):
        m = re.match(r'fmuls f(\d+), f(\d+), f(\d+)$', t)
        if not m:
            continue
        d, a, b = m.groups()
        # walk backwards to find the defining load of each source
        def deffor(reg, upto):
            for j in range(upto - 1, -1, -1):
                mm = re.match(r'lfs f%s, (.*)$' % reg, ins[j])
                if mm:
                    return j, mm.group(1)
                if re.match(r'f(muls|adds|subs) f%s,' % reg, ins[j]):
                    return j, '<computed>'
            return None, '?'
        ja, sa = deffor(a, i)
        jb, sb = deffor(b, i)
        srcs = {sa: ('f' + a, ja), sb: ('f' + b, jb)}
        memk = [k for k in srcs if re.match(r'0x60\(r\d+\)', k)]
        conk = [k for k in srcs if '@sda21' in k]
        if memk and conk:
            out.append((i, srcs[memk[0]], srcs[conk[0]]))
    return out


def run(name):
    body = VARIANTS[name]
    src = open(BASE, encoding='utf-8').read()
    start = src.index(START)
    end = src.index(END)
    out_src = os.path.join(HERE, 'v_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(src[:start] + body + src[end:])

    obj = os.path.join(HERE, 'v_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        print('variant  : %s\nCOMPILE FAILED\n%s' % (name, log[-1500:]))
        return
    txt = os.path.join(HERE, 'v_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print('variant  : %s\nDISASM FAILED\n%s' % (name, log[-1500:]))
        return

    draft = [t for _, t in parse(txt).get(FN)]
    target = [t for _, t in parse(TARGET).get(FN)]
    eq = harness.canonicalise(draft) == harness.canonicalise(target)
    print('=== %s : target %d  draft %d (%+d)%s' %
          (name, len(target), len(draft), len(draft) - len(target),
           '   CANONICALLY EQUAL' if eq else ''))
    for tag, ins in (('TGT', target), ('DFT', draft)):
        for (i, (mr, mj), (cr, cj)) in key_regs(ins):
            print('   %s fmuls@%-3d member=%s(load@%s)  const=%s(load@%s)'
                  % (tag, i, mr, mj, cr, cj))
    if '--diff' in sys.argv and not eq:
        shown = 0
        for i in range(max(len(draft), len(target))):
            t = target[i] if i < len(target) else '--'
            d = draft[i] if i < len(draft) else '--'
            if t != d:
                print('   %-4d %-38s %s' % (i, t, d))
                shown += 1
                if shown > 40:
                    print('   ... truncated')
                    break


def main():
    a = sys.argv[1] if len(sys.argv) > 1 else '--list'
    if a == '--list':
        print('\n'.join(VARIANTS))
    elif a == '--all':
        for k in VARIANTS:
            run(k)
    else:
        run(a)


main()
