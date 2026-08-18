import re
import os
import subprocess

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")

# 1. Parse all functions in orig_mpad_text.txt
fns = []
with open(os.path.join(ROOT, "scratch", "gemini_round4", "orig_mpad_text.txt")) as f:
    cur_fn = None
    cur_size = 0
    cur_addr = 0
    for line in f:
        line = line.strip()
        if line.startswith("# .text:"):
            # # .text:0x0 | 0x8016F330 | size: 0x30
            m = re.search(r'0x([0-9A-Fa-f]+)\s*\|\s*size:\s*(0x[0-9A-Fa-f]+|\d+)', line)
            if m:
                cur_addr = int(m.group(1), 16)
                s_str = m.group(2)
                cur_size = int(s_str, 16) if s_str.startswith("0x") else int(s_str)
        elif line.startswith(".fn "):
            parts = line.split()
            fn_name = parts[1].rstrip(",")
            fns.append((cur_addr, cur_size, fn_name))

print(f"Total functions in m_pad.cpp: {len(fns)}")
for addr, size, name in fns:
    print(f"{hex(addr)} {hex(size):6} {name}")
