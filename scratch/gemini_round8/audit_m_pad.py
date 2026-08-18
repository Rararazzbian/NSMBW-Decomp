import os, sys

def parse_symbols(sym_file):
    symbols = []
    with open(sym_file, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            parts = line.split()
            # format: Name = Section:0xAddress; // type:..., size: 0x...
            if '=' in line and ';' in line:
                name = parts[0]
                sec_addr = parts[2].rstrip(';')
                if ':' in sec_addr:
                    sec, addr = sec_addr.split(':')
                    addr = int(addr, 16)
                    # parse size if present
                    size = 0
                    if 'size:' in line:
                        s_idx = line.find('size:')
                        size_str = line[s_idx+5:].split()[0].rstrip(';')
                        size = int(size_str, 16) if size_str.startswith('0x') else int(size_str)
                    symbols.append({
                        'name': name,
                        'section': sec,
                        'addr': addr,
                        'size': size,
                        'raw': line
                    })
    return symbols

symbols = parse_symbols('bin/dtk/wiimj2d_symbols.txt')

print(f"Loaded {len(symbols)} symbols.")

# Let's inspect symbols in each section around m_pad ranges
sections_to_check = ['.text', '.data', '.rodata', '.sdata', '.sdata2', '.bss', '.sbss', '.ctors', '.dtors']

for sec in sections_to_check:
    sec_syms = [s for s in symbols if s['section'] == sec]
    sec_syms.sort(key=lambda s: s['addr'])
    print(f"\n================ Section {sec} (total {len(sec_syms)}) ================")
    
    for i, s in enumerate(sec_syms):
        if sec == '.text' and 0x8016ECE0 <= s['addr'] <= 0x80170E00:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.data' and 0x80329DC0 <= s['addr'] <= 0x8032A000:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.rodata' and 0x80310000 <= s['addr'] <= 0x80360000:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.bss' and 0x80377E00 <= s['addr'] <= 0x80378200:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.sbss' and 0x8042A680 <= s['addr'] <= 0x8042A800:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.sdata' and 0x80429700 <= s['addr'] <= 0x80429800:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.sdata2' and 0x8042DFE0 <= s['addr'] <= 0x8042E0C0:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.ctors' and 0x802EDE00 <= s['addr'] <= 0x802EE000:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
        elif sec == '.dtors' and 0x802EDF00 <= s['addr'] <= 0x802EE100:
            print(f"  [{i}] 0x{s['addr']:08X} (0x{s['size']:04X}) {s['name']}")
