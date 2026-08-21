import re, sys

LO = 0x800A8710
HI = 0x800B0A20

sym_re = re.compile(r'^(\S+)\s*=\s*\.text:(0x[0-9A-Fa-f]+);\s*//\s*type:function\s*size:(0x[0-9A-Fa-f]+)')

syms = []
with open('bin/dtk/wiimj2d_symbols.txt', encoding='utf-8') as f:
    for line in f:
        m = sym_re.match(line.strip())
        if m:
            name, addr_s, sz_s = m.group(1), m.group(2), m.group(3)
            addr = int(addr_s, 16)
            sz = int(sz_s, 16)
            if LO <= addr < HI:
                syms.append((name, addr, sz))

syms.sort(key=lambda x: x[1])
print(f'Total functions in range: {len(syms)}')
print(f'Total bytes: {sum(s[2] for s in syms)}')

# check for gaps/overlaps
prev_end = LO
gaps = []
for name, addr, sz in syms:
    if addr != prev_end:
        gaps.append((prev_end, addr, addr - prev_end))
    prev_end = addr + sz
if prev_end != HI:
    gaps.append((prev_end, HI, HI - prev_end))

print(f'\nGaps/overlaps (expect none if range is tightly packed): {len(gaps)}')
for g in gaps:
    print(f'  0x{g[0]:08X} - 0x{g[1]:08X} ({g[2]:#x} bytes)')

with open('wip/kokoopa_verify3/target_list.txt', 'w', encoding='utf-8') as f:
    for name, addr, sz in syms:
        f.write(f'{name}\t0x{addr:08X}\t0x{sz:X}\n')
print('\nWrote wip/kokoopa_verify3/target_list.txt')
