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

# Sort syms by section, then address
syms_by_sec = {}
for s in syms:
    syms_by_sec.setdefault(s['sec'], []).append(s)

for sec in syms_by_sec:
    syms_by_sec[sec].sort(key=lambda x: x['addr'])

print("=== Check each section around d_nand_thread ===")

def inspect_range(sec, start, end):
    print(f"\n--- Section {sec} ({hex(start)} - {hex(end)}) ---")
    in_range = [s for s in syms_by_sec.get(sec, []) if start - 0x100 <= s['addr'] <= end + 0x100]
    for s in in_range:
        marker = "  >>> " if start <= s['addr'] < end else "      "
        print(f"{marker}{hex(s['addr'])} (size {hex(s['size'])}): {s['name']} [{s['type']}]")

inspect_range('.text', 0x800CED00, 0x800CFCE0)
inspect_range('.rodata', 0x802F1460, 0x802F1600)
inspect_range('.data', 0x80317CC8, 0x80317F00)
inspect_range('.bss', 0x80361F00, 0x80371050)
inspect_range('.sdata', 0x80427F60, 0x80427FC0)
inspect_range('.sbss', 0x8042A280, 0x8042A2C0)
inspect_range('.sdata2', 0x8042CC80, 0x8042CCC0)
inspect_range('.sbss2', 0x8042E000, 0x8042E100)
inspect_range('.ctors', 0x802ED000, 0x802EE000)
inspect_range('.dtors', 0x802EE000, 0x802EF000)
