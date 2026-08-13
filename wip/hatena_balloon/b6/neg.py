"""Negative controls: each mutation must be caught by the named check.

A check that never fails is not a check. Of particular interest is N1: it changes
a literal VALUE only, and because dtk zeroes relocations the raw 4-byte words are
IDENTICAL -- so the word comparator passes it. Only E can see it.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import verify as V  # noqa: E402
import sweep  # noqa: E402

BASE = sweep.base_text()

# (name, mutation, check that MUST fail, checks that must still PASS)
CONTROLS = [
    ('N1 wrong float literal (16.0 -> 17.0 in fly_dispin_check)',
     lambda s: s.replace("""        if (mPos.x <= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                          + dBgParameter_c::ms_Instance_p->xSize() - 16.0f) {""",
                         """        if (mPos.x <= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                          + dBgParameter_c::ms_Instance_p->xSize() - 17.0f) {""", 1),
     'E const values', ['B raw words', 'C callee names', 'D symbol order']),

    ('N2 wrong .rodata table entry (-32.0 -> -33.0)',
     lambda s: s.replace('{ 0.0f, -32.0f, 32.0f, -64.0f }',
                         '{ 0.0f, -33.0f, 32.0f, -64.0f }', 1),
     'E2 l_create_diff bytes', ['B raw words', 'C callee names', 'D symbol order']),

    ('N3 wrong callee (identical words, different symbol)',
     lambda s: s.replace('static float bg_dispx_get(daEnHatenaBalloon_c *balloon) {',
                         'static float bg_dispx_get2(daEnHatenaBalloon_c *balloon);\n'
                         'static float bg_dispx_get(daEnHatenaBalloon_c *balloon) {', 1)
                .replace('-(bg_dispx_get(this)', '-(bg_dispx_get2(this)', 1)
                .replace('// ---------------------------------------------------------------- 0x80112950',
                         'static float bg_dispx_get2(daEnHatenaBalloon_c *balloon) {\n'
                         '    return bg_dispx_get(balloon);\n}\n\n'
                         '// ------------- 0x80112950', 1),
     'C callee names', []),

    ('N4 emitted symbol order (swap two functions in the file)',
     None, 'D symbol order', ['B raw words', 'C callee names', 'E const values']),

    ('N5 wrong instruction (xSize -> ySize in escape_dispout_check)',
     lambda s: s.replace("""        if (mPos.x <= 48.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                                   + dBgParameter_c::ms_Instance_p->xSize())) {""",
                         """        if (mPos.x <= 48.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                                   + dBgParameter_c::ms_Instance_p->ySize())) {""", 1),
     'B raw words', ['D symbol order']),
]


def swap_two_functions(s):
    """Move fly_dispin_check after escape_dispout_check."""
    a = s.index('// ---------------------------------------------------------------- 0x80112EF0')
    b = s.index('// ---------------------------------------------------------------- 0x80112FC0')
    c = s.index('/// @brief X/Y nudge')
    return s[:a] + s[b:c] + s[a:b] + s[c:]


CONTROLS[3] = (CONTROLS[3][0], swap_two_functions) + CONTROLS[3][2:]

# Baseline failures (fly_ydisp_check's known 2-word swap) are subtracted, so a
# control is judged on what IT changed, not on a pre-existing gap.
_bp = os.path.join(HERE, 'negbase.cpp')
open(_bp, 'w', newline='\n').write(BASE)
BASELINE = {k: set(map(str, v)) for k, v in V.run(_bp, tag='negbase', verbose=False).items()}
print('baseline failures:', {k: len(v) for k, v in BASELINE.items() if v}, '\n')

nfail = 0
for i, (name, mut, must_fail, must_pass) in enumerate(CONTROLS):
    src = mut(BASE)
    if src == BASE:
        print('%-58s !! MUTATION DID NOT APPLY' % name)
        nfail += 1
        continue
    p = os.path.join(HERE, 'neg%d.cpp' % i)
    open(p, 'w', newline='\n').write(src)
    res = V.run(p, tag='neg%d' % i, verbose=False)
    if 'BUILD' in res:
        print('%-58s !! BUILD FAILED' % name)
        print(res['BUILD'][0][-800:])
        nfail += 1
        continue
    new = {k: set(map(str, v)) - BASELINE.get(k, set()) for k, v in res.items()}
    fired = bool(new.get(must_fail))
    leaked = [c for c in must_pass if new.get(c)]
    ok = fired and not leaked
    nfail += not ok
    print('%-58s %s' % (name, 'CONTROL OK' if ok else 'CONTROL BROKEN'))
    print('     %s -> %s' % (must_fail, 'FAILED (good)' if fired else 'PASSED (BAD)'))
    if leaked:
        print('     collateral: these were supposed to stay clean but did not: %s' % leaked)
    other = [c for c in new if new[c] and c != must_fail]
    if other:
        print('     also fired (expected for this mutation): %s' % other)

print('\n%d/%d controls behaved as designed' % (len(CONTROLS) - nfail, len(CONTROLS)))
