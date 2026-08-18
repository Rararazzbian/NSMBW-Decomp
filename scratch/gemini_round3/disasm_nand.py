import os
import subprocess

# Disassemble auto_03_800CED00_text.o using dtk
dtk_path = os.path.abspath("bin/dtk-windows-x86_64.exe")
obj_path = os.path.abspath("bin/dtkspl/obj/auto_03_800CED00_text.o")
out_path = os.path.abspath("scratch/gemini_round3/d_nand_thread_disasm.txt")

cmd = [dtk_path, "elf", "disasm", obj_path, out_path]
res = subprocess.run(cmd, capture_output=True, text=True)
print("dtk return code:", res.returncode)
if res.stderr:
    print("dtk stderr:", res.stderr)

with open(out_path, 'r') as f:
    disasm = f.read()

print(f"Disassembly written, length: {len(disasm)} chars, lines: {len(disasm.splitlines())}")
