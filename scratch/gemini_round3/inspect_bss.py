import re

with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    lines = f.readlines()

syms = []
for line in lines:
    line = line.strip()
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);\s*(?://\s*type:(\S+)\s*size:(0x[0-9a-fA-F]+))?', line)
    if m:
        name, sec, addr_s, stype, size_s = m.groups()
        syms.append({'name': name, 'sec': sec, 'addr': int(addr_s, 16), 'size': int(size_s, 16) if size_s else 0, 'type': stype})

bss_syms = [s for s in syms if s['sec'] == '.bss' and 0x8035D000 <= s['addr'] <= 0x80372000]
bss_syms.sort(key=lambda x: x['addr'])

print("=== .bss symbols around 0x8035DFC0 - 0x80371000 ===")
for s in bss_syms:
    print(f"{hex(s['addr'])} (size {hex(s['size'])} = {s['size']:5d} B): {s['name']}")
