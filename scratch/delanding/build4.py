import sys, os, hashlib
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
from harness import compile_draft

UNITS = [
    ('d_a_wm_manta', {'.text': 0x55c, '.rodata': 0x18, '.data': 0x98}),
    ('d_a_floor_jr_b', {'.text': 0xb0, '.data': 0x298}),
    ('d_a_peach_castle_sequence', {'.text': 0xa20, '.ctors': 0x4, '.data': 0x2d8, '.bss': 0x48}),
    ('d_a_wm_sandpillar', {'.text': 0x1cf0, '.ctors': 0x4, '.rodata': 0xa0, '.data': 0x5a0, '.bss': 0x250}),
]

for name, claim in UNITS:
    src = 'source/d_basesNP/bases/%s.cpp' % name
    obj = 'scratch/delanding/%s.o' % name
    ok, log = compile_draft(src, obj, module='d_basesNP')
    print('=' * 60)
    print('%s: compile %s' % (name, 'OK' if ok else 'FAILED'))
    if not ok:
        print(log[:2000])
        continue
    print('  md5 %s  size %d' % (hashlib.md5(open(obj, 'rb').read()).hexdigest(),
                                 os.path.getsize(obj)))
    print('  claim: ' + '  '.join('%s=%#x' % kv for kv in claim.items()))
