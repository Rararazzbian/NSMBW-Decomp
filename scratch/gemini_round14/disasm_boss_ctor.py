import struct

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

symbols = []
with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('//') or '=' not in line:
            continue
        parts = line.split('=', 1)
        sym_name = parts[0].strip()
        rest = parts[1].strip()
        import re
        m = re.match(r'(\.[a-zA-Z0-9_\$]+):0x([0-9a-fA-F]+);\s*(//\s*size:0x([0-9a-fA-F]+))?', rest)
        if m:
            sec = m.group(1)
            addr = int(m.group(2), 16)
            size = int(m.group(4), 16) if m.group(4) else 0
            symbols.append({'name': sym_name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line})

addr_to_sym = {s['addr']: s for s in symbols}

ct_va = 0x800983C0
print(f"Constructor of dEnBoss_c @ {hex(ct_va)} (size 0xF0):")
for i in range(0, 0xF0, 4):
    word = dol_read_u32(ct_va + i)
    # Check if branch (bl)
    # opcode 18 is branch (0x48000000)
    opcode = (word >> 26) & 0x3F
    disasm = f"0x{word:08x}"
    if opcode == 18: # B, BL, BA, BLA
        target = word & 0x03FFFFFC
        if target & 0x02000000:
            target -= 0x04000000
        target_va = ct_va + i + target
        sym = addr_to_sym.get(target_va)
        sname = sym['name'] if sym else f"0x{target_va:08x}"
        disasm += f" -> BL {sname}"
    print(f"  +{hex(i):>4s} (0x{ct_va+i:08x}): {disasm}")

