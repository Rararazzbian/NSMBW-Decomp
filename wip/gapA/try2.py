"""Second Gap A residual: the mBaseSpeed / 0.8910065f register permutation.

With the newBase reload solved, executeState_Left30Left is length-exact and the
ONLY remaining difference is that retail holds mBaseSpeed in f1 and the constant
in f0, while the draft holds them the other way round. The multiply itself is
emitted identically (`fmuls f6, f1, f0`) -- so the operands are reaching the
allocator in the opposite order.

Substitutes whole function bodies into wip/gapA/gapA_all.cpp (which already has
the newBase fix) and diffs.

Usage:  python try2.py <variant>   |   python try2.py --list
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(HERE, 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'executeState_Left30Left__10dLineMng_cFv'

HEAD = '''    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;'''
BRANCH = '''        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;'''

VARIANTS = {
    'control': (HEAD, BRANCH),

    # R1: constant first in the product.
    'const_first': ('''    mSpeed.x = 0.8910065f * mBaseSpeed;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;''',
                    '''        mSpeed.x = 0.8910065f * mBaseSpeed;
        mSpeed.y = 0.5f * mSpeed.x;'''),

    # R2: half-product written variable-first.
    'half_swap': ('''    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = mSpeed.x * 0.5f;
    mPos.x += mSpeed.x;''',
                  '''        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = mSpeed.x * 0.5f;'''),

    # R3: both.
    'both_swap': ('''    mSpeed.x = 0.8910065f * mBaseSpeed;
    mSpeed.y = mSpeed.x * 0.5f;
    mPos.x += mSpeed.x;''',
                  '''        mSpeed.x = 0.8910065f * mBaseSpeed;
        mSpeed.y = mSpeed.x * 0.5f;'''),

    # R4: a named local for the product -- one evaluation, two uses.
    'local_speed': ('''    f32 sp = mBaseSpeed * 0.8910065f;
    mSpeed.x = sp;
    mSpeed.y = 0.5f * sp;
    mPos.x += mSpeed.x;''',
                    '''        f32 sp = mBaseSpeed * 0.8910065f;
        mSpeed.x = sp;
        mSpeed.y = 0.5f * sp;'''),
}


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


def main():
    name = sys.argv[1]
    if name == '--list':
        print('\n'.join(VARIANTS))
        return
    head, branch = VARIANTS[name]

    src = open(BASE, encoding='utf-8').read()
    # Localise the edit: only executeState_Left30Left, not its seven siblings.
    start = src.index('void dLineMng_c::executeState_Left30Left()')
    end = src.index('void dLineMng_c::initializeState_Left30Right()')
    body = src[start:end]
    if body.count(HEAD) != 1 or body.count(BRANCH) != 1:
        sys.exit('anchor not unique inside the function')
    body = body.replace(HEAD, head).replace(BRANCH, branch)
    out_src = os.path.join(HERE, 'r_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(src[:start] + body + src[end:])

    obj = os.path.join(HERE, 'r_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        sys.exit('COMPILE FAILED\n' + log[-3000:])
    txt = os.path.join(HERE, 'r_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        sys.exit('DISASM FAILED\n' + log[-2000:])

    draft, target = parse(txt).get(FN), parse(TARGET).get(FN)
    print('variant  : %s' % name)
    print('target   : %d   draft: %d  (%+d)' % (len(target), len(draft), len(draft) - len(target)))
    if harness.canonicalise([t for _, t in draft]) == harness.canonicalise([t for _, t in target]):
        print('RESULT   : canonically EQUAL')
        return
    print('RESULT   : differs')
    n = max(len(draft), len(target))
    shown = 0
    for i in range(n):
        t = target[i][1] if i < len(target) else '--'
        d = draft[i][1] if i < len(draft) else '--'
        if t != d:
            print('%-4d %-38s %s' % (i, t, d))
            shown += 1
            if shown > 30:
                print('... truncated')
                break


main()
