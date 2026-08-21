import re
import os

# Assert per-function instruction_count*4 == declared size, per symbol map.
# Target disasm files carry: "# .text:0xN | 0xADDR | size: 0xNN" then .fn blocks.
# Symbol map format: `name = .sec:0xADDR; // type:... size:0xNN`

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
SYMS = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtk\wiimj2d_symbols.txt'

FILES = ['target_8007E17C.txt', 'target_sinit_d_bg_actor_mn.txt', 'target_8007F6D4.txt']

# Parse symbol map: name -> (addr, size)
sym = {}
with open(SYMS, encoding='utf-8', errors='replace') as fh:
    for line in fh:
        m = re.match(r'(\S+)\s*=\s*\.(\w+):(0x[0-9A-Fa-f]+);\s*//.*size:(0x[0-9A-Fa-f]+)', line)
        if m:
            sym[m.group(1)] = (int(m.group(3), 16), int(m.group(4), 16))

SIZE_HINT = re.compile(r'# .text:0x[0-9A-Fa-f]+ \| (0x[0-9A-Fa-f]+) \| size: (0x[0-9A-Fa-f]+)')
FN = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')

issues = 0
for fname in FILES:
    path = os.path.join(BASE, fname)
    with open(path, encoding='utf-8', errors='replace') as fh:
        lines = fh.readlines()
    i = 0
    while i < len(lines):
        s = lines[i].strip()
        m = SIZE_HINT.match(s)
        if not m:
            i += 1
            continue
        addr, size = int(m.group(1), 16), int(m.group(2), 16)
        i += 1
        # scan forward past blank/comment lines to the .fn
        fnline = ''
        while i < len(lines):
            cand = lines[i].strip()
            if cand.startswith('.fn'):
                fnline = cand
                break
            if not cand or cand.startswith('#'):
                i += 1
                continue
            break
        fm = FN.match(fnline)
        if not fm or fnline.startswith('.fn pad_') or fnline.startswith('.fn gap_'):
            i += 1
            continue
        name = fm.group(1).strip().strip('"')
        i += 1
        count = 0
        while i < len(lines):
            e = lines[i].strip()
            if e.startswith('.endfn'):
                i += 1
                break
            if INSN.match(e) and '/*' in e:
                count += 1
            i += 1
        # .4byte data lines (pad/gap) aren't instructions; count only real insns.
        if name in sym:
            saddr, ssize = sym[name]
            if saddr != addr:
                issues += 1
                print('MISMATCH-ADDR %s: disasm 0x%08X sym 0x%08X' % (name, addr, saddr))
            if count * 4 != size or size != ssize:
                issues += 1
                print('MISMATCH-SIZE %s: insns*4=0x%X disasm-size=0x%X sym-size=0x%X' %
                      (name, count * 4, size, ssize))
            else:
                print('OK %-55s @0x%08X size 0x%X (%d insns)' % (name, addr, size, count))
        else:
            print('NOSTS %-55s @0x%08X size 0x%X (%d insns)  [not in symbol map]' %
                  (name, addr, size, count))

print('---')
print('total issues: %d' % issues)
