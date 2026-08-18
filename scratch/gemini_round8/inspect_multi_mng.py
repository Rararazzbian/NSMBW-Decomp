import os, sys

def search_symbols(query):
    results = []
    with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8') as f:
        for line in f:
            if query in line:
                results.append(line.strip())
    return results

print("=== Symbols containing dMultiMng ===")
for r in search_symbols('dMultiMng'):
    print(r)

print("\n=== Symbols in .text 0x800CE8E0 .. 0x800CED20 ===")
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8') as f:
    for line in f:
        if '.text' in line:
            parts = line.split()
            sec_addr = parts[2].rstrip(';')
            sec, addr_str = sec_addr.split(':')
            addr = int(addr_str, 16)
            if 0x800CE800 <= addr <= 0x800CED20:
                print(line.strip())

print("\n=== Splits around 0x800CE8F0 ===")
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    for i, line in enumerate(f):
        if any(a in line for a in ['800CE', '800CD', '80317C', '8042A2', '804291', '8042D', '8035', '8036', '8037']):
            # let's filter relevant splits
            if '800CE' in line or '80317C' in line or '8042A2' in line:
                print(f"{i:3d}: {line.strip()}")
