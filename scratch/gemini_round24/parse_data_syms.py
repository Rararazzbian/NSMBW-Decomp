import struct, os

def parse_elf(path):
    with open(path, 'rb') as f:
        data = f.read()
    
    # ELF32 Big-Endian
    e_shoff = struct.unpack('>I', data[32:36])[0]
    e_shentsize = struct.unpack('>H', data[46:48])[0]
    e_shnum = struct.unpack('>H', data[48:50])[0]
    e_shstrndx = struct.unpack('>H', data[50:52])[0]
    
    # Read section headers
    shstr_hdr = data[e_shoff + e_shstrndx * e_shentsize : e_shoff + (e_shstrndx + 1) * e_shentsize]
    shstr_offset = struct.unpack('>I', shstr_hdr[16:20])[0]
    shstr_size = struct.unpack('>I', shstr_hdr[20:24])[0]
    shstrtab = data[shstr_offset : shstr_offset + shstr_size]
    
    def get_shname(idx):
        end = shstrtab.find(b'\0', idx)
        return shstrtab[idx:end].decode('ascii', errors='ignore')
    
    sections = []
    symtab_hdr = None
    strtab_hdr = None
    data_hdr = None
    for i in range(e_shnum):
        sh = data[e_shoff + i * e_shentsize : e_shoff + (i + 1) * e_shentsize]
        sh_name_idx = struct.unpack('>I', sh[0:4])[0]
        sh_type = struct.unpack('>I', sh[4:8])[0]
        sh_flags = struct.unpack('>I', sh[8:12])[0]
        sh_addr = struct.unpack('>I', sh[12:16])[0]
        sh_offset = struct.unpack('>I', sh[16:20])[0]
        sh_size = struct.unpack('>I', sh[20:24])[0]
        sh_link = struct.unpack('>I', sh[24:28])[0]
        sh_info = struct.unpack('>I', sh[28:32])[0]
        sh_addralign = struct.unpack('>I', sh[32:36])[0]
        sh_entsize = struct.unpack('>I', sh[36:40])[0]
        name = get_shname(sh_name_idx)
        sections.append((name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_entsize, i))
        if sh_type == 2: # SHT_SYMTAB
            symtab_hdr = (sh_offset, sh_size, sh_entsize, sh_link)
        if name == '.data':
            data_hdr = (i, sh_offset, sh_size)

    strtab_offset, strtab_size = struct.unpack('>II', data[e_shoff + symtab_hdr[3] * e_shentsize + 16 : e_shoff + symtab_hdr[3] * e_shentsize + 24])
    strtab = data[strtab_offset : strtab_offset + strtab_size]
    
    def get_symname(idx):
        end = strtab.find(b'\0', idx)
        return strtab[idx:end].decode('ascii', errors='ignore')
    
    syms = []
    sym_offset, sym_size, sym_entsize, _ = symtab_hdr
    num_syms = sym_size // sym_entsize
    for i in range(num_syms):
        sdata = data[sym_offset + i * sym_entsize : sym_offset + (i + 1) * sym_entsize]
        st_name = struct.unpack('>I', sdata[0:4])[0]
        st_value = struct.unpack('>I', sdata[4:8])[0]
        st_size = struct.unpack('>I', sdata[8:12])[0]
        st_info = sdata[12]
        st_other = sdata[13]
        st_shndx = struct.unpack('>H', sdata[14:16])[0]
        name = get_symname(st_name)
        syms.append((name, st_value, st_size, st_shndx, st_info))
    
    return sections, syms, data_hdr

sections, syms, data_hdr = parse_elf('scratch/gemini_round21/d_enemy_toride_kokoopa.o')
data_shndx = data_hdr[0]
print(f'.data section index: {data_shndx}, size: 0x{data_hdr[2]:04X} ({data_hdr[2]} B)')

data_syms = [s for s in syms if s[3] == data_shndx and s[0] != '']
data_syms.sort(key=lambda s: s[1])

print('\n=== DRAFT .o .data SYMBOLS ===')
for name, val, sz, shndx, info in data_syms:
    print(f'offset 0x{val:04X} (size 0x{sz:04X}, {sz:4d} B) : {name}')
