#!/usr/bin/env python3
# .bss accounting for the three RELs: claimed vs filler, and the ordered tiling.
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
sys.path.insert(0, str(REPO / 'tools' / 'utils'))

from slicelib import load_slice_file  # noqa: E402

for mod in ('d_basesNP', 'd_enemiesNP', 'd_en_bossNP'):
    sf = load_slice_file(REPO / 'slices' / f'{mod}.json')
    claimed = filler = 0
    rows = []
    for s in sf.parsed_slices:
        for sec in s.sliceSecs:
            if sec.sec_name != '.bss':
                continue
            size = sec.end_offs - sec.start_offs
            kind = 'filler' if s.sliceName.startswith('filler_') else 'claim'
            if kind == 'filler':
                filler += size
            else:
                claimed += size
            rows.append((sec.start_offs, sec.end_offs, size, kind, s.sliceName))
    print(f'===== {mod} .bss =====')
    for st, en, sz, kind, nm in rows:
        print(f'  {st:#08x}-{en:#08x}  {sz:#8x}  {kind:<6} {nm}')
    meta = sf.meta.sections['.bss'].size
    print(f'  filler {filler:#x} + claimed {claimed:#x} = {filler + claimed:#x}   '
          f'meta/REL .bss size = {meta:#x}   '
          f'{"MATCH" if filler + claimed == meta else "MISMATCH"}')
    print()
