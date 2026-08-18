import re
import os

with open('scratch/gemini_round3/d_nand_thread_disasm.txt', 'r') as f:
    text = f.read()

# Filter out gap_ functions
raw_blocks = text.split('.fn ')
functions = []
for block in raw_blocks[1:]:
    lines = block.splitlines()
    fn_header = lines[0].split(',')[0].strip()
    if fn_header.startswith('gap_'):
        continue
    # Find address and size from comments above .fn in previous lines
    functions.append((fn_header, lines))

print(f"Total real functions: {len(functions)}")

# Let's inspect the symbols from wiimj2d_symbols.txt for exact addresses & sizes
with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    sym_lines = f.readlines()

sym_map = {}
for line in sym_lines:
    line = line.strip()
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);\s*(?://\s*type:(\S+)\s*size:(0x[0-9a-fA-F]+))?', line)
    if m:
        name, sec, addr_s, stype, size_s = m.groups()
        sym_map[name] = {
            'sec': sec,
            'addr': int(addr_s, 16),
            'size': int(size_s, 16) if size_s else 0,
            'type': stype
        }

for i, (fn_name, lines) in enumerate(functions):
    sym = sym_map.get(fn_name, None)
    addr_str = hex(sym['addr']) if sym else "UNKNOWN"
    size_str = hex(sym['size']) if sym else "UNKNOWN"
    
    # Extract branches/calls in this function
    calls = []
    data_refs = []
    reg_usage = []
    
    for l in lines:
        m_bl = re.search(r'bl\s+([a-zA-Z0-9_@:]+)', l)
        if m_bl:
            calls.append(m_bl.group(1))
        m_ref = re.search(r'/\*\s*([0-9a-fA-F]+:\s*R_[A-Za-z0-9_]+\s+[^*/]+)\*/', l)
        if m_ref:
            data_refs.append(m_ref.group(1))
        m_sda = re.search(r'([a-zA-Z0-9_@]+)@(sda21|ha|l)', l)
        if m_sda:
            data_refs.append(m_sda.group(0))

    print(f"\n[{i+1:2d}] {fn_name} @ {addr_str} (size: {size_str})")
    print(f"     Calls: {list(dict.fromkeys(calls))}")
    print(f"     Data refs: {list(dict.fromkeys(data_refs))[:6]}")
