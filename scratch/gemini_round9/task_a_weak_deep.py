import json
import sys
from pathlib import Path

# Add tools to path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent / 'tools'))

from elffile import ElfFile, ElfSymtab, STB, STT
from slicelib import load_slice_file, SliceType

ROOT = Path(__file__).resolve().parent.parent.parent
COMPILED_DIR = ROOT / 'bin' / 'compiled' / 'wiimj2d'
SLICES_JSON = ROOT / 'slices' / 'wiimj2d.json'
LCF_PATH = ROOT / 'bin' / 'wiimj2d.lcf'
ELF_PATH = ROOT / 'bin' / 'wiimj2d.elf'

slice_file = load_slice_file(SLICES_JSON)

# Check keepWeak and deadstrip in slice_file
print(f"slice_file.keepWeak ({len(slice_file.keepWeak)} items): {slice_file.keepWeak[:10]}")
print(f"slice_file.deadstrip ({len(slice_file.deadstrip)} items): {slice_file.deadstrip[:10]}")

# Read LCF FORCEACTIVE
lcf_text = LCF_PATH.read_text()
force_active = set()
in_force = False
for line in lcf_text.splitlines():
    line = line.strip()
    if line == 'FORCEACTIVE {':
        in_force = True
    elif in_force:
        if line == '}':
            in_force = False
        elif line:
            force_active.add(line)

print(f"Total FORCEACTIVE symbols in bin/wiimj2d.lcf: {len(force_active)}")

# Read wiimj2d.elf symbol table if available
if ELF_PATH.exists():
    final_elf = ElfFile.read(ELF_PATH.read_bytes())
    final_symtab = final_elf.get_section('.symtab')
    final_sym_names = {sym.name: sym for sym in final_symtab.syms} if final_symtab else {}
    print(f"Total symbols in final wiimj2d.elf: {len(final_sym_names)}")
else:
    final_sym_names = {}
    print("ELF file does not exist")

# Let's inspect each compiled slice and check what happened to its weak functions
total_slices = 0
slices_with_weak = 0
total_weak_funcs = 0
weak_funcs_in_forceactive = 0
weak_funcs_in_final_elf = 0
weak_funcs_deadstripped = 0

details = []

for slice in slice_file.slices:
    if not slice.source or slice.nonMatching:
        continue
    
    compiled_o = COMPILED_DIR / Path(slice.source).with_suffix('.o')
    if not compiled_o.exists():
        continue
    
    total_slices += 1
    elf = ElfFile.read(compiled_o.read_bytes())
    symtab = None
    for s in elf.sections:
        if s.name == '.symtab':
            symtab = s
            break
            
    if not symtab:
        continue
        
    weak_in_this_unit = []
    for sym in symtab.syms:
        if sym.st_info_type == STT.STT_FUNC and sym.st_info_bind == STB.STB_WEAK:
            in_fa = sym.name in force_active
            in_elf = sym.name in final_sym_names
            # Check address if in ELF
            addr_in_elf = hex(final_sym_names[sym.name].st_value) if in_elf else None
            weak_in_this_unit.append((sym.name, sym.st_size, in_fa, in_elf, addr_in_elf))
            
            total_weak_funcs += 1
            if in_fa:
                weak_funcs_in_forceactive += 1
            if in_elf:
                weak_funcs_in_final_elf += 1
            else:
                weak_funcs_deadstripped += 1
                
    if weak_in_this_unit:
        slices_with_weak += 1
        details.append((slice.source, slice.memoryRanges.get('.text', 'NONE'), weak_in_this_unit))

print(f"\n--- WEAK SYMBOLS SUMMARY ---")
print(f"Total compiled units analyzed: {total_slices}")
print(f"Units emitting weak functions: {slices_with_weak} ({(slices_with_weak/total_slices)*100:.1f}%)")
print(f"Total weak functions emitted across all units: {total_weak_funcs}")
print(f"Weak functions in FORCEACTIVE (via keepWeak): {weak_funcs_in_forceactive}")
print(f"Weak functions placed in final ELF (referenced or kept): {weak_funcs_in_final_elf}")
print(f"Weak functions deadstripped / omitted by linker: {weak_funcs_deadstripped} ({(weak_funcs_deadstripped/total_weak_funcs)*100:.1f}%)")

print("\n--- DETAILED BREAKDOWN OF UNITS WITH WEAK SYMBOLS ---")
for src, text_range, weaks in details:
    print(f"\nSource: {src} (Slice range: {text_range})")
    for name, size, in_fa, in_elf, addr in weaks:
        status = f"PLACED at {addr}" if in_elf else "DEADSTRIPPED (omitted)"
        fa_str = " [in FORCEACTIVE]" if in_fa else ""
        print(f"  - {name} (0x{size:X}B): {status}{fa_str}")
