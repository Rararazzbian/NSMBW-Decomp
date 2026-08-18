with open('scratch/disasm/auto_03_8016F330_text.o.txt') as f:
    text1 = f.read()
with open('scratch/disasm/auto_sinit__m_pad_cpp_text.o.txt') as f:
    text2 = f.read()
with open('scratch/disasm/auto_03_8016F808_text.o.txt') as f:
    text3 = f.read()

# In text3, only up to __arraydtor$13953 (0x8016F87C)
text3_pad = text3.split('__ct__Q26mPrint14MyPrintBase<c>Fv')[0]

full_pad_asm = text1 + '\n' + text2 + '\n' + text3_pad

import re
relocs = set()
for m in re.finditer(r'(?:bl|b)\s+([a-zA-Z0-9_<>@$]+)', full_pad_asm):
    sym = m.group(1)
    if not sym.startswith('.L_'):
        relocs.add(sym)

for m in re.finditer(r'["\']?([a-zA-Z0-9_<>@$]+)["\']?@(ha|l|sda21|sdarx)', full_pad_asm):
    relocs.add(m.group(1))

print("All reloc targets in m_pad.cpp:")
for r in sorted(relocs):
    print(" ", r)
