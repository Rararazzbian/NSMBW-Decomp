import sys, subprocess
cmd = [sys.executable, 'tools/auto_decomp/fndiff.py', 'scratch/gemini_round24/target_all_text.txt', 'scratch/gemini_round24/test_quake_temp.txt', 'setQuakeDead__18dEnTorideKokoopa_cFv', '-v']
subprocess.run(cmd)
