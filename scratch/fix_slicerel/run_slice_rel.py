#!/usr/bin/env python3
# Runs slice_rel.py end-to-end for the three RELs into scratch/fix_slicerel/out.
# Writes ONLY under scratch/fix_slicerel/out -- never bin/compiled or original/.
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
sys.path.insert(0, str(REPO / 'tools' / 'utils'))

import os
os.chdir(REPO)

from slicelib import load_slice_file          # noqa: E402
from slice_rel import slice_rel               # noqa: E402

OUT = REPO / 'scratch' / 'fix_slicerel' / 'out'
ALIAS = REPO / 'alias_db.txt'

for mod in ('d_basesNP', 'd_enemiesNP', 'd_en_bossNP'):
    rel = REPO / 'original' / f'{mod}.rel'
    sf = load_slice_file(REPO / 'slices' / f'{mod}.json')
    unit = sf.unit_name()
    for s in sf.parsed_slices:
        if not s.source or s.nonMatching:
            (OUT / unit / s.sliceName).parent.mkdir(parents=True, exist_ok=True)
    try:
        slice_rel(rel, OUT, ALIAS)
        objs = sorted((OUT / unit).rglob('*.o'))
        tot = sum(o.stat().st_size for o in objs)
        print(f'{mod}: OK  {len(objs)} objects written, {tot} bytes total')
    except Exception as e:
        print(f'{mod}: FAILED  {type(e).__name__}: {e}')
