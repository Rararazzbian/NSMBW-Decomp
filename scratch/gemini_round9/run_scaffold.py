import os
import sys
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
MWCC = ROOT / 'compilers' / 'Wii' / '1.1' / 'mwcceppc.exe'
CPP = ROOT / 'scratch' / 'gemini_round9' / 'scaffold_coin.cpp'
OBJ = ROOT / 'scratch' / 'gemini_round9' / 'scaffold_coin.o'

cmd = [
    str(MWCC), '-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto',
    '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file',
    '-enc', 'SJIS', '-DREVOLUTION', '-I-',
    '-i', 'include', '-i', 'include/lib', '-i', 'include/lib/MSL', '-i', 'include/lib/MSL/internal',
    '-i', 'include/lib/revolution/BTE/include', '-i', 'include/lib/revolution/BTE/stack/include',
    '-i', 'include/lib/revolution/BTE/stack/btm', '-i', 'include/lib/revolution/BTE/bta/include',
    '-i', 'include/lib/revolution/BTE/bta/sys', '-i', 'include/lib/revolution/BTE/gki/common',
    '-i', 'include/lib/revolution/BTE/gki/platform',
    str(CPP), '-o', str(OBJ)
]

p = subprocess.run(cmd, cwd=str(ROOT), capture_output=True, text=True)
print("Return code:", p.returncode)
print("STDOUT:", p.stdout)
print("STDERR:", p.stderr)

if p.returncode == 0:
    sys.path.insert(0, str(ROOT / 'tools'))
    from elffile import ElfFile, STB, STT
    elf = ElfFile.read(OBJ.read_bytes())
    for s in elf.sections:
        print(f"Section {s.name:10s}: size=0x{s.header.sh_size:04X} ({s.header.sh_size} B)")
    symtab = elf.get_section('.symtab')
    print("\nSymbols:")
    for sym in symtab.syms:
        bind = 'GLOBAL' if sym.st_info_bind == STB.STB_GLOBAL else ('WEAK' if sym.st_info_bind == STB.STB_WEAK else 'LOCAL')
        stype = 'FUNC' if sym.st_info_type == STT.STT_FUNC else ('OBJ' if sym.st_info_type == STT.STT_OBJECT else 'OTHER')
        if sym.st_shndx != 0:
            print(f"  [{bind:6s}] [{stype:4s}] {sym.name:50s} size=0x{sym.st_size:X} val=0x{sym.st_value:X}")
