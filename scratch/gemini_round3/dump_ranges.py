import re

with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    lines = f.readlines()

syms = []
for line in lines:
    line = line.strip()
    if not line:
        continue
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);\s*(?://\s*type:(\S+)\s*size:(0x[0-9a-fA-F]+))?', line)
    if m:
        name, sec, addr_s, stype, size_s = m.groups()
        addr = int(addr_s, 16)
        sz = int(size_s, 16) if size_s else 0
        syms.append({'name': name, 'sec': sec, 'addr': addr, 'type': stype, 'size': sz, 'raw': line})

syms_by_sec = {}
for s in syms:
    syms_by_sec.setdefault(s['sec'], []).append(s)

for sec in syms_by_sec:
    syms_by_sec[sec].sort(key=lambda x: x['addr'])

def print_sec_range(sec, start, end):
    print(f"\n================ Section {sec} ({hex(start)} - {hex(end)}) ================")
    in_range = [s for s in syms_by_sec.get(sec, []) if start <= s['addr'] <= end]
    for s in in_range:
        print(f"{hex(s['addr'])} (size {hex(s['size'])} = {s['size']:5d} B): {s['name']} [{s['type']}]")

# Let's check:
# 1. .text (0x800CE8F0 - 0x800D03C0)
print_sec_range('.text', 0x800CE8F0, 0x800D03C0)

# 2. .rodata (0x802F1450 - 0x802F1500)
print_sec_range('.rodata', 0x802F1450, 0x802F1500)

# 3. .data (0x80317CC0 - 0x80317E00)
print_sec_range('.data', 0x80317CC0, 0x80317E00)

# 4. .bss (0x80361F00 - 0x80371050)
print_sec_range('.bss', 0x80361F00, 0x80371050)

# 5. .sdata (0x80427F60 - 0x80427FC0)
print_sec_range('.sdata', 0x80427F60, 0x80427FC0)

# 6. .sbss (0x8042A280 - 0x8042A2C0)
print_sec_range('.sbss', 0x8042A280, 0x8042A2C0)

# 7. .sdata2 (0x8042CC80 - 0x8042CCC0)
print_sec_range('.sdata2', 0x8042CC80, 0x8042CCC0)

# 8. .sbss2
print_sec_range('.sbss2', 0x8042E000, 0x8042E100)
