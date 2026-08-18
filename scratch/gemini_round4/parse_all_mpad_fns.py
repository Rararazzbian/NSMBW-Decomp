import re
import os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"

# Let's inspect all function disassembly in orig_mpad_text.txt and orig_mpad_text2.txt
text_files = [
    os.path.join(ROOT, "scratch", "gemini_round4", "orig_mpad_text.txt"),
    os.path.join(ROOT, "scratch", "gemini_round4", "orig_mpad_text2.txt")
]

all_fns = []
for tf in text_files:
    with open(tf) as f:
        cur_fn = None
        cur_lines = []
        cur_addr = None
        cur_size = 0
        for line in f:
            line_s = line.strip()
            if line_s.startswith("# .text:"):
                m = re.search(r'0x([0-9A-Fa-f]+)\s*\|\s*size:\s*(0x[0-9A-Fa-f]+|\d+)', line_s)
                if m:
                    cur_addr = int(m.group(1), 16)
                    s_str = m.group(2)
                    cur_size = int(s_str, 16) if s_str.startswith("0x") else int(s_str)
            elif line_s.startswith(".fn "):
                parts = line_s.split()
                cur_fn = parts[1].rstrip(",")
                cur_lines = []
            elif line_s.startswith(".endfn"):
                if cur_fn and not cur_fn.startswith("gap_") and not cur_fn.startswith("pad_"):
                    all_fns.append((cur_addr, cur_size, cur_fn, cur_lines))
                cur_fn = None
            elif cur_fn:
                cur_lines.append(line_s)

# Also check for __sinit_
all_fns.append((0x8016F7B0, 0x58, "__sinit_\\m_pad_cpp", []))
all_fns.sort(key=lambda x: x[0])

print(f"Total real functions in m_pad.cpp: {len(all_fns)}")
print("Address    Size   Name")
for addr, size, name, lines in all_fns:
    print(f"{hex(addr)} {hex(size):6} {name}")
