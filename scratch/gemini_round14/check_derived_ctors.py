import struct
import re

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
        m = re.match(r'(\.[a-zA-Z0-9_\$]+):0x([0-9a-fA-F]+);\s*(//\s*size:0x([0-9a-fA-F]+))?', rest)
        if m:
            sec = m.group(1)
            addr = int(m.group(2), 16)
            size = int(m.group(4), 16) if m.group(4) else 0
            symbols.append({'name': sym_name, 'sec': sec, 'addr': addr, 'size': size, 'raw': line})

addr_to_sym = {s['addr']: s for s in symbols}

# Disassemble dEnBossKoopaJrBase_c ctor (0x8009ad30)
# Disassemble dEnTorideKokoopa_c ctor (find its address in symbols)
kokoopa_ctor = [s for s in symbols if '__ct__18dEnTorideKokoopa_cFv' in s['name']][0]
print(f"Kokoopa ctor at {hex(kokoopa_ctor['addr'])}")

def disasm_func(va, sz, name):
    print(f"=== {name} @ {hex(va)} ===")
    for i in range(0, sz, 4):
        w = dol_read_u32(va + i)
        if w is None:
            break
        op = (w >> 26) & 0x3F
        disasm = f"0x{w:08x}"
        if op == 18:
            target = w & 0x03FFFFFC
            if target & 0x02000000:
                target -= 0x04000000
            tva = va + i + target
            sym = addr_to_sym.get(tva)
            sname = sym['name'] if sym else f"0x{tva:08x}"
            disasm += f" -> BL {sname}"
        print(f"  +{hex(i):>4s} (0x{va+i:08x}): {disasm}")

disasm_func(0x8009AD30, 0x80, "dEnBossKoopaJrBase_c ctor")
disasm_func(kokoopa_ctor['addr'], 0x80, "dEnTorideKokoopa_c ctor")

