import sys
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from dolfile import Dol

dol = Dol(file=open(ROOT / 'original' / 'wiimj2d.dol', 'rb'))

for sec in dol.sections:
    if sec.virt_addr <= 0x80028150 < sec.virt_addr + sec.sec_len:
        offs = 0x80028150 - sec.virt_addr
        data = sec.data[offs : offs + 0x70]
        # Dump instructions
        print("Disassembly of 0x80028150..0x800281C0:")
        for i in range(0, len(data), 4):
            addr = 0x80028150 + i
            val = struct.unpack('>I', data[i:i+4])[0]
            print(f"  0x{addr:08X}: {val:08X}")
