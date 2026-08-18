import sys, os, shutil
sys.path.append('tools/auto_decomp')
import harness
import subprocess

mock_base = 'scratch/mock_include/game/bases'
os.makedirs(mock_base, exist_ok=True)
for h in ['d_pause_manager.hpp', 'd_s_stage.hpp', 'd_game_display.hpp', 'd_stage_timer.hpp', 'd_quake.hpp']:
    shutil.copyfile(f'scratch/{h}', f'{mock_base}/{h}')

cmd = [harness.MWCC] + harness.CFLAGS + ['-i', os.path.abspath('scratch/mock_include')]
for inc in harness.INCLUDES:
    cmd.extend(['-i', os.path.join(harness.ROOT, inc)])
cmd.extend(['-o', os.path.abspath('scratch/test_update_draft2.o'), os.path.abspath('scratch/test_update_draft2.cpp')])

res = subprocess.run(cmd, capture_output=True, text=True)
print('Return code:', res.returncode)
print('Stdout:', res.stdout)
print('Stderr:', res.stderr)
