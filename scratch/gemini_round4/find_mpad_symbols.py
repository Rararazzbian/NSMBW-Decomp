import os
import subprocess
import re

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")

pat = re.compile(r'^(\S+)\s*=\s*(\.[^:]+):(0x[0-9A-Fa-f]+);\s*(.*)$')
size_pat = re.compile(r'size:(0x[0-9A-Fa-f]+|\d+)')

symbols = []
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')) as f:
    for line in f:
        m = pat.match(line.strip())
        if m:
            name, sec, addr_s, rest = m.groups()
            addr = int(addr_s, 16)
            sm = size_pat.search(rest)
            size_s = sm.group(1) if sm else '?'
            size = int(size_s, 16) if size_s.startswith('0x') else (int(size_s) if size_s != '?' else 0)
            symbols.append((sec, addr, size, name, rest))

# Let's inspect memory ranges from slices/wiimj2d.json
# m_mtx.cpp:
#   .text: 0x8016ECE0..0x8016F330
#   .ctors: 0x80006378..0x8000637C (offset 0x218..0x21c in .ctors)
#   .bss: 0x80370EB0..0x80370EE0 (offset 0x265d8..0x26608)
#   .sdata2: 0x8042FE20..0x8042FE30 (offset 0x2ca0..0x2cb0)
#
# m_pad.cpp:
#   .text: 0x8016F330..0x80170AC0
#   .ctors: 0x8000637C..0x80006380 (offset 0x21c..0x220 in .ctors)
#   .bss: 0x80370EE0..0x80371020 (offset 0x26608..0x26748) -> size 0x140
#   .sdata2: 0x8042FE30..0x8042FE50 (offset 0x2cb0..0x2cd0) -> size 0x20
#   .data: offset 0x2b8c0..0x2b8d0 (size 0x10)
#
# m_vec.cpp:
#   .text: 0x80170AC0..0x80170D90
#   .ctors: 0x80006380..0x80006384 (offset 0x220..0x224 in .ctors)
#   .bss: 0x80371020..0x80371050 (offset 0x26748..0x26778)
#   .sdata2: 0x8042FE50..0x8042FE60 (offset 0x2cd0..0x2ce0)

print("=== m_pad.cpp SYMBOLS IN ALL SECTIONS ===")

# Find symbols in each section range
for sec_name, (start_addr, end_addr) in [
    (".text", (0x8016F330, 0x80170AC0)),
    (".ctors", (0x8000637C, 0x80006380)),
    (".data", (0x8031A000, 0x80320000)),
    (".rodata", (0x802F0000, 0x802F8000)),
    (".bss", (0x80370EE0, 0x80371050)),
    (".sdata", (0x80427000, 0x80429000)),
    (".sbss", (0x8042A000, 0x8042C000)),
    (".sdata2", (0x8042FE28, 0x8042FE60))
]:
    print(f"\n--- {sec_name} ({hex(start_addr)}..{hex(end_addr)}) ---")
    for sec, addr, size, name, rest in symbols:
        if sec == sec_name and start_addr <= addr < end_addr:
            print(f"{hex(addr)} {hex(size):6} {name:50} | {rest}")
