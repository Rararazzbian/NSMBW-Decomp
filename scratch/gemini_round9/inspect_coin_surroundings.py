import sys
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
SPLITS_FILE = ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt'

lines = SYMS_FILE.read_text().splitlines()

entries_by_sec = {}

for line in lines:
    if not line.strip(): continue
    m = re.match(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, sec, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        size_m = re.search(r'size:(0x[0-9A-Fa-f]+)', meta)
        size = int(size_m.group(1), 16) if size_m else 0
        scope_m = re.search(r'scope:(\w+)', meta)
        scope = scope_m.group(1) if scope_m else ''
        type_m = re.search(r'type:(\w+)', meta)
        stype = type_m.group(1) if type_m else ''
        
        if sec not in entries_by_sec:
            entries_by_sec[sec] = []
        entries_by_sec[sec].append({
            'name': name, 'sec': sec, 'addr': addr, 'size': size, 'scope': scope, 'type': stype, 'raw': line.strip()
        })

for sec in entries_by_sec:
    entries_by_sec[sec].sort(key=lambda s: s['addr'])

def print_around(sec_name, target_addr, count_before=5, count_after=10):
    if sec_name not in entries_by_sec:
        print(f"Section {sec_name} not found!")
        return
    sec_entries = entries_by_sec[sec_name]
    idx = 0
    while idx < len(sec_entries) and sec_entries[idx]['addr'] < target_addr:
        idx += 1
    start_idx = max(0, idx - count_before)
    end_idx = min(len(sec_entries), idx + count_after)
    print(f"\n=== Section {sec_name} around 0x{target_addr:08X} ===")
    for i in range(start_idx, end_idx):
        s = sec_entries[i]
        marker = " >>> " if s['addr'] == target_addr else "     "
        print(f"{marker}0x{s['addr']:08X} (0x{s['size']:04X}) [{s['scope']:6s}] {s['name']}")

print_around('.text', 0x800272F0, 4, 25)
print_around('.rodata', 0x802EE7E0, 5, 10)
print_around('.data', 0x80303078, 5, 8)
print_around('.sdata2', 0x8042B630, 8, 15)
print_around('.ctors', 0x802EDCE0, 5, 10) # check ctors
print_around('.bss', 0x80350000, 0, 10) # check bss if any
print_around('.sbss', 0x80429780, 0, 10) # check sbss if any
