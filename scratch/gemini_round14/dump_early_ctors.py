import json
import re
import struct

with open('slices/wiimj2d.json') as f:
    slice_data = json.load(f)

sec_meta = slice_data['meta']['sections']

symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('//') or '=' not in line:
            continue
        parts = line.split('=', 1)
        sym_name = parts[0].strip()
        rest = parts[1].strip()
        m = re.match(r'(\.[a-zA-Z0-9_\$]+):0x([0-9a-fA-F]+);\s*(//\s*size:0x([0-9a-fA-F]+))?', rest)
        if m:
            sec = m.group(1)
            addr = int(m.group(2), 16)
            size = int(m.group(4), 16) if m.group(4) else 0
            symbols.append({'name': sym_name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line})

addr_to_sym = {s['addr']: s for s in symbols}

with open('original/wiimj2d.dol', 'rb') as f:
    dol_bytes = f.read()

text_offsets = struct.unpack('>7I', dol_bytes[0:0x1C])
data_offsets = struct.unpack('>11I', dol_bytes[0x1C:0x48])
text_addrs = struct.unpack('>7I', dol_bytes[0x48:0x64])
data_addrs = struct.unpack('>11I', dol_bytes[0x64:0x90])
text_sizes = struct.unpack('>7I', dol_bytes[0x90:0xAC])
data_sizes = struct.unpack('>11I', dol_bytes[0xAC:0xD8])

def va_to_dol_offset(va):
    for o, a, s in zip(text_offsets, text_addrs, text_sizes):
        if a <= va < a + s:
            return o + (va - a)
    for o, a, s in zip(data_offsets, data_addrs, data_sizes):
        if a <= va < a + s:
            return o + (va - a)
    return None

def dol_read_u32(va):
    off = va_to_dol_offset(va)
    if off is not None and off + 4 <= len(dol_bytes):
        return struct.unpack('>I', dol_bytes[off:off+4])[0]
    return None

ctors_base = int(sec_meta['.ctors']['addr'], 16) + int(sec_meta['.ctors'].get('offset', '0x0'), 16)
ctors_size = int(sec_meta['.ctors']['size'], 16)

for idx in range(0, 65):
    entry_va = ctors_base + idx * 4
    fn_ptr = dol_read_u32(entry_va)
    sym = addr_to_sym.get(fn_ptr)
    sym_name = sym['name'] if sym else 'UNKNOWN'
    offset_in_ctors = idx * 4
    print(f"ctors[{idx:3d}] (off +0x{offset_in_ctors:02x}, VA {hex(entry_va)}): ptr={hex(fn_ptr)} -> {sym_name}")

