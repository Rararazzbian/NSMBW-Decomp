import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent / 'tools'))

from elffile import ElfFile, ElfSymtab, STB, STT
from slicelib import load_slice_file, SliceType

ROOT = Path(__file__).resolve().parent.parent.parent
COMPILED_DIR = ROOT / 'bin' / 'compiled' / 'wiimj2d'
SLICES_JSON = ROOT / 'slices' / 'wiimj2d.json'
LCF_PATH = ROOT / 'bin' / 'wiimj2d.lcf'
ELF_PATH = ROOT / 'bin' / 'wiimj2d.elf'

slice_file = load_slice_file(SLICES_JSON)

# Parse FORCEACTIVE
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

# Read final ELF symbols
final_elf = ElfFile.read(ELF_PATH.read_bytes())
final_symtab = final_elf.get_section('.symtab')
final_sym_names = {sym.name: sym for sym in final_symtab.syms} if final_symtab else {}

total_units = 0
total_units_with_text = 0
units_exact_raw = 0
units_overflow_raw = 0
units_underflow_raw = 0
units_with_weak = 0
units_with_unplaced_weak = 0
total_weak_symbols = 0
weak_deadstripped = 0
weak_placed_in_origin_slice = 0
weak_placed_in_other_slice = 0
weak_in_forceactive = 0

for slice in slice_file.slices:
    if not slice.source or slice.nonMatching:
        continue
    
    total_units += 1
    compiled_o = COMPILED_DIR / Path(slice.source).with_suffix('.o')
    if not compiled_o.exists():
        continue
    
    elf = ElfFile.read(compiled_o.read_bytes())
    text_sec = None
    symtab = None
    for s in elf.sections:
        if s.name == '.text': text_sec = s
        elif s.name == '.symtab': symtab = s
        
    compiled_text_size = text_sec.header.sh_size if text_sec else 0
    claimed_text_range = slice.memoryRanges.get('.text')
    if claimed_text_range:
        start_hex, end_hex = claimed_text_range.split('-')
        claimed_text_size = int(end_hex, 16) - int(start_hex, 16)
        total_units_with_text += 1
    else:
        claimed_text_size = 0
        
    raw_diff = compiled_text_size - claimed_text_size
    if raw_diff > 0:
        units_overflow_raw += 1
    elif raw_diff == 0:
        units_exact_raw += 1
    else:
        units_underflow_raw += 1
        
    has_weak = False
    has_unplaced_weak = False
    
    if symtab:
        for sym in symtab.syms:
            if sym.st_info_bind == STB.STB_WEAK:
                has_weak = True
                total_weak_symbols += 1
                in_fa = sym.name in force_active
                in_elf = sym.name in final_sym_names
                if in_fa:
                    weak_in_forceactive += 1
                if not in_elf:
                    weak_deadstripped += 1
                    has_unplaced_weak = True
                else:
                    # check address
                    slice_info = slice_file.meta.sections['.text']
                    runtime_start = slice_info.addr + int(start_hex, 16) - slice_info.offset if claimed_text_range else 0
                    runtime_end = slice_info.addr + int(end_hex, 16) - slice_info.offset if claimed_text_range else 0
                    val = final_sym_names[sym.name].st_value
                    if runtime_start <= val < runtime_end:
                        weak_placed_in_origin_slice += 1
                    else:
                        weak_placed_in_other_slice += 1
                        has_unplaced_weak = True
                        
    if has_weak:
        units_with_weak += 1
    if has_unplaced_weak:
        units_with_unplaced_weak += 1

print(f"Total landed/matching TUs in wiimj2d.json: {total_units}")
print(f"Total landed TUs with .text: {total_units_with_text}")
print(f"Raw object .text size == slice claim: {units_exact_raw} ({(units_exact_raw/total_units_with_text)*100:.1f}%)")
print(f"Raw object .text size > slice claim: {units_overflow_raw} ({(units_overflow_raw/total_units_with_text)*100:.1f}%)")
print(f"TUs emitting weak symbols: {units_with_weak} ({(units_with_weak/total_units)*100:.1f}%)")
print(f"TUs carrying unplaced weak symbols: {units_with_unplaced_weak} ({(units_with_unplaced_weak/total_units)*100:.1f}%)")
print(f"\nTotal weak symbol instances across all landed TUs: {total_weak_symbols}")
print(f"  - Deadstripped (unreferenced anywhere): {weak_deadstripped} ({(weak_deadstripped/total_weak_symbols)*100:.1f}%)")
print(f"  - Placed in OTHER TU (deduplicated by linker): {weak_placed_in_other_slice} ({(weak_placed_in_other_slice/total_weak_symbols)*100:.1f}%)")
print(f"  - Placed in THIS TU (surviving definition): {weak_placed_in_origin_slice} ({(weak_placed_in_origin_slice/total_weak_symbols)*100:.1f}%)")
print(f"  - In FORCEACTIVE (via keepWeak): {weak_in_forceactive}")
