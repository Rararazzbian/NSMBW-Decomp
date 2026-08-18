import subprocess, os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")
obj = os.path.join(ROOT, "scratch", "gemini_round8", "d_multi_manager_clean2.o")

p = subprocess.run([DTK, 'elf', 'disasm', obj, 'scratch/gemini_round8/clean2_disasm.txt'], cwd=ROOT, capture_output=True, text=True)
print(p.stdout, p.stderr)
