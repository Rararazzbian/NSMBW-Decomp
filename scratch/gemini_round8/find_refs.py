import struct

# Let's inspect DOL sections or elf objects
# Or read directly from elf split files or disasm
# Let's check text disassembly around m_heap (0x8016E5F0 - 0x8016ECE0) and m_pad (0x8016F330 - 0x80170AC0)
# to see where @3600, @3601, @3975, @3978, @3981, @3984 are referenced!

import glob, os

def find_refs(syms):
    hits = {s: [] for s in syms}
    for root, dirs, files in os.walk('bin/dtkspl/obj'):
        for f in files:
            if f.endswith('.txt') or f.endswith('.s'):
                path = os.path.join(root, f)
                with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                    for lno, line in enumerate(fh):
                        for s in syms:
                            if s in line:
                                hits[s].append((path, lno+1, line.strip()))
    return hits

# Also let's check all disassembled text files in scratch or dump text
# Let's disassemble auto_03_80160C10_text.o, auto_03_8016ABB0_text.o, auto_03_8016B090_text.o, etc.
