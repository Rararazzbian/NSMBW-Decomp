"""Gap A angle A1: are 0.8910065f / 0.5f NAMED constants in the original source?

Same measurement rig as wip/gapA/try2.py, but each variant may also inject a
file-scope PRELUDE (a named-constant declaration) ahead of the function, and may
optionally patch a private copy of the shadow header (for the class-scope
static-const-member idiom, in the style of dLineMng_c::smc_UNIT_SIZE_X).

Nothing outside wip/gapA/a1_namedconst/ is written.

Usage:  python tryA1.py <variant>  |  python tryA1.py --list  |  python tryA1.py --all
"""
import os, re, shutil, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
ORIG_INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
MY_INC = os.path.join(HERE, 'shadow_include')          # private copy, patchable
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'executeState_Left30Left__10dLineMng_cFv'

HEAD = '''    mSpeed.x = mBaseSpeed * 0.8910065f;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;'''
BRANCH = '''        mSpeed.x = mBaseSpeed * 0.8910065f;
        mSpeed.y = 0.5f * mSpeed.x;'''

# The other two float literals inside this function's remaining body, kept as
# literals unless a variant says otherwise.
ANCHOR = 'void dLineMng_c::initializeState_Left30Left()'
CLASS_ANCHOR = '    static const float smc_UNIT_SIZE_X;'


def body(cos_expr, half_expr):
    """Build (head, branch) with the two constants spelled however asked."""
    h = ('    mSpeed.x = mBaseSpeed * %s;\n'
         '    mSpeed.y = %s * mSpeed.x;\n'
         '    mPos.x += mSpeed.x;') % (cos_expr, half_expr)
    b = ('        mSpeed.x = mBaseSpeed * %s;\n'
         '        mSpeed.y = %s * mSpeed.x;') % (cos_expr, half_expr)
    return h, b


def V(prelude, cos_expr, half_expr, hdr=None):
    h, b = body(cos_expr, half_expr)
    return {'prelude': prelude, 'head': h, 'branch': b, 'hdr': hdr}


VARIANTS = {}
VARIANTS['control'] = {'prelude': '', 'head': HEAD, 'branch': BRANCH, 'hdr': None}

# --- 1. file-scope static const f32 -----------------------------------------
VARIANTS['fs_cos'] = V('static const f32 kCos27 = 0.8910065f;\n\n',
                       'kCos27', '0.5f')
VARIANTS['fs_half'] = V('static const f32 kHalf = 0.5f;\n\n',
                        '0.8910065f', 'kHalf')
VARIANTS['fs_both'] = V('static const f32 kCos27 = 0.8910065f;\n'
                        'static const f32 kHalf = 0.5f;\n\n',
                        'kCos27', 'kHalf')

# --- 2. anonymous namespace --------------------------------------------------
VARIANTS['anon_cos'] = V('namespace { const f32 kCos27 = 0.8910065f; }\n\n',
                         'kCos27', '0.5f')
VARIANTS['anon_both'] = V('namespace {\nconst f32 kCos27 = 0.8910065f;\n'
                          'const f32 kHalf = 0.5f;\n}\n\n',
                          'kCos27', 'kHalf')

# --- 3. class-scope static const member, smc_UNIT_SIZE_X idiom ---------------
#   3a. declared in the header, DEFINED here (so the value is visible).
VARIANTS['cls_def_cos'] = V('const float dLineMng_c::smc_COS27 = 0.8910065f;\n\n',
                            'smc_COS27', '0.5f',
                            hdr='    static const float smc_COS27;\n')
VARIANTS['cls_def_both'] = V('const float dLineMng_c::smc_COS27 = 0.8910065f;\n'
                             'const float dLineMng_c::smc_HALF = 0.5f;\n\n',
                             'smc_COS27', 'smc_HALF',
                             hdr='    static const float smc_COS27;\n'
                                 '    static const float smc_HALF;\n')
#   3b. declared but NOT defined in this TU -- exactly smc_UNIT_SIZE_X's shape.
VARIANTS['cls_extern_cos'] = V('', 'smc_COS27', '0.5f',
                               hdr='    static const float smc_COS27;\n')
VARIANTS['cls_extern_both'] = V('', 'smc_COS27', 'smc_HALF',
                                hdr='    static const float smc_COS27;\n'
                                    '    static const float smc_HALF;\n')
#   3c. in-class initialiser (enum-style constant, folded at compile time).
VARIANTS['cls_inline_cos'] = V('', 'smc_COS27', '0.5f',
                               hdr='    static const float smc_COS27 = 0.8910065f;\n')

