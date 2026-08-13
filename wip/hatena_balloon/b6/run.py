"""Batch-6 driver: compile, disasm, diff all 8 functions against the target.

NO shim include dir -- the repo header is authoritative now.
"""
import os
import sys
import difflib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402
import b5cmp  # noqa: E402

TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_a_en_hatena_balloon', 'target.txt')

STATIC_NAME = 'bg_dispx_get__FP19daEnHatenaBalloon_c'

PAIRS = [
    ('fn_80112040', STATIC_NAME),
    ('fly_yspeed_set__19daEnHatenaBalloon_cFv', 'fly_yspeed_set__19daEnHatenaBalloon_cFv'),
    ('fly_xspeed_set__19daEnHatenaBalloon_cFb', 'fly_xspeed_set__19daEnHatenaBalloon_cFb'),
    ('fly_ydisp_check__19daEnHatenaBalloon_cFb', 'fly_ydisp_check__19daEnHatenaBalloon_cFb'),
    ('fly_xdisp_check__19daEnHatenaBalloon_cFb', 'fly_xdisp_check__19daEnHatenaBalloon_cFb'),
    ('fly_dispin_check__19daEnHatenaBalloon_cFv', 'fly_dispin_check__19daEnHatenaBalloon_cFv'),
    ('escape_dispout_check__19daEnHatenaBalloon_cFv', 'escape_dispout_check__19daEnHatenaBalloon_cFv'),
    ('create_wait_pos_set__19daEnHatenaBalloon_cFv', 'create_wait_pos_set__19daEnHatenaBalloon_cFv'),
]

RENAME = {'fn_80112040': STATIC_NAME}

SIZES = {
    'fn_80112040': 0x88,
    'fly_yspeed_set__19daEnHatenaBalloon_cFv': 0x1AC,
    'fly_xspeed_set__19daEnHatenaBalloon_cFb': 0x164,
    'fly_ydisp_check__19daEnHatenaBalloon_cFb': 0xDC,
    'fly_xdisp_check__19daEnHatenaBalloon_cFb': 0x198,
    'fly_dispin_check__19daEnHatenaBalloon_cFv': 0xC8,
    'escape_dispout_check__19daEnHatenaBalloon_cFv': 0xCC,
    'create_wait_pos_set__19daEnHatenaBalloon_cFv': 0x190,
}


def check_sizes():
    bad = []
    for name, size in SIZES.items():
        body = b5cmp.extract_exact(TARGET, name)
        if body is None:
            bad.append('%s: MISSING from target' % name)
        elif len(body) * 4 != size:
            bad.append('%s: %d insns x4 = 0x%X != map 0x%X'
                       % (name, len(body), len(body) * 4, size))
    return bad


def build(src, tag):
    obj = os.path.join(HERE, tag + '.o')
    txt = os.path.join(HERE, tag + '.txt')
    ok, log = H.compile_draft(src, obj)
    if not ok:
        return None, 'COMPILE FAILED\n' + log[-6000:]
    ok, log2 = H.disasm(obj, txt)
    if not ok:
        return None, 'DISASM FAILED\n' + log2[-2000:]
    return txt, log


def main(only=None, ctx=4, src=None, tag='v'):
    bad = check_sizes()
    if bad:
        print('SIZE ASSERTION FAILED')
        print('\n'.join(bad))
        return 1
    src = src or os.path.join(os.path.dirname(HERE), 'hb-b6.cpp')
    txt, log = build(src, tag)
    if txt is None:
        print(log)
        return 1
    if log.strip():
        print('--- compiler output ---')
        print(log[-2000:])
    emitted = [n for n in b5cmp.names(txt) if not n.startswith('gap_')]
    print('emitted order:', emitted)
    print()
    nmatch = ntried = 0
    total = 0
    for tgt, drf in PAIRS:
        if only and only not in tgt and only not in drf:
            continue
        ntried += 1
        a = b5cmp.extract_exact(TARGET, tgt, RENAME)
        b = b5cmp.extract_exact(txt, drf)
        if a is None:
            good, rep, n = False, 'TARGET MISSING %s' % tgt, 999
        elif b is None:
            good, rep, n = False, 'DRAFT MISSING %s' % drf, 999
        elif a == b:
            good, rep, n = True, 'MATCH (%d insns)' % len(a), 0
        else:
            good = False
            d = [l for l in difflib.unified_diff(a, b, 'target', 'draft',
                                                 lineterm='', n=ctx)]
            n = len([l for l in d if (l.startswith('+') or l.startswith('-'))
                     and not l.startswith('+++') and not l.startswith('---')])
            rep = 'MISMATCH target=%d draft=%d insns  DIFFLINES=%d\n' % (
                len(a), len(b), n)
            rep += '\n'.join(d)
        total += 0 if good else n
        nmatch += good
        print('=== %-46s %s' % (tgt, 'MATCH' if good else 'diff=%d' % n))
        if not good:
            print(rep)
        print()
    print('matched %d/%d   total difflines %d' % (nmatch, ntried, total))
    return 0


if __name__ == '__main__':
    a = sys.argv[1:]
    sys.exit(main(a[0] if a else None, int(a[1]) if len(a) > 1 else 4,
                  a[2] if len(a) > 2 else None,
                  a[3] if len(a) > 3 else 'v'))
