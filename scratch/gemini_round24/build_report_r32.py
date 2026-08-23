import os, sys, subprocess

ROOT = os.path.abspath('.')
sys.path.append('.')
from scratch.gemini_round24 import tool

# Re-run eval_all and poolcheck to ensure 100% fresh data
tool.compile_and_disasm()
matched, unmatched = tool.eval_all()

# Capture fndiff --all output
r_fndiff = subprocess.run([sys.executable, 'tools/auto_decomp/fndiff.py',
                           'scratch/gemini_round24/target_all_text.txt',
                           'scratch/gemini_round24/draft_disasm.txt', '--all'],
                          capture_output=True, text=True)
fndiff_all_text = r_fndiff.stdout.strip()

# Capture poolcheck output
r_pool = subprocess.run([sys.executable, 'tools/auto_decomp/poolcheck.py',
                         '--obj', 'scratch/gemini_round24/d_enemy_toride_kokoopa.o',
                         '--txt', 'scratch/gemini_round24/draft_disasm.txt',
                         'scratch/gemini_round24/auto_03_800A8710_text.txt',
                         'scratch/gemini_round24/auto_sinit_text.txt',
                         'scratch/gemini_round24/auto_03_800B03D8_text.txt'],
                        capture_output=True, text=True)
poolcheck_text = r_pool.stdout.strip()

print('fndiff len:', len(fndiff_all_text))
print('poolcheck len:', len(poolcheck_text))
