"""Angle A6: is the mBaseSpeed/0.8910065f register permutation sensitive to
TRANSLATION-UNIT ordering (float-literal pool order, function order)?

Each variant is a whole-TU text transform applied to a COPY of gapA_all.cpp.
Judged ONLY on indices 2/10/33/34 of executeState_Left30Left:

    idx  TARGET                                CONTROL DRAFT
    2    lfs f0, <0.8910065>                   lfs f1, <0.8910065>
    10   lfs f1, 0x60(r3)                      lfs f0, 0x60(r3)
    33   lfs f1, 0x60(r30)                     lfs f1, <0.8910065>
    34   lfs f0, <0.8910065>                   lfs f0, 0x60(r30)

Usage:  python tu_order.py <variant>|--list|--all
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
KEY = [2, 10, 33, 34]

# ---------------------------------------------------------------- TU surgery

# Anchor: first line of the first real function definition in the TU.
TOP_ANCHOR = 'dLineMng_c::dLineMng_c() :'


def insert_at_top(src, text):
    i = src.index(TOP_ANCHOR)
    return src[:i] + text + '\n\n' + src[i:]


def fn_span(src, sig):
    """Byte span of a top-level function definition beginning with `sig`,
    ending at the closing brace in column 0 (or the end of a one-liner)."""
    i = src.index(sig)
    # walk to first '{'
    j = src.index('{', i)
    depth = 0
    k = j
    while True:
        c = src[k]
        if c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                break
        k += 1
    return i, k + 1


STATE_TRIPLES = ['Left45', 'Right45', 'Side', 'Height', 'CornerHeightLine',
                 'CornerSideLine', 'Left30Left', 'Left30Right', 'Right30Left',
                 'Right30Right', 'Left60Up', 'Left60Down', 'Right60Down',
                 'Right60Up']


def triple_span(src, name):
    """Span covering initializeState_X / finalizeState_X / executeState_X."""
    a, _ = fn_span(src, 'void dLineMng_c::initializeState_%s()' % name)
    _, b = fn_span(src, 'void dLineMng_c::executeState_%s()' % name)
    # swallow trailing newlines
    while b < len(src) and src[b] in '\r\n':
        b += 1
    return a, b


def move_triple(src, name, before=None, to_end=False):
    a, b = triple_span(src, name)
    block = src[a:b]
    rest = src[:a] + src[b:]
    if to_end:
        return rest.rstrip() + '\n\n' + block
    i, _ = triple_span(rest, before)
    return rest[:i] + block + rest[i:]


def reverse_thirty_sixty(src):
    """Reverse the order of the six 30/60-degree triples among themselves."""
    names = ['Left30Left', 'Left30Right', 'Right30Left', 'Right30Right',
             'Left60Up', 'Left60Down', 'Right60Down', 'Right60Up']
    spans = [triple_span(src, n) for n in names]
    # they are contiguous in the file; verify
    spans.sort()
    lo, hi = spans[0][0], spans[-1][1]
    blocks = [src[a:b] for a, b in spans]
    return src[:lo] + ''.join(reversed(blocks)) + src[hi:]


DUMMY_891 = '''static f32 s_a6_sink891;
static void a6_touch891(f32 v) { s_a6_sink891 = v * 0.8910065f; }
'''

DUMMY_HALF = '''static f32 s_a6_sinkhalf;
static void a6_touchhalf(f32 v) { s_a6_sinkhalf = v * 0.5f; }
'''

DUMMY_BOTH = '''static f32 s_a6_sinkb;
static void a6_touchb(f32 v) { s_a6_sinkb = (v * 0.8910065f) * 0.5f; }
'''

DUMMY_BOTH_REV = '''static f32 s_a6_sinkbr;
static void a6_touchbr(f32 v) { s_a6_sinkbr = (v * 0.5f) * 0.8910065f; }
'''

DUMMY_PLAIN = '''static int s_a6_plain;
static void a6_plain(int v) { s_a6_plain = v + 3; }
'''

# A dummy that uses the SAME shape (member * const) on a dLineMng_c, so that
# the const and the member load appear together earlier in the TU.
# mBaseSpeed is private, so this has to live inside an existing member fn.
SETBASE_SIG = 'void dLineMng_c::SetBaseSpeed(f32 speed)'


def V_control(src):
    return src


def V_pre891(src):
    return insert_at_top(src, DUMMY_891)


def V_prehalf(src):
    return insert_at_top(src, DUMMY_HALF)


def V_preboth(src):
    return insert_at_top(src, DUMMY_891 + '\n' + DUMMY_HALF)


def V_prehalf_then891(src):
    return insert_at_top(src, DUMMY_HALF + '\n' + DUMMY_891)


def V_preprod(src):
    return insert_at_top(src, DUMMY_BOTH)


def V_preprod_rev(src):
    return insert_at_top(src, DUMMY_BOTH_REV)


def _inject_into_setbase(src, stmts):
    """Append statements to the early member fn SetBaseSpeed (line ~121),
    which is well before every executeState_*."""
    a, b = fn_span(src, SETBASE_SIG)
    body = src[a:b]
    assert body.rstrip().endswith('}')
    cut = body.rstrip()[:-1]
    return src[:a] + cut + stmts + '}\n' + src[b:]


def V_preshape(src):
    """Earlier use of the EXACT statement shape: member * 0.8910065f."""
    return _inject_into_setbase(src, '    mSpeed.x = mBaseSpeed * 0.8910065f;\n')


def V_prepair(src):
    """Earlier use of the exact TWO-statement pair from the residual."""
    return _inject_into_setbase(
        src,
        '    mSpeed.x = mBaseSpeed * 0.8910065f;\n'
        '    mSpeed.y = 0.5f * mSpeed.x;\n')


def V_preplain(src):
    return insert_at_top(src, DUMMY_PLAIN)


def V_l30l_first(src):
    """Move the Left30Left triple to be the first of the state triples."""
    return move_triple(src, 'Left30Left', before='Left45')


def V_l30l_last(src):
    return move_triple(src, 'Left30Left', to_end=True)


def V_l30l_after_l60(src):
    """Left30Left moved to just before Right60Up (late, but still amid states)."""
    return move_triple(src, 'Left30Left', before='Right60Up')


def V_reverse_states(src):
    return reverse_thirty_sixty(src)


def V_drop_falldown(src):
    """Remove an unrelated earlier function body (executeState_FallDown's
    contents) -- changes which literals enter the pool before Left30Left."""
    a, b = fn_span(src, 'void dLineMng_c::executeState_FallDown()')
    return src[:a] + 'void dLineMng_c::executeState_FallDown() {}' + src[b:]


def V_add_early_fn(src):
    """Add an unrelated non-float function early in the TU."""
    return insert_at_top(src, '''static int s_a6_acc;
static void a6_extra(int a, int b) { s_a6_acc = a * b + s_a6_acc; }
''')


def V_retailpool(src):
    """Force 0.5f into the pool VERY early -- before 0.8910065f and before the
    0.5 double -- reproducing retail's relative pool ordering
    16.0f < 0.5f < 0.5(double) < 0.8910065f."""
    return _inject_into_setbase(src, '    mSpeed.y = mBaseSpeed * 0.5f;\n')


def V_retailpool_mid(src):
    """Same, but injected into a later function (move_on_circle_speedset) so
    16.0f is already in the pool first, matching retail exactly."""
    a, b = fn_span(src, 'void dLineMng_c::move_on_circle_speedset(f32 radius, f32 speedScale)')
    body = src[a:b]
    cut = body.rstrip()[:-1]
    return src[:a] + cut + '    mSpeed.y = mBaseSpeed * 0.5f;\n}\n' + src[b:]


VARIANTS = {
    'control': V_control,
    'pre891': V_pre891,
    'prehalf': V_prehalf,
    'preboth': V_preboth,
    'prehalf_then891': V_prehalf_then891,
    'preprod': V_preprod,
    'preprod_rev': V_preprod_rev,
    'preshape': V_preshape,
    'prepair': V_prepair,
    'retailpool': V_retailpool,
    'retailpool_mid': V_retailpool_mid,
    'preplain': V_preplain,
    'l30l_first': V_l30l_first,
    'l30l_last': V_l30l_last,
    'l30l_after_l60': V_l30l_after_l60,
    'reverse_states': V_reverse_states,
    'drop_falldown': V_drop_falldown,
    'add_early_fn': V_add_early_fn,
}


# ------------------------------------------------------------------ analysis

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
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)',
                          line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


def run(name):
    src = open(BASE, encoding='utf-8').read()
    out_src = os.path.join(HERE, 'v_%s.cpp' % name)
    open(out_src, 'w', encoding='utf-8').write(VARIANTS[name](src))
    obj = os.path.join(HERE, 'v_%s.o' % name)
    ok, log = harness.compile_draft(out_src, obj, extra_inc=[INC])
    if not ok:
        print('%-18s COMPILE FAILED' % name)
        print(log[-1500:])
        return
    txt = os.path.join(HERE, 'v_%s.txt' % name)
    ok, log = harness.disasm(obj, txt)
    if not ok:
        print('%-18s DISASM FAILED' % name)
        return
    draft = parse(txt).get(FN)
    target = parse(TARGET).get(FN)
    if draft is None:
        print('%-18s function not found' % name)
        return
    can_eq = (harness.canonicalise([t for _, t in draft]) ==
              harness.canonicalise([t for _, t in target]))
    print('=== %s   len=%d (target %d, %+d)%s' %
          (name, len(draft), len(target), len(draft) - len(target),
           '  CANONICALLY EQUAL' if can_eq else ''))
    for i in KEY:
        t = target[i][1] if i < len(target) else '--'
        d = draft[i][1] if i < len(draft) else '--'
        flag = 'SAME' if t == d else ('shape-match' if _shape(t) == _shape(d)
                                      else 'DIFF')
        print('  %-4d %-40s %-40s %s' % (i, t, d, flag))
    ndiff = sum(1 for i in range(max(len(draft), len(target)))
                if (target[i][1] if i < len(target) else '') !=
                   (draft[i][1] if i < len(draft) else ''))
    print('  raw-text differing lines: %d' % ndiff)


LIT = re.compile(r'"?@[0-9_A-Za-z]+"?@sda21\(r0\)')


def _shape(s):
    """Collapse literal symbol names so f0/f1 + operand kind is what compares."""
    return LIT.sub('<LIT>', s)


def main():
    if len(sys.argv) < 2 or sys.argv[1] == '--list':
        print('\n'.join(VARIANTS))
        return
    if sys.argv[1] == '--all':
        for k in VARIANTS:
            run(k)
        return
    run(sys.argv[1])


main()
