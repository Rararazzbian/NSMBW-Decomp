import os
import sys

sys.path.insert(0, os.path.abspath('tools'))
from dolfile import Dol

dol = Dol(open('bin/wiimj2d.dol', 'rb'))
# Find section containing 0x801954B0
for s in dol.sections:
    if s.virt_addr <= 0x801954B0 < s.virt_addr + s.sec_len:
        offset = 0x801954B0 - s.virt_addr
        data = s.data[offset:offset+0x40]
        print("Raw bytes at 0x801954B0:")
        for i in range(0, len(data), 16):
            chunk = data[i:i+16]
            hex_str = ' '.join(f'{b:02X}' for b in chunk)
            print(f'{0x801954B0+i:08X}: {hex_str}')
