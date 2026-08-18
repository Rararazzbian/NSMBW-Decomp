import sys
import struct
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from dolfile import Dol

SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
DOL_FILE = ROOT / 'original' / 'wiimj2d.dol'

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

# Find address of __vt__15daEnBlockMain_c
block_vt_addr = None
for line in lines:
    if '__vt__15daEnBlockMain_c' in line:
        block_vt_addr = int(re.search(r':(0x[0-9A-Fa-f]+)', line).group(1), 16)
        break

print(f"Block main vtable addr: 0x{block_vt_addr:08X}")
block_vt = read_vtable(block_vt_addr, 0x2EC)
coin_vt = read_vtable(0x80303078, 0x2EC)

print("\n--- DIFFERENCES between daEnBlockMain_c and daEnCoinMain_c vtables ---")
diffs = []
for i in range(len(coin_vt)):
    off, c_val, c_name = coin_vt[i]
    _, b_val, b_name = block_vt[i]
    if c_val != b_val:
        diffs.append((off, b_val, b_name, c_val, c_name))
        print(f"Slot +0x{off:03X}:")
        print(f"   daEnBlockMain_c: 0x{b_val:08X} ({b_name})")
        print(f"   daEnCoinMain_c : 0x{c_val:08X} ({c_name})")

print(f"\nTotal differing slots: {len(diffs)} / {len(coin_vt)}")
