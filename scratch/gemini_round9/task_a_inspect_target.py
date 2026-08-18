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

for slice in slice_file.slices:
    if 'multi_manager' in slice.source or 'player_manager' in slice.source or 'm_pad' in slice.source:
        print(f"Slice: {slice.source}, nonMatching={slice.nonMatching}, ranges={slice.memoryRanges}")
        compiled_o = COMPILED_DIR / Path(slice.source).with_suffix('.o')
        print(f"Compiled obj exists: {compiled_o.exists()}")
        if compiled_o.exists():
            elf = ElfFile.read(compiled_o.read_bytes())
            text_sec = None
            symtab = None
            for s in elf.sections:
                if s.name == '.text': text_sec = s
                elif s.name == '.symtab': symtab = s
            print(f"  .text size: {hex(text_sec.header.sh_size) if text_sec else 'None'}")
            for sym in symtab.syms:
                if sym.st_info_bind == STB.STB_WEAK:
                    print(f"  weak sym: {sym.name} ({hex(sym.st_size)})")
