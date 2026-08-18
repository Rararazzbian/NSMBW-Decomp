import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
# Disassemble the object containing 0x800145B0
# Find which auto_ object contains 0x800145B0
dtkspl = ROOT / 'bin' / 'dtkspl' / 'obj'

found_obj = None
for p in dtkspl.rglob('*.o'):
    if 'auto_' in p.name:
        # check address range
        name = p.name
        # format: auto_03_800145B0_text.o or similar
        # let's parse hex
        parts = name.split('_')
        if len(parts) >= 3:
            try:
                addr = int(parts[2], 16)
                if addr <= 0x800145B0 < addr + 0x10000: # rough range
                    print(f"Candidate: {p}")
            except:
                pass