# --- 4. #define --------------------------------------------------------------
VARIANTS['def_cos'] = V('#define COS27 0.8910065f\n\n', 'COS27', '0.5f')
VARIANTS['def_both'] = V('#define COS27 0.8910065f\n#define HALF 0.5f\n\n',
                         'COS27', 'HALF')

# --- 5. const f32 local at the top of the function ---------------------------
VARIANTS['loc_cos'] = {
    'prelude': '',
    'head': '''    const f32 kCos27 = 0.8910065f;
    mSpeed.x = mBaseSpeed * kCos27;
    mSpeed.y = 0.5f * mSpeed.x;
    mPos.x += mSpeed.x;''',
    'branch': '''        mSpeed.x = mBaseSpeed * kCos27;
        mSpeed.y = 0.5f * mSpeed.x;''',
    'hdr': None}
VARIANTS['loc_both'] = {
    'prelude': '',
    'head': '''    const f32 kCos27 = 0.8910065f;
    const f32 kHalf = 0.5f;
    mSpeed.x = mBaseSpeed * kCos27;
    mSpeed.y = kHalf * mSpeed.x;
    mPos.x += mSpeed.x;''',
    'branch': '''        mSpeed.x = mBaseSpeed * kCos27;
        mSpeed.y = kHalf * mSpeed.x;''',
    'hdr': None}
# 5b: local declared BEFORE `mVec2_c old = mPos;` is not expressible with this
# anchor, so instead: local initialised from the member, i.e. the const local
# holds the *product operand order* the other way round.

# --- 6. element of a file-scope static const array ---------------------------
VARIANTS['arr_cos'] = V('static const f32 kSlope[] = { 0.8910065f, 0.5f };\n\n',
                        'kSlope[0]', '0.5f')
VARIANTS['arr_both'] = V('static const f32 kSlope[] = { 0.8910065f, 0.5f };\n\n',
                         'kSlope[0]', 'kSlope[1]')
VARIANTS['arr_cos_tbl'] = V(
    'static const f32 kSinCos[4] = { 0.8910065f, 0.4539905f, 0.5f, 0.8660254f };\n\n',
    'kSinCos[0]', 'kSinCos[2]')

# --- 7. const reference binding ---------------------------------------------
VARIANTS['ref_cos'] = V('static const f32 kCos27_v = 0.8910065f;\n'
                        'static const f32 &kCos27 = kCos27_v;\n\n',
                        'kCos27', '0.5f')
VARIANTS['ref_both'] = V('static const f32 kCos27_v = 0.8910065f;\n'
                         'static const f32 kHalf_v = 0.5f;\n'
                         'static const f32 &kCos27 = kCos27_v;\n'
                         'static const f32 &kHalf = kHalf_v;\n\n',
                         'kCos27', 'kHalf')

# --- extras: volatile-ish / extern non-const, to see if a NON-foldable named
#     constant (one the compiler must actually load) changes anything.
VARIANTS['fs_nonconst'] = V('static f32 kCos27 = 0.8910065f;\n\n', 'kCos27', '0.5f')
VARIANTS['extern_cos'] = V('extern const f32 kCos27;\n\n', 'kCos27', '0.5f')

# --- batch 2: narrowing WHICH property of a named constant does the work -----
# 6b. single-element static const array (defined in TU, still not folded?)
VARIANTS['arr1_cos'] = V('static const f32 kCos27a[1] = { 0.8910065f };\n\n',
                         'kCos27a[0]', '0.5f')
# 6c. member of a file-scope static const struct
VARIANTS['struct_cos'] = V('struct SlopeK { f32 c, h; };\n'
                           'static const SlopeK kK = { 0.8910065f, 0.5f };\n\n',
                           'kK.c', '0.5f')
VARIANTS['struct_both'] = V('struct SlopeK { f32 c, h; };\n'
                            'static const SlopeK kK = { 0.8910065f, 0.5f };\n\n',
                            'kK.c', 'kK.h')
# 3d. address-taken plain static const scalar -- does defeating the fold alone
#     reproduce it, without an aggregate?
VARIANTS['addr_cos'] = V('static const f32 kCos27 = 0.8910065f;\n'
                         'const f32 *const kCos27p = &kCos27;\n\n',
                         'kCos27', '0.5f')
VARIANTS['deref_cos'] = V('static const f32 kCos27 = 0.8910065f;\n'
                          'static const f32 *const kCos27p = &kCos27;\n\n',
                          '*kCos27p', '0.5f')
