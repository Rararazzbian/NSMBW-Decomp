#!/usr/bin/env python3
# Two negative tests for the guards in slicelib.load_slice_file / make_filler_slice.
#   A) out-of-order slice list  -> caught by the ascending-order guard (45e72fe)
#   B) a claim running past the section's recorded size -> the TRAILING filler
#      goes negative; only the make_filler_slice backstop catches this.
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
sys.path.insert(0, str(REPO / 'tools' / 'utils'))
HERE = Path(__file__).resolve().parent

from slicelib import load_slice_file  # noqa: E402

print('A) pre-fix (alphabetically re-sorted) d_basesNP.json:')
try:
    load_slice_file(HERE / 'd_basesNP.BROKEN.json')
    print('   NOT CAUGHT')
except ValueError as e:
    print(f'   caught: {str(e).splitlines()[0][:160]}')

# B) synthetic: last .bss claim extended past the section end
d = json.loads((REPO / 'slices' / 'd_basesNP.json').read_text())
for s in d['slices']:
    if s['source'] == 'd_basesNP/bases/d_a_wm_tower.cpp':
        s['memoryRanges']['.bss'] = '0x10350-0x13000'   # past .bss size 0x12484
p = HERE / 'd_basesNP.OVERCLAIM.json'
p.write_text(json.dumps(d, indent=4))
print('B) synthetic over-claim (.bss 0x10350-0x13000, section size 0x12484):')
try:
    sf = load_slice_file(p)
    bad = [(s.sliceName, sec.sec_name, sec.start_offs, sec.end_offs)
           for s in sf.parsed_slices for sec in s.sliceSecs
           if sec.end_offs < sec.start_offs]
    print(f'   NOT CAUGHT -- negative sections produced: {bad}')
except ValueError as e:
    print(f'   caught: {str(e).splitlines()[0][:200]}')

# C) the good files must still load
print('C) all five real slice files:')
for f in sorted((REPO / 'slices').glob('*.json')):
    try:
        load_slice_file(f)
        print(f'   {f.stem}: ok')
    except Exception as e:
        print(f'   {f.stem}: FAILED {type(e).__name__}: {e}')
