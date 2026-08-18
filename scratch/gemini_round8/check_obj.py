import subprocess, os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")
obj = os.path.join(ROOT, "scratch", "gemini_round8", "d_multi_manager.o")

# Let's run dtk elf dump or inspect sections
# We can use python to inspect sections if pyelftools or similar, or inspect objdump
p = subprocess.run([DTK, 'elf', 'disasm', obj, 'scratch/gemini_round8/full_disasm_draft.txt'], cwd=ROOT, capture_output=True, text=True)
print(p.stdout, p.stderr)
