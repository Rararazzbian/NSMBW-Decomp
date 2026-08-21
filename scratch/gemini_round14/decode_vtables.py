import json
import re
import struct

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

def dump_vtable(vt_name, vt_addr, size):
    slot_count = (size - 8) // 4
    rtti_0 = dol_read_u32(vt_addr)
    rtti_1 = dol_read_u32(vt_addr + 4)
    print(f"=== VTABLE {vt_name} @ {hex(vt_addr)} (size {hex(size)}, slots: {slot_count}) ===")
    print(f"  header: 0x{rtti_0:08x} 0x{rtti_1:08x}")
    slots = []
    for i in range(slot_count):
        fn_ptr = dol_read_u32(vt_addr + 8 + i * 4)
        sym = addr_to_sym.get(fn_ptr)
        sname = sym['name'] if sym else f"fn_{fn_ptr:08X}"
        slots.append((i, fn_ptr, sname))
    return slots

boss_slots = dump_vtable('__vt__9dEnBoss_c', 0x80312288, 0x390)
kokoopa_slots = dump_vtable('__vt__18dEnTorideKokoopa_c', 0x80314360, 0x5E4)

with open('scratch/gemini_round14/vtable_dEnBoss.txt', 'w', encoding='utf-8') as f:
    for idx, fn_ptr, sname in boss_slots:
        f.write(f"[{idx:3d}] 0x{fn_ptr:08x}: {sname}\n")

with open('scratch/gemini_round14/vtable_kokoopa.txt', 'w', encoding='utf-8') as f:
    for idx, fn_ptr, sname in kokoopa_slots:
        f.write(f"[{idx:3d}] 0x{fn_ptr:08x}: {sname}\n")

print(f"Saved vtable comparisons! boss: {len(boss_slots)} slots, kokoopa: {len(kokoopa_slots)} slots")

