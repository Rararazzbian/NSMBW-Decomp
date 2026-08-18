import sys, os
sys.path.insert(0, 'tools/auto_decomp')
sys.path.insert(0, 'wip/wm_units')
import harness as H
from verify_anon import functions, norm

HERE = 'wip/wm_units/agent_tempwall/kc_test'
SRC = os.path.join(HERE, 'd_a_wm_koopa_castle.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')

ok, log = H.compile_draft(SRC, OBJ, extra_inc=[os.path.join(HERE, 'shadow_include')], module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-6000:])
    sys.exit(2)
H.disasm(OBJ, TXT)

CACHE = 'wip/wm_units/agent_tempwall/kc_test/_dis_cache'
os.makedirs(CACHE, exist_ok=True)
TARGET_OBJS = {
    0x1910A4: 'bin/dtkspl/d_basesNP/obj/auto_00_001910A4_text.o',
    0x191C30: 'bin/dtkspl/d_basesNP/obj/auto_fn_2_191C30_text.o',
    0x191D18: 'bin/dtkspl/d_basesNP/obj/auto_00_00191D18_text.o',
}

target_addr = int(sys.argv[1], 0)
draft_name_substr = sys.argv[2]

target = None
tname = None
for base, obj in TARGET_OBJS.items():
    out = os.path.join(CACHE, os.path.basename(obj) + '.txt')
    if not os.path.exists(out):
        H.disasm(obj, out)
    for addr, name, ins in functions(out, with_addr=True):
        if addr == target_addr:
            target = ins
            tname = name
            break
    if target is not None:
        break

if target is None:
    print('target not found at', hex(target_addr))
    sys.exit(1)

draft_all = functions(TXT)
draft = None
for name, ins in draft_all:
    if draft_name_substr in name:
        draft = ins
        dname = name
        break
if draft is None:
    print('draft not found containing', draft_name_substr)
    sys.exit(1)

a, b = norm(target), norm(draft)
diffs = 0
first_diff = None
for i in range(max(len(a), len(b))):
    x = a[i] if i < len(a) else '<none>'
    y = b[i] if i < len(b) else '<none>'
    if x != y:
        diffs += 1
        if first_diff is None:
            first_diff = i
print('target=%s draft=%s diffs=%d first_diff=%s target_len=%d draft_len=%d' % (tname, dname, diffs, first_diff, len(a), len(b)))
for i in range(max(len(a), len(b))):
    x = a[i] if i < len(a) else '<none>'
    y = b[i] if i < len(b) else '<none>'
    mark = '  ' if x == y else '<<'
    if mark == '<<':
        print('%3d %s want:%-42s got:%-42s' % (i, mark, x, y))
