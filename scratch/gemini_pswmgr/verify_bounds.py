import json
import os

with open('slices/wiimj2d.json', encoding='utf-8') as f:
    data = json.load(f)

meta_sections = data['meta']['sections']
print('Meta sections bases:')
for k in ['.text', '.data', '.sbss', '.rodata', '.bss', '.sdata', '.sdata2']:
    if k in meta_sections:
        v = meta_sections[k]
        print(f"  {k:<10}: {v['addr']} (base=0x{int(v['addr'], 16):08X})")

proposed = {
    '.text': ('0xD1F40', '0xD21C0', int(meta_sections['.text']['addr'], 16)),
    '.data': ('0x1A8A8', '0x1A8B8', int(meta_sections['.data']['addr'], 16)),
    '.sbss': ('0x440', '0x448', int(meta_sections['.sbss']['addr'], 16)),
}

print('\nProposed slice memory ranges:')
for sec, (p_start_hex, p_end_hex, base) in proposed.items():
    p_start, p_end = int(p_start_hex, 16), int(p_end_hex, 16)
    addr_start = base + p_start
    addr_end = base + p_end
    size = p_end - p_start
    print(f"  {sec:<8}: offset 0x{p_start:X}-0x{p_end:X} | addr 0x{addr_start:08X}-0x{addr_end:08X} (size 0x{size:X} = {size} B)")

overlaps = 0
for s in data['slices']:
    mr = s.get('memoryRanges', {})
    for sec, (p_start_hex, p_end_hex, base) in proposed.items():
        if sec in mr:
            s_start, s_end = [int(x, 16) for x in mr[sec].split('-')]
            p_start, p_end = int(p_start_hex, 16), int(p_end_hex, 16)
            if max(s_start, p_start) < min(s_end, p_end):
                print(f"  OVERLAP in {sec} with {s.get('source')}: {mr[sec]} vs {p_start_hex}-{p_end_hex}")
                overlaps += 1

if overlaps == 0:
    print("\nOVERLAP CHECK CLEAN: No overlaps with any existing slices in slices/wiimj2d.json!")

# Check neighbouring symbols in wiimj2d_symbols.txt
print("\nChecking symbols in wiimj2d_symbols.txt around proposed addresses:")
with open('bin/dtk/wiimj2d_symbols.txt', encoding='utf-8') as f:
    sym_lines = f.readlines()

for sec, (p_start_hex, p_end_hex, base) in proposed.items():
    p_start, p_end = int(p_start_hex, 16), int(p_end_hex, 16)
    addr_start = base + p_start
    addr_end = base + p_end
    print(f"\n--- {sec} (0x{addr_start:08X} - 0x{addr_end:08X}) ---")
    for line in sym_lines:
        line_s = line.strip()
        if f"={sec}:0x" in line_s.replace(' ', ''):
            parts = line_s.split('=')
            name = parts[0].strip()
            rest = parts[1].strip().split(';')[0].split(':')
            s_addr = int(rest[1], 16)
            if addr_start - 0x20 <= s_addr <= addr_end + 0x20:
                print(f"  0x{s_addr:08X}: {name} ({line_s})")
