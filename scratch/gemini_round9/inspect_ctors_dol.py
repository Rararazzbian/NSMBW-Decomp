import sys
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
DOL_PATH = ROOT / 'original' / 'wiimj2d.dol'

# Let's inspect .ctors in original DOL
# We can find where .ctors is loaded in DOL
# Or read via elffile if sliced
sys.path.insert(0, str(ROOT / 'tools'))
from dolfile import Dol

dol = Dol(file=open(DOL_PATH, 'rb'))
for idx, sec in enumerate(dol.sections):
    if sec.virt_addr <= 0x802EDD14 < sec.virt_addr + sec.sec_len:
        print(f"Found section {idx}: virt_addr=0x{sec.virt_addr:08X}, len=0x{sec.sec_len:X}")
        offs = 0x802EDD14 - sec.virt_addr
        data = sec.data[offs-8 : offs+32]
        for i in range(0, len(data), 4):
            addr = 0x802EDD14 - 8 + i
            val = struct.unpack('>I', data[i:i+4])[0]
            print(f"  0x{addr:08X}: 0x{val:08X}")
