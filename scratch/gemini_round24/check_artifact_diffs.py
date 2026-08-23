import sys
sys.path.append('.')
from scratch.gemini_round24 import tool

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
draft_all = tool.parse_disasm(tool.DRAFT_DIS)

for name in ['__sinit', 'executeState_ShellAtk_St']:
    for k in target_all:
        if name in k:
            t_fn = target_all[k]
            d_fn = draft_all[k]
            raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
            d_can = tool.harness.canonicalise([t for _, t in d_fn])
            t_can = tool.harness.canonicalise([t for _, t in t_fn])
            can_diffs = sum(1 for tt, dt in zip(t_can, d_can) if tt != dt) + abs(len(t_can) - len(d_can))
            print(f"{k}: raw diffs = {raw_diffs}, canonical diffs = {can_diffs}")
