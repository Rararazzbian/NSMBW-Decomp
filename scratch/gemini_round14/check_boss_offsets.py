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

sec_symbols = {}
for s in symbols:
    sec_symbols.setdefault(s['sec'], []).append(s)
for sec in sec_symbols:
    sec_symbols[sec].sort(key=lambda x: (x['addr'], x['size']))

text_syms = [s for s in sec_symbols.get('.text', []) if 0x80098350 <= s['addr'] < 0x8009AD30]
for i in range(len(text_syms)):
    curr = text_syms[i]
    if i + 1 < len(text_syms):
        curr['size'] = text_syms[i+1]['addr'] - curr['addr']
    else:
        curr['size'] = 0x8009AD30 - curr['addr']

# Let us find all dEnBoss_c member offsets accessed in dEnBoss_c functions
# We look for lwz/stw/lfs/stfs/lbz/stb/lhz/sth with base registers
# Let us track offsets between 0x500 and 0x700
boss_funcs = [s for s in text_syms if '__9dEnBoss_c' in s['name']]
print(f"Found {len(boss_funcs)} dEnBoss_c member functions")

accessed_offsets = {}
for s in boss_funcs:
    va = s['addr']
    sz = s['size']
    for off in range(0, sz, 4):
        w = dol_read_u32(va + off)
        if w is None:
            continue
        op = (w >> 26) & 0x3F
        # D-form load/store opcodes:
        # lwz=32, lwzu=33, lbz=34, lbzu=35, stw=36, stwu=37, stb=38, stbu=39
        # lhz=40, lhzu=41, lha=42, lhau=43, sth=44, sthu=45, lmw=46, stmw=47
        # lfs=48, lfsu=49, lfd=50, lfdu=51, stfs=52, stfsu=53, stfd=54, stfdu=55
        if 32 <= op <= 55:
            d = w & 0xFFFF
            if d & 0x8000:
                d -= 0x10000
            if 0x500 <= d <= 0x700:
                op_names = {32:'lwz', 34:'lbz', 36:'stw', 38:'stb', 40:'lhz', 44:'sth', 48:'lfs', 50:'lfd', 52:'stfs', 54:'stfd'}
                opn = op_names.get(op, f"op{op}")
                accessed_offsets.setdefault(d, set()).add((opn, s['name']))

print("=== Offsets accessed in dEnBoss_c (>= 0x500) ===")
for d in sorted(accessed_offsets.keys()):
    ops = accessed_offsets[d]
    op_types = set(o[0] for o in ops)
    func_names = set(o[1] for o in ops)
    print(f"  +0x{d:03x}: types={op_types} in {len(func_names)} funcs (e.g. {list(func_names)[0]})")

