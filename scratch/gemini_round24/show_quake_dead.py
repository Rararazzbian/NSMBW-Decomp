import sys
sys.path.append('.')
from scratch.gemini_round24 import tool

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
t_fn = target_all.get('setQuakeDead__18dEnTorideKokoopa_cFv')

print(f"Target insns ({len(t_fn)}):")
for i, (b, t) in enumerate(t_fn):
    print(f"{i:2d}: [{b}] {t}")