# 7b. const reference bound directly to the literal -- the only named shape that
#     could plausibly still emit an ANONYMOUS pool symbol.
VARIANTS['ref_lit'] = V('static const f32 &kCos27 = 0.8910065f;\n\n',
                        'kCos27', '0.5f')
# 3e. class-static declared-not-defined, cos only, but spelled fully qualified
VARIANTS['cls_extern_q'] = V('', 'dLineMng_c::smc_COS27', '0.5f',
                             hdr='    static const float smc_COS27;\n')


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


def ensure_inc():
    if not os.path.isdir(MY_INC):
        shutil.copytree(ORIG_INC, MY_INC)


def run(name, verbose=True):
    v = VARIANTS[name]
    inc = ORIG_INC
    if v['hdr']:
        ensure_inc()
        hp = os.path.join(MY_INC, 'game', 'bases', 'd_line_mng.hpp')
        orig = open(os.path.join(ORIG_INC, 'game', 'bases', 'd_line_mng.hpp'),
                    encoding='utf-8').read()
        assert orig.count(CLASS_ANCHOR) == 1
        open(hp, 'w', encoding='utf-8').write(
            orig.replace(CLASS_ANCHOR, CLASS_ANCHOR + '\n' + v['hdr'].rstrip('\n')))
        inc = MY_INC

    src = open(BASE, encoding='utf-8').read()
    start = src.index('void dLineMng_c::executeState_Left30Left()')
    end = src.index('void dLineMng_c::initializeState_Left30Right()')
    fnbody = src[start:end]
    if fnbody.count(HEAD) != 1 or fnbody.count(BRANCH) != 1:
        sys.exit('anchor not unique inside the function')
    fnbody = fnbody.replace(HEAD, v['head']).replace(BRANCH, v['branch'])

    pre_at = src.index(ANCHOR)
    assert pre_at < start
    out = src[:pre_at] + v['prelude'] + src[pre_at:start] + fnbody + src[end:]

    out_src = os.path.join(HERE, 'a_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(out)

    obj = os.path.join(HERE, 'a_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[inc])
    if not ok:
        print('%-18s COMPILE FAILED' % name)
        if verbose:
            print(log[-1500:])
        return None
    txt = os.path.join(HERE, 'a_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print('%-18s DISASM FAILED' % name)
        return None

    draft, target = parse(txt).get(FN), parse(TARGET).get(FN)
    eq_bytes = [b for b, _ in draft] == [b for b, _ in target]
    eq_canon = (harness.canonicalise([t for _, t in draft])
                == harness.canonicalise([t for _, t in target]))
    # the four load lines under test, compared SEMANTICALLY: (dest reg, kind)
    # where kind is MEM (struct member load) vs K (constant from .sdata2), so
    # the pool-symbol renaming noise cannot mask or fake a hit.
    def shape(s):
        m = re.match(r'lfs (f\d+), (.*)', s)
        if not m:
            return s
        return '%s<-%s' % (m.group(1), 'K' if '@sda21' in m.group(2) else
                           ('MEM' + m.group(2).split('(')[0] if '(' in m.group(2) else '?'))
    nfixed, detail = 0, []
    for i in (2, 10, 33, 34):
        t = shape(target[i][1]) if i < len(target) else '--'
        d = shape(draft[i][1]) if i < len(draft) else '--'
        nfixed += (t == d)
        detail.append('%d:%s' % (i, 'ok' if t == d else '%s/%s' % (t, d)))
    status = 'MATCH' if (eq_bytes or eq_canon) else 'differs'
    print('%-18s len %3d (%+d)  %-8s  2/10/33/34 %d/4  %s' %
          (name, len(draft), len(draft) - len(target), status, nfixed,
           ' '.join(detail)))
    if verbose:
        n = max(len(draft), len(target))
        shown = 0
        for i in range(n):
            t = target[i][1] if i < len(target) else '--'
            d = draft[i][1] if i < len(draft) else '--'
            if t != d:
                print('    %-4d %-38s %s' % (i, t, d))
                shown += 1
                if shown > 30:
                    print('    ... truncated')
                    break
    return nfixed


def main():
    a = sys.argv[1] if len(sys.argv) > 1 else '--all'
    if a == '--list':
        print('\n'.join(VARIANTS))
        return
    if a == '--all':
        for k in VARIANTS:
            run(k, verbose=False)
        return
    run(a)


main()
