import sys, os, struct, re
sys.path.append('.')
from tools.auto_decomp import pool

def parse_elf_symbols_and_relocs(obj_path):
    d = open(obj_path, 'rb').read()
    shoff = struct.unpack('>I', d[0x20:0x24])[0]
    shent, shnum, shstr = struct.unpack('>HHH', d[0x2E:0x34])

    def sh(i):
        o = shoff + i * shent
        return struct.unpack('>IIIIIIIIII', d[o:o + 40])

    def name_at(tab, x):
        end = d.index(b'\0', tab + x)
        return d[tab + x:end].decode('utf-8', 'replace')

    sn = sh(shstr)[4]
    sec_names = {name_at(sn, sh(i)[0]): i for i in range(shnum)}
    
    symtab_sec = sh(sec_names['.symtab'])
    strtab_off = sh(sec_names['.strtab'])[4]
    
    symbols = []
    for k in range(symtab_sec[5] // 16):
        o = symtab_sec[4] + k * 16
        st_name, st_val, st_size, _, _, st_shndx = struct.unpack('>IIIBBH', d[o:o + 16])
        nm = name_at(strtab_off, st_name) if st_name else ''
        symbols.append({'name': nm, 'value': st_val, 'size': st_size, 'shndx': st_shndx})
        
    relocs = []
    if '.rela.data' in sec_names:
        rela_sec = sh(sec_names['.rela.data'])
        for k in range(rela_sec[5] // 12):
            o = rela_sec[4] + k * 12
            r_offset, r_info, r_addend = struct.unpack('>III', d[o:o + 12])
            sym_idx = r_info >> 8
            relocs.append({'offset': r_offset, 'sym_idx': sym_idx, 'sym_name': symbols[sym_idx]['name'], 'addend': r_addend})
            
    return symbols, relocs

def analyze():
    with open('original/wiimj2d.dol', 'rb') as f:
        dol = f.read()

    sym_map = {}
    with open('bin/dtk/wiimj2d_symbols.txt', encoding='utf-8') as f:
        for line in f:
            m = re.match(r'^(\S+)\s*=\s*\.(?:text|data|rodata|sdata2?|bss):\s*(0x[0-9A-Fa-f]+)', line.strip())
            if m:
                sym_map[int(m.group(2), 16)] = m.group(1)

    vt_va = 0x80314360
    vt_sz = 0x5E4
    off = pool.va_to_off(vt_va, pool.load()[1])
    print(f"Retail vtable: VA 0x{vt_va:08X} (file offset 0x{off:X}), size 0x{vt_sz:X} ({vt_sz // 4} words)")

    retail_slots = []
    for i in range(vt_sz // 4):
        word = struct.unpack('>I', dol[off + i*4 : off + i*4 + 4])[0]
        name = sym_map.get(word, f"0x{word:08X}")
        retail_slots.append((i, word, name))

    symbols, relocs = parse_elf_symbols_and_relocs('scratch/gemini_round20/d_enemy_toride_kokoopa.o')
    
    vt_sym = next(s for s in symbols if s['name'] == '__vt__18dEnTorideKokoopa_c')
    vt_val = vt_sym['value']
    vt_size = vt_sym['size']
    
    print(f"Draft vtable: size 0x{vt_size:X} ({vt_size // 4} words), offset in .data 0x{vt_val:X}")
    print(f"Slot difference: {vt_sz // 4} - {vt_size // 4} = {(vt_sz - vt_size) // 4} slots (0x{vt_sz - vt_size:X} bytes = +0x90 bytes exactly!)")

    draft_slots = {}
    for r in relocs:
        if vt_val <= r['offset'] < vt_val + vt_size:
            slot_idx = (r['offset'] - vt_val) // 4
            draft_slots[slot_idx] = r['sym_name']

    print(f"\nDraft has {len(draft_slots)} relocations across {vt_size // 4} words.")
    
    # Save full slot comparison to file
    with open('scratch/gemini_round20/vtable_alignment.txt', 'w', encoding='utf-8') as out:
        out.write(f"VTABLE COMPARISON: Retail (377 slots) vs Draft (341 slots) -> 36 missing slots (144 bytes = 0x90)\n\n")
        out.write(f"{'Slot':<6} | {'Retail VA':<10} | {'Retail Target Symbol / VA':<60} | {'Draft Symbol':<60}\n")
        out.write("-" * 140 + "\n")
        for r_idx in range(len(retail_slots)):
            r_va = vt_va + r_idx * 4
            r_word, r_name = retail_slots[r_idx][1], retail_slots[r_idx][2]
            d_name = draft_slots.get(r_idx, "<none>")
            out.write(f"{r_idx:<6} | 0x{r_va:08X} | 0x{r_word:08X} {r_name:<50} | {d_name:<60}\n")

    print("Wrote vtable_alignment.txt")

if __name__ == '__main__':
    analyze()
