import sys
from pathlib import Path
sys.path.insert(0, 'tools/auto_decomp')
import harness
root = Path('scratch/codex_round10/checkCorrectCreateInfo')
label = sys.argv[1]
src = root / (label + '.cpp')
obj = root / (label + '.o')
txt = root / (label + '.txt')
ok, log = harness.compile_draft(str(src), str(obj))
print('compile', ok, log)
if not ok:
    raise SystemExit(1)
ok, log = harness.disasm(str(obj), str(txt))
print('disasm', ok, log)
print(harness.diff_fn('wip/player_manager/target_text.txt', str(txt), 'checkCorrectCreateInfo__9daPyMng_cFv'))
