import sys, re, subprocess
sys.path.insert(0, 'tools/auto_decomp')
sys.path.insert(0, 'wip/wm_units')
import harness as H
from verify_anon import functions, norm

SRC = 'wip/wm_units/agent_wall/d_a_wm_kinoko_base.cpp'
OBJ = 'wip/wm_units/agent_wall/draft.o'
TXT = 'wip/wm_units/agent_wall/draft.txt'

ok, log = H.compile_draft(SRC, OBJ, extra_inc=['wip/wm_units/agent_wall/shadow_include'], module='d_basesNP')
if not ok:
    print('COMPILE FAIL')
    print(log[-3000:])
    sys.exit(1)
H.disasm(OBJ, TXT)

target_funcs = functions('wip/wm_units/_dis/auto_00_0016B2A4_text.o.txt', with_addr=True)
target = None
for addr, name, ins in target_funcs:
    if addr == 0x16b620:
        target = ins
draft_all = functions(TXT)
draft = None
for name, ins in draft_all:
    if 'createModel' in name:
        draft = ins
        break

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
print('diffs=%d first_diff=%s target_len=%d draft_len=%d' % (diffs, first_diff, len(a), len(b)))
if diffs and diffs < 60:
    for i in range(max(len(a), len(b))):
        x = a[i] if i < len(a) else '<none>'
        y = b[i] if i < len(b) else '<none>'
        mark = '  ' if x == y else '<<'
        print('%3d %s want:%-42s got:%-42s' % (i, mark, x, y))
