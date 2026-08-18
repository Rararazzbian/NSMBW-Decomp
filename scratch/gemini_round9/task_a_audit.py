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

slice_file = load_slice_file(SLICES_JSON)

results = []

for slice in slice_file.slices:
    if not slice.source or slice.nonMatching:
        continue
    
    compiled_o = COMPILED_DIR / Path(slice.source).with_suffix('.o')
    if not compiled_o.exists():
        continue
    
    elf = ElfFile.read(compiled_o.read_bytes())
    text_sec = None
    symtab = None
    for s in elf.sections:
        if s.name == '.text':
            text_sec = s
        elif s.name == '.symtab':
            symtab = s
            
    compiled_text_size = text_sec.header.sh_size if text_sec else 0
    
    claimed_text_range = slice.memoryRanges.get('.text')
    if claimed_text_range:
        start_hex, end_hex = claimed_text_range.split('-')
        claimed_text_size = int(end_hex, 16) - int(start_hex, 16)
    else:
        claimed_text_size = 0
    
    # Analyze symbols in .symtab
    weak_funcs = []
    global_funcs = []
    local_funcs = []
    weak_objs = []
    global_objs = []
    if symtab:
        for sym in symtab.syms:
            if sym.st_info_type == STT.STT_FUNC:
                if sym.st_info_bind == STB.STB_WEAK:
                    weak_funcs.append((sym.name, sym.st_size, hex(sym.st_value)))
                elif sym.st_info_bind == STB.STB_GLOBAL:
                    global_funcs.append((sym.name, sym.st_size, hex(sym.st_value)))
                elif sym.st_info_bind == STB.STB_LOCAL:
                    local_funcs.append((sym.name, sym.st_size, hex(sym.st_value)))
            elif sym.st_info_type == STT.STT_OBJECT:
                if sym.st_info_bind == STB.STB_WEAK:
                    weak_objs.append((sym.name, sym.st_size, hex(sym.st_value)))
                elif sym.st_info_bind == STB.STB_GLOBAL:
                    global_objs.append((sym.name, sym.st_size, hex(sym.st_value)))
                    
    results.append({
        'source': slice.source,
        'compiled_text_size': compiled_text_size,
        'claimed_text_size': claimed_text_size,
        'diff': compiled_text_size - claimed_text_size,
        'num_weak_funcs': len(weak_funcs),
        'weak_funcs': weak_funcs,
        'num_global_funcs': len(global_funcs),
        'num_local_funcs': len(local_funcs),
        'weak_objs': weak_objs
    })

print(f"Total compiled matching slices in wiimj2d.json: {len(results)}")
over_slices = [r for r in results if r['diff'] > 0]
exact_slices = [r for r in results if r['diff'] == 0]
under_slices = [r for r in results if r['diff'] < 0]
units_with_weak = [r for r in results if r['num_weak_funcs'] > 0 or len(r['weak_objs']) > 0]
print(f"Compiled text > Claimed text (Overflow): {len(over_slices)}")
print(f"Compiled text == Claimed text (Exact): {len(exact_slices)}")
print(f"Compiled text < Claimed text (Underflow): {len(under_slices)}")
print(f"Units with ANY weak symbols: {len(units_with_weak)} / {len(results)}")

print("\n--- Overflow units details ---")
for r in over_slices:
    print(f"{r['source']}: compiled=0x{r['compiled_text_size']:X}, claim=0x{r['claimed_text_size']:X}, diff=+0x{r['diff']:X}")
    print(f"  Weak functions ({r['num_weak_funcs']}):")
    weak_sum = 0
    for name, size, val in r['weak_funcs']:
        print(f"    - {name} (size=0x{size:X}, val={val})")
        weak_sum += size
    print(f"  Total weak func size: 0x{weak_sum:X}")

print("\n--- Exact units with weak symbols ---")
for r in exact_slices:
    if r['num_weak_funcs'] > 0:
        print(f"{r['source']}: exact text size 0x{r['compiled_text_size']:X} with {r['num_weak_funcs']} weak funcs:")
        for name, size, val in r['weak_funcs']:
            print(f"    - {name} (size=0x{size:X}, val={val})")
