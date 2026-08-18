import os, sys, subprocess

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')

obj_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'm_pad_scaffold.o')
txt_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'm_pad_scaffold.txt')

res = subprocess.run([DTK, 'elf', 'disasm', obj_path, txt_path], capture_output=True, text=True)
print("Disasm code:", res.returncode)

with open(txt_path) as f:
    text = f.read()

# Let's inspect sections in txt
print("=== SCAFFOLD DISASSEMBLY SUMMARY ===")
for line in text.splitlines():
    if line.startswith('#') or line.startswith('.fn') or line.startswith('.obj') or line.startswith('.section') or line.startswith('.text') or line.startswith('.data') or line.startswith('.bss') or line.startswith('.sbss') or line.startswith('.ctors'):
        print(line)
