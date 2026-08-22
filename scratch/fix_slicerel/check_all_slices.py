#!/usr/bin/env python3
# Proof that no filler in ANY slice file is negative, and that every section
# tiles monotonically (start of each piece == end of the previous).
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
sys.path.insert(0, str(REPO / 'tools' / 'utils'))

from slicelib import load_slice_file  # noqa: E402

rc = 0
for p in sorted((REPO / 'slices').glob('*.json')):
    try:
        sf = load_slice_file(p)
    except Exception as e:
        print(f'{p.stem}: EXCEPTION {type(e).__name__}: {e}')
        rc = 1
        continue
    per_sec = {}
    neg = 0
    for s in sf.parsed_slices:
        for sec in s.sliceSecs:
            if sec.end_offs < sec.start_offs:
                neg += 1
                print(f'  {p.stem} NEGATIVE {s.sliceName} {sec.sec_name} '
                      f'{sec.start_offs:#x}->{sec.end_offs:#x}')
            per_sec.setdefault(sec.sec_name, []).append(
                (sec.start_offs, sec.end_offs, s.sliceName))
    gaps = 0
    for name, pieces in per_sec.items():
        cur = sf.meta.sections[name].offset
        for st, en, nm in pieces:
            if st != cur:
                gaps += 1
                print(f'  {p.stem} NON-TILING {name}: {nm} starts {st:#x}, expected {cur:#x}')
            cur = en
        end = sf.meta.sections[name].size + sf.meta.sections[name].offset
        if cur != end:
            gaps += 1
            print(f'  {p.stem} SHORT {name}: ends {cur:#x}, section end {end:#x}')
    status = 'CLEAN' if (neg == 0 and gaps == 0) else 'PROBLEM'
    print(f'{p.stem:<14} slices={len(sf.slices):<4} parsed={len(sf.parsed_slices):<4} '
          f'negative={neg} tiling_errors={gaps}  {status}')
    if neg or gaps:
        rc = 1
sys.exit(rc)
