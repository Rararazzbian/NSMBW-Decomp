import sys
import struct
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from dolfile import Dol

SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
DOL_FILE = ROOT / 'original' / 'wiimj2d.dol'

# Load symbols: addr -> name
lines = SYMS_FILE.read_text().splitlines()
addr_to_sym = {}
for line in lines:
    m = re.match(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, sec, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        addr_to_sym[addr] = name

dol = Dol(file=open(DOL_FILE, 'rb'))

def read_vtable(vtable_addr, size):
    slots = []
    for sec in dol.sections:
        if sec.virt_addr <= vtable_addr < sec.virt_addr + sec.sec_len:
            offs = vtable_addr - sec.virt_addr
            for i in range(0, size, 4):
                val = struct.unpack('>I', sec.data[offs+i : offs+i+4])[0]
                sym_name = addr_to_sym.get(val, f"UNKNOWN_0x{val:08X}")
                slots.append((i, val, sym_name))
            break
    return slots

coin_vt = read_vtable(0x80303078, 0x2EC)
block_vt = read_vtable(0x803029F0, 0x2EC) # let's find block vtable addr
den_vt = read_vtable(0x802FEF98, 0x280) # let's check dEn_c vtable addr

print(f"Total slots in __vt__14daEnCoinMain_c (size 0x2EC): {len(coin_vt)}")
for offset, val, name in coin_vt:
    print(f"  +0x{offset:03X} (0x{val:08X}): {name}")
