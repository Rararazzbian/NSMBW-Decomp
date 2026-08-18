import json
import re
import sys
from pathlib import Path

sys.path.append('tools')
from slicelib import load_slice_file, SliceType

slice_file = load_slice_file(Path('slices/d_basesNP.json'))
print("d_basesNP meta:", slice_file.meta.fileName, slice_file.meta.type)
print("Sections:")
for sname, ssec in slice_file.meta.sections.items():
    print(f"  {sname}: index={ssec.index} size={hex(ssec.size)}")

print(f"\nTotal slices in d_basesNP.json: {len(slice_file.parsed_slices)}")
for s in slice_file.parsed_slices:
    if s.source:
        print(f"  {s.sliceName}: source={s.source}, nonMatching={s.nonMatching}")
        for sec in s.sliceSecs:
            print(f"     {sec.sec_name}: {hex(sec.start_offs)}-{hex(sec.end_offs)} (size {hex(sec.end_offs - sec.start_offs)})")

