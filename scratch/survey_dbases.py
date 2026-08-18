import re
import json

# Let's inspect d_basesNP_symbols.txt
symbols = []
with open('bin/dtk/d_basesNP_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sec, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            type_m = re.search(r'type:(\w+)', comment)
            sym_type = type_m.group(1) if type_m else 'unknown'
            addr_int = int(addr, 16)
            symbols.append({
                'name': name.strip(),
                'sec': sec.strip(),
                'addr': addr_int,
                'size': size,
                'type': sym_type,
                'comment': comment
            })

print(f"Total symbols in d_basesNP_symbols.txt: {len(symbols)}")

# Let's find all __sinit_ / __vt__ symbols to identify TUs and classes
sinits = [s for s in symbols if '__sinit_' in s['name']]
vtables = [s for s in symbols if s['name'].startswith('__vt__')]

print(f"Total __sinit_ symbols: {len(sinits)}")
print(f"Total __vt__ symbols: {len(vtables)}")

for s in sinits:
    print(f"  {hex(s['addr'])}: {s['name']}")

