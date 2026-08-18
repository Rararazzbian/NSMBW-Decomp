import json, subprocess, os, struct

BASE = r"C:/Users/Razz/Documents/Projects/NSMBW-Decomp"
VTABLE_ADDR = 0x80350AF8
VTABLE_SIZE = 0x9C

# Find which object file contains the vtable
obj_dir = os.path.join(BASE, "bin/dtkspl/obj")
found = None
for f in sorted(os.listdir(obj_dir)):
    if not f.startswith("auto_") or not f.endswith(".o"):
        continue
    parts = f[5:-2].split("_")
    if len(parts) != 2:
        continue
    lo = int(parts[0], 16)
    hi = int(parts[1], 16)
    if lo <= 0x350AF8 <= hi:
        print(f"Object covering 0x350AF8: {f} [0x{lo:08X} - 0x{hi:08X}]")
        found = (f, lo, hi)

if not found:
    print("No object found covering 0x350AF8")
    exit(1)

# Disassemble
fname, lo, hi = found
dtk = os.path.join(BASE, "bin/dtk-windows-x86_64.exe")
target = os.path.join(obj_dir, fname)
out = os.path.join(BASE, "scratch/codex_round7/vtable_disasm.txt")

result = subprocess.run([dtk, "elf", "disasm", target, out], capture_output=True, text=True)
print(f"dtk return: {result.returncode}")
print(result.stdout[:500])
print(result.stderr[:500])
