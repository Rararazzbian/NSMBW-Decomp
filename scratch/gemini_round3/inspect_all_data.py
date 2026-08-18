import re
import struct

with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    lines = f.readlines()

syms = []
for line in lines:
    line = line.strip()
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);\s*(?://\s*type:(\S+)\s*size:(0x[0-9a-fA-F]+)(?:\s+scope:(\S+))?(?:\s+align:(\d+))?(?:\s+data:(\S+))?)?', line)
    if m:
        name, sec, addr_s, stype, size_s, scope, align, data_kind = m.groups()
        syms.append({
            'name': name,
            'sec': sec,
            'addr': int(addr_s, 16),
            'size': int(size_s, 16) if size_s else 0,
            'type': stype,
            'scope': scope,
            'align': align,
            'data': data_kind,
            'raw': line
        })

print(f"Total parsed symbols: {len(syms)}")

# Let's inspect the sections in dtk_splits_wiimj2d.txt for adjacent files
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r') as f:
    splits_txt = f.read()

# Let's write a detailed section inspection script
with open('bin/wiimj2d.elf', 'rb') as f:
    elf_data = f.read()

e_shoff = struct.unpack('>I', elf_data[32:36])[0]
e_shentsize = struct.unpack('>H', elf_data[46:48])[0]
e_shnum = struct.unpack('>H', elf_data[48:50])[0]
e_shstrndx = struct.unpack('>H', elf_data[50:52])[0]

sections = []
for i in range(e_shnum):
    offset = e_shoff + i * e_shentsize
    sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info, sh_addralign, sh_entsize = struct.unpack(
        '>10I', elf_data[offset:offset+40]
    )
    sections.append({
        'name_idx': sh_name,
        'type': sh_type,
        'flags': sh_flags,
        'addr': sh_addr,
        'offset': sh_offset,
        'size': sh_size,
        'entsize': sh_entsize
    })

shstrtab = elf_data[sections[e_shstrndx]['offset']:sections[e_shstrndx]['offset'] + sections[e_shstrndx]['size']]
def get_name(idx):
    return shstrtab[idx:].split(b'\x00')[0].decode('latin-1')

for s in sections:
    s['name'] = get_name(s['name_idx'])

def read_vaddr(vaddr, length):
    for s in sections:
        if s['addr'] <= vaddr < s['addr'] + s['size']:
            off = s['offset'] + (vaddr - s['addr'])
            return elf_data[off:off+length]
    return None

# Let's check objects in .rodata, .data, .bss, .sdata, .sbss, .sdata2, .sbss2
print("\n=======================================================")
print("ALL DATA OBJECTS BELONGING TO d_nand_thread.cpp")
print("=======================================================")

# Let's inspect .rodata: 0x802F1470 - 0x802F1498
print("\n--- .rodata range (0x802F1470 - 0x802F1498) ---")
ro_syms = [s for s in syms if s['sec'] == '.rodata' and 0x802F1470 <= s['addr'] < 0x802F1498]
for s in ro_syms:
    raw_b = read_vaddr(s['addr'], s['size'])
    print(f"{hex(s['addr'])} (size {hex(s['size'])}): {s['name']} -> {raw_b}")

# Let's inspect .data: 0x80317CD8 - 0x80317D78
print("\n--- .data range (0x80317CD8 - 0x80317D78) ---")
data_syms = [s for s in syms if s['sec'] == '.data' and 0x80317CD8 <= s['addr'] < 0x80317D78]
for s in data_syms:
    raw_b = read_vaddr(s['addr'], s['size'])
    print(f"{hex(s['addr'])} (size {hex(s['size'])}): {s['name']} -> {raw_b}")

# Let's inspect .bss: 0x80359FC0 - 0x80371000
print("\n--- .bss range (0x80359FC0 - 0x80371000) ---")
bss_syms = [s for s in syms if s['sec'] == '.bss' and 0x80359FC0 <= s['addr'] < 0x80371000]
for s in bss_syms:
    print(f"{hex(s['addr'])} (size {hex(s['size'])} = {s['size']} B): {s['name']}")

# Let's inspect .sdata: 0x80427F78 - 0x80427F88
print("\n--- .sdata range (0x80427F78 - 0x80427F88) ---")
sdata_syms = [s for s in syms if s['sec'] == '.sdata' and 0x80427F78 <= s['addr'] < 0x80427F88]
for s in sdata_syms:
    raw_b = read_vaddr(s['addr'], s['size'])
    print(f"{hex(s['addr'])} (size {hex(s['size'])}): {s['name']} -> {raw_b}")

# Let's inspect .sbss: 0x8042A298 - 0x8042A2A0
print("\n--- .sbss range (0x8042A298 - 0x8042A2A0) ---")
sbss_syms = [s for s in syms if s['sec'] == '.sbss' and 0x8042A298 <= s['addr'] < 0x8042A2A0]
for s in sbss_syms:
    print(f"{hex(s['addr'])} (size {hex(s['size'])}): {s['name']}")

# Let's inspect .sdata2: 0x8042CC98
print("\n--- .sdata2 range (0x8042CC90 - 0x8042CCA0) ---")
sdata2_syms = [s for s in syms if s['sec'] == '.sdata2' and 0x8042CC90 <= s['addr'] <= 0x8042CCA0]
for s in sdata2_syms:
    print(f"{hex(s['addr'])} (size {hex(s['size'])}): {s['name']}")
