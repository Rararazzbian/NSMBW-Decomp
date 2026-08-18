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

stats = {
    'total_matching_slices': 0,
    'slices_with_text': 0,
    'slices_overflow_raw': 0,
    'slices_exact_raw': 0,
    'slices_underflow_raw': 0,
    'slices_with_weak': 0,
    'slices_with_unplaced_weak': 0,
    'slices_exact_after_discarding_unplaced_weak': 0,
    'overflow_details': []
}

for slice in slice_file.slices:
    if not slice.source or slice.nonMatching:
        continue
    
    stats['total_matching_slices'] += 1
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
        stats['slices_with_text'] += 1
    else:
        claimed_text_size = 0
        
    raw_diff = compiled_text_size - claimed_text_size
    if raw_diff > 0:
        stats['slices_overflow_raw'] += 1
    elif raw_diff == 0:
        stats['slices_exact_raw'] += 1
    else:
        stats['slices_underflow_raw'] += 1
        
    # Analyze weak functions in this unit
    weak_funcs = []
    unplaced_weak_size = 0
    placed_weak_size = 0
    if symtab:
        for sym in symtab.syms:
            if sym.st_info_type == STT.STT_FUNC and sym.st_info_bind == STB.STB_WEAK:
                in_fa = sym.name in force_active
                in_elf = sym.name in final_sym_names
                # Check if placed in final ELF *at the location corresponding to this TU or elsewhere*
                # Note: If placed elsewhere, it's defined/placed in another slice.
                # If placed in this slice, its st_value in final ELF falls within the slice's runtime address!
                placed_in_this_slice = False
                if in_elf and claimed_text_range:
                    # Let's check runtime address of slice
                    slice_info = slice_file.meta.sections['.text']
                    runtime_start = slice_info.addr + int(start_hex, 16) - slice_info.offset
                    runtime_end = slice_info.addr + int(end_hex, 16) - slice_info.offset
                    elf_sym_val = final_sym_names[sym.name].st_value
                    if runtime_start <= elf_sym_val < runtime_end:
                        placed_in_this_slice = True
                
                weak_funcs.append({
                    'name': sym.name,
                    'size': sym.st_size,
                    'in_fa': in_fa,
                    'in_elf': in_elf,
                    'placed_in_this_slice': placed_in_this_slice,
                    'final_addr': hex(final_sym_names[sym.name].st_value) if in_elf else None
                })
                
                if not placed_in_this_slice:
                    unplaced_weak_size += sym.st_size
                else:
                    placed_weak_size += sym.st_size

    if weak_funcs:
        stats['slices_with_weak'] += 1
        if unplaced_weak_size > 0:
            stats['slices_with_unplaced_weak'] += 1
            
    effective_text_size = compiled_text_size - unplaced_weak_size
    effective_diff = effective_text_size - claimed_text_size
    if effective_diff == 0:
        stats['slices_exact_after_discarding_unplaced_weak'] += 1
    
    if raw_diff > 0 or weak_funcs:
        stats['overflow_details'].append({
            'source': slice.source,
            'compiled': compiled_text_size,
            'claimed': claimed_text_size,
            'raw_diff': raw_diff,
            'unplaced_weak_size': unplaced_weak_size,
            'placed_weak_size': placed_weak_size,
            'effective_diff': effective_diff,
            'weaks': weak_funcs
        })

print("=== STATS SUMMARY ===")
print(f"Total matching slices: {stats['total_matching_slices']}")
print(f"Slices with .text: {stats['slices_with_text']}")
print(f"Raw .text size == claim: {stats['slices_exact_raw']}")
print(f"Raw .text size > claim (overflow): {stats['slices_overflow_raw']}")
print(f"Raw .text size < claim (underflow): {stats['slices_underflow_raw']}")
print(f"Slices emitting weak functions: {stats['slices_with_weak']}")
print(f"Slices with UNPLACED weak functions: {stats['slices_with_unplaced_weak']}")
print(f"Slices exact after accounting for unplaced weak functions: {stats['slices_exact_after_discarding_unplaced_weak']} / {stats['slices_with_text']}")

print("\n=== DETAILED LIST OF OVERFLOWING / WEAK-EMITTING UNITS ===")
for d in stats['overflow_details']:
    print(f"\n{d['source']}:")
    print(f"  Compiled: 0x{d['compiled']:X} | Claim: 0x{d['claimed']:X} | Raw Diff: +0x{d['raw_diff']:X} | Unplaced Weak: 0x{d['unplaced_weak_size']:X} | Effective Diff: 0x{d['effective_diff']:X}")
    for w in d['weaks']:
        status = f"PLACED in this slice ({w['final_addr']})" if w['placed_in_this_slice'] else (f"PLACED elsewhere ({w['final_addr']})" if w['in_elf'] else "DEADSTRIPPED")
        fa = " [FORCEACTIVE]" if w['in_fa'] else ""
        print(f"    - {w['name']}: size=0x{w['size']:X}, status={status}{fa}")
