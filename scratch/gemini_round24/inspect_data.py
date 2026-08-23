import struct

def parse_elf(path):
    with open(path, 'rb') as f:
        data = f.read()
    
    # ELF Header (32-bit Big Endian)
    e_ident, e_type, e_machine, e_version, e_entry, e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx = struct.unpack('>16sHHIIIIIHHHHHH', data[:52])
    assert e_ident[:4] == b'\x7fELF', "Not an ELF file"
    
    # Section headers
    sections = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info, sh_addralign, sh_entsize = struct.unpack('>IIIIIIIIII', data[off:off+40])
        sections.append({
            'index': i,
            'name_idx': sh_name,
            'type': sh_type,
            'flags': sh_flags,
            'addr': sh_addr,
            'offset': sh_offset,
            'size': sh_size,
            'link': sh_link,
            'info': sh_info,
            'align': sh_addralign,
            'entsize': sh_entsize,
        })
        
    shstr = sections[e_shstrndx]
    shstr_data = data[shstr['offset']:shstr['offset']+shstr['size']]
    def get_sh_name(idx):
        end = shstr_data.find(b'\x00', idx)
        return shstr_data[idx:end].decode('latin-1')
        
    for s in sections:
        s['name'] = get_sh_name(s['name_idx'])
        
    # Symbol table
    symtab = [s for s in sections if s['name'] == '.symtab'][0]
    strtab = sections[symtab['link']]
    strtab_data = data[strtab['offset']:strtab['offset']+strtab['size']]
    def get_sym_name(idx):
        end = strtab_data.find(b'\x00', idx)
        return strtab_data[idx:end].decode('latin-1')
        
    symbols = []
    num_syms = symtab['size'] // symtab['entsize']
    for i in range(num_syms):
        off = symtab['offset'] + i * symtab['entsize']
        st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack('>IIIBBH', data[off:off+16])
        symbols.append({
            'name': get_sym_name(st_name),
            'value': st_value,
            'size': st_size,
            'info': st_info,
            'shndx': st_shndx
        })
        
    return sections, symbols

sections, symbols = parse_elf('scratch/gemini_round24/d_enemy_toride_kokoopa.o')

print("=== SECTIONS ===")
for s in sections:
    print(f"[{s['index']:2d}] {s['name']:20s} size=0x{s['size']:05X} align={s['align']}")

print("\n=== .data SYMBOLS ===")
data_sec = [s for s in sections if s['name'] == '.data'][0]
for sym in sorted(symbols, key=lambda x: x['value']):
    if sym['shndx'] == data_sec['index'] and sym['size'] > 0:
        print(f"0x{sym['value']:04X} (+0x{sym['size']:04X}) {sym['name']}")
