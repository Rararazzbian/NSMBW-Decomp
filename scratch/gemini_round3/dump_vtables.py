import struct

with open('bin/wiimj2d.elf', 'rb') as f:
    elf_data = f.read()

# Let's parse ELF program headers / section headers to read memory at virtual addresses
# Or we can parse ELF section table
e_shoff = struct.unpack('>I', elf_data[32:36])[0]
e_shentsize = struct.unpack('>H', elf_data[46:48])[0]
e_shnum = struct.unpack('>H', elf_data[48:50])[0]
e_shstrndx = struct.unpack('>H', elf_data[50:52])[0]

# Read section headers
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

# Dump .data around vtables: 0x80317D48
print("=== VTABLE dNandThread_c (0x80317D48) ===")
vt_bytes = read_vaddr(0x80317D48, 0x18)
for i in range(0, len(vt_bytes), 4):
    w = struct.unpack('>I', vt_bytes[i:i+4])[0]
    print(f"+0x{i:02X} (0x{0x80317D48+i:08X}): 0x{w:08X}")

print("\n=== VTABLE mMutex (0x80317D60) ===")
vt_m = read_vaddr(0x80317D60, 0xC)
for i in range(0, len(vt_m), 4):
    w = struct.unpack('>I', vt_m[i:i+4])[0]
    print(f"+0x{i:02X} (0x{0x80317D60+i:08X}): 0x{w:08X}")

print("\n=== VTABLE EGG::Mutex (0x80317D6C) ===")
vt_e = read_vaddr(0x80317D6C, 0xC)
for i in range(0, len(vt_e), 4):
    w = struct.unpack('>I', vt_e[i:i+4])[0]
    print(f"+0x{i:02X} (0x{0x80317D6C+i:08X}): 0x{w:08X}")
