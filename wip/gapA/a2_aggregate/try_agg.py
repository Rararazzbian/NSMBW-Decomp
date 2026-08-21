"""Agent A2 -- aggregate / temporary store shapes for mSpeed.

Same driver structure as wip/gapA/try2.py, but with its own variant table and
its own output filenames so it cannot collide with the other agents.

Usage:  python try_agg.py <variant>   |   python try_agg.py --list
        python try_agg.py --all
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

HEAD = '''    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;'''
BRANCH = '''        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;'''


def ind(text, n):
    pad = ' ' * n
    return '\n'.join(pad + l if l.strip() else l for l in text.strip('\n').split('\n'))


# Each entry: (head-core, branch-core).  The head-core is emitted at 4 spaces
# and gets `mPos.x += mSpeed.x;` appended; the branch-core at 8 spaces.
CORES = {
    # --- baseline -------------------------------------------------------
    'control': ('''mSpeed.x = mBaseSpeed * 0.8910065f;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    # --- 1. construct and assign whole ----------------------------------
    'agg_ctor': ('''mSpeed = mVec2_c(mBaseSpeed * 0.8910065f, 0.5f * (mBaseSpeed * 0.8910065f));''', None),

    'agg_ctor_tmp': ('''f32 sx = mBaseSpeed * 0.8910065f;
mSpeed = mVec2_c(sx, 0.5f * sx);''', None),

    # --- 2. uninitialised temp, field-by-field, then aggregate assign ----
    'tmp_uninit': ('''mVec2_c s;
s.x = mBaseSpeed * 0.8910065f;
s.y = 0.5f * s.x;
mSpeed = s;''', None),

    # --- 3. the literal Gap A shape: copy-then-adjust --------------------
    'tmp_copy': ('''mVec2_c s = mSpeed;
s.x = mBaseSpeed * 0.8910065f;
s.y = 0.5f * s.x;
mSpeed = s;''', None),

    # --- 4. the set() method --------------------------------------------
    'set_dup': ('''mSpeed.set(mBaseSpeed * 0.8910065f, 0.5f * (mBaseSpeed * 0.8910065f));''', None),

    'set_tmp': ('''f32 sx = mBaseSpeed * 0.8910065f;
mSpeed.set(sx, 0.5f * sx);''', None),

    # --- 5. y computed before x -----------------------------------------
    'y_first': ('''mSpeed.y = 0.5f * (mBaseSpeed * 0.8910065f);
mSpeed.x = mBaseSpeed * 0.8910065f;''', None),

    'y_first_tmp': ('''f32 sx = mBaseSpeed * 0.8910065f;
mSpeed.y = 0.5f * sx;
mSpeed.x = sx;''', None),

    # --- 6. y from the temp rather than re-reading mSpeed.x -------------
    #        (local_speed was measured in try2.py; here it is the aggregate
    #         variant -- temp declared as an mVec2_c field pair.)
    'tmp_uninit_ydup': ('''mVec2_c s;
s.x = mBaseSpeed * 0.8910065f;
s.y = 0.5f * (mBaseSpeed * 0.8910065f);
mSpeed = s;''', None),

    # --- 7. y recomputed from mBaseSpeed directly ------------------------
    'y_folded_const': ('''mSpeed.x = mBaseSpeed * 0.8910065f;
mSpeed.y = mBaseSpeed * 0.44550325f;''', None),

    'y_reparen': ('''mSpeed.x = mBaseSpeed * 0.8910065f;
mSpeed.y = 0.5f * (mBaseSpeed * 0.8910065f);''', None),

    # --- extra: operator*= on the whole vector ---------------------------
    'op_scale': ('''mSpeed.x = mBaseSpeed;
mSpeed.y = 0.5f * mBaseSpeed;
mSpeed *= 0.8910065f;''', None),

    # --- batch 2: temporary for the MEMBER (not the product) -------------
    'base_tmp': ('''f32 bs = mBaseSpeed;
mSpeed.x = bs * 0.8910065f;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    'base_tmp_const': ('''f32 bs = mBaseSpeed;
mSpeed.x = 0.8910065f * bs;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    'base_tmp_ref': ('''const f32 &bs = mBaseSpeed;
mSpeed.x = bs * 0.8910065f;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    # named constant instead of an inline literal
    'named_const': ('''const f32 K = 0.8910065f;
mSpeed.x = mBaseSpeed * K;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    # aggregate written through a reference alias to mSpeed
    'ref_alias': ('''mVec2_c &s = mSpeed;
s.x = mBaseSpeed * 0.8910065f;
s.y = 0.5f * s.x;''', None),

    # whole-vector scalar-product operator, member-first inside the ctor
    'vec_mul': ('''mSpeed = mVec2_c(mBaseSpeed, 0.5f * mBaseSpeed) * 0.8910065f;''', None),

    # scalar temp for the product but stored to x BEFORE y is formed
    'store_x_then_y': ('''f32 sx = mBaseSpeed * 0.8910065f;
mSpeed.x = sx;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    # both fields from one temp, y re-reading the member field
    'set_from_field': ('''mSpeed.x = mBaseSpeed * 0.8910065f;
mSpeed.set(mSpeed.x, 0.5f * mSpeed.x);''', None),

    # --- batch 3: base_tmp is 2 lines off (fmuls operand order). Chase it. -
    'mul_eq_field': ('''mSpeed.x = mBaseSpeed;
mSpeed.x *= 0.8910065f;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    'mul_eq_tmp': ('''f32 sx = mBaseSpeed;
sx *= 0.8910065f;
mSpeed.x = sx;
mSpeed.y = 0.5f * sx;''', None),

    'base_tmp_muleq': ('''f32 bs = mBaseSpeed;
bs *= 0.8910065f;
mSpeed.x = bs;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    'static_k': ('''static const f32 K = 0.8910065f;
mSpeed.x = mBaseSpeed * K;
mSpeed.y = 0.5f * mSpeed.x;''',
                 '''mSpeed.x = mBaseSpeed * K;
mSpeed.y = 0.5f * mSpeed.x;'''),

    'base_tmp_static_k': ('''static const f32 K = 0.8910065f;
f32 bs = mBaseSpeed;
mSpeed.x = bs * K;
mSpeed.y = 0.5f * mSpeed.x;''',
                          '''f32 bs2 = mBaseSpeed;
mSpeed.x = bs2 * K;
mSpeed.y = 0.5f * mSpeed.x;'''),

    # temp for the member, but the product written const-first
    'base_tmp_ktmp': ('''f32 bs = mBaseSpeed;
f32 k = 0.8910065f;
mSpeed.x = bs * k;
mSpeed.y = 0.5f * mSpeed.x;''', None),

    # temp for the member, aggregate-assigned result
    'base_tmp_set': ('''f32 bs = mBaseSpeed;
mSpeed.set(bs * 0.8910065f, 0.5f * (bs * 0.8910065f));''', None),

    # temp for the member, y read back from the field (== base_tmp) but
    # with the mPos.x update reordered before y
    'base_tmp_ydup': ('''f32 bs = mBaseSpeed;
mSpeed.x = bs * 0.8910065f;
mSpeed.y = 0.5f * (bs * 0.8910065f);''', None),
}


def build(name):
    head_core, branch_core = CORES[name]
    if branch_core is None:
        branch_core = head_core
    head = ind(head_core, 4) + '\n    mPos.x += mSpeed.x;'
    branch = ind(branch_core, 8)
    return head, branch


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


def run(name, verbose=True):
    head, branch = build(name)

    src = open(BASE, encoding='utf-8').read()
    start = src.index('void dLineMng_c::executeState_Left30Left()')
    end = src.index('void dLineMng_c::initializeState_Left30Right()')
    body = src[start:end]
    if body.count(HEAD) != 1 or body.count(BRANCH) != 1:
        sys.exit('anchor not unique inside the function')
    body = body.replace(BRANCH, '@@B@@').replace(HEAD, '@@H@@')
    body = body.replace('@@H@@', head).replace('@@B@@', branch)
    out_src = os.path.join(HERE, 'a2_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(src[:start] + body + src[end:])

    obj = os.path.join(HERE, 'a2_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        print('variant  : %s\nRESULT   : COMPILE FAILED' % name)
        if verbose:
            print(log[-1500:])
        return None
    txt = os.path.join(HERE, 'a2_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print('variant  : %s\nRESULT   : DISASM FAILED' % name)
        return None

    draft, target = parse(txt).get(FN), parse(TARGET).get(FN)
    print('variant  : %s' % name)
    print('target   : %d   draft: %d  (%+d)' % (len(target), len(draft), len(draft) - len(target)))
    if harness.canonicalise([t for _, t in draft]) == harness.canonicalise([t for _, t in target]):
        print('RESULT   : canonically EQUAL  <<<<<<<<<<<<')
        return draft
    print('RESULT   : differs')
    n = max(len(draft), len(target))
    shown = 0
    for i in range(n):
        t = target[i][1] if i < len(target) else '--'
        d = draft[i][1] if i < len(draft) else '--'
        if t != d:
            print('%-4d %-38s %s' % (i, t, d))
            shown += 1
            if shown > 34:
                print('... truncated')
                break
    return draft


def main():
    a = sys.argv[1] if len(sys.argv) > 1 else '--list'
    if a == '--list':
        print('\n'.join(CORES))
        return
    if a == '--all':
        for k in CORES:
            run(k)
            print('-' * 70)
        return
    run(a)


main()
