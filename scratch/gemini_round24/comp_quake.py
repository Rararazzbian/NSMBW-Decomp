import sys
sys.path.append('.')
from scratch.gemini_round24 import tool

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
draft_all = tool.parse_disasm(tool.DRAFT_DIS)

t_fn = target_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')
d_fn = draft_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')

print(f"Target: {len(t_fn)} insns, Draft: {len(d_fn)} insns")
for i in range(max(len(t_fn), len(d_fn))):
    tb = t_fn[i][0] if i < len(t_fn) else '        '
    tt = t_fn[i][1] if i < len(t_fn) else ''
    db = d_fn[i][0] if i < len(d_fn) else '        '
    dt = d_fn[i][1] if i < len(d_fn) else ''
    eq = '==' if tb == db else '!='
    print(f"{i:2d} {eq} T: [{tb}] {tt:45s} | D: [{db}] {dt}")
