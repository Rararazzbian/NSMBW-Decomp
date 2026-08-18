import struct
import json
import re
from pathlib import Path
from collections import defaultdict

# Let's inspect original/d_basesNP.rel
with open('original/d_basesNP.rel', 'rb') as f:
    rel_data = f.read()

# REL header:
# id (4 bytes), next (4), prev (4), numSections (4), sectionInfoOffset (4),
# nameOffset (4), nameSize (4), version (4), bssSize (4), relOffset (4),
# impOffset (4), impSize (4), prologSection (1), epilogSection (1),
# unresolvedSection (1), bssSection (1), prolog (4), epilog (4), unresolved (4)

num_secs, sec_info_off = struct.unpack('>II', rel_data[12:20])
print(f"d_basesNP.rel numSections: {num_secs}, secInfoOffset: {hex(sec_info_off)}")

sections = []
for i in range(num_secs):
    off = sec_info_off + i * 8
    sec_off_flags, sec_len = struct.unpack('>II', rel_data[off:off+8])
    sec_off = sec_off_flags & ~1
    is_exec = bool(sec_off_flags & 1)
    sections.append((i, sec_off, sec_len, is_exec))
    print(f"  Section {i}: off={hex(sec_off)} len={hex(sec_len)} exec={is_exec}")

# Section 1 is .text, Section 2 is .ctors, Section 3 is .dtors, Section 4 is .rodata, Section 5 is .data, Section 6 is .bss
