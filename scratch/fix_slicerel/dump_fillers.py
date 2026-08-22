#!/usr/bin/env python3
# Read-only diagnostic: dump every parsed slice section for the three RELs,
# flagging any negative-size section.
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
sys.path.insert(0, str(REPO / 'tools' / 'utils'))

from slicelib import load_slice_file  # noqa: E402

for mod in ('d_basesNP', 'd_enemiesNP', 'd_en_bossNP'):
    p = REPO / 'slices' / f'{mod}.json'
    print(f'===== {mod} =====')
    try:
        sf = load_slice_file(p)
    except Exception as e:
        print(f'  EXCEPTION: {type(e).__name__}: {e}')
        continue
    meta_bss = sf.meta.sections['.bss']
    print(f'  meta .bss: index={meta_bss.index} align={meta_bss.align} '
          f'size={meta_bss.size:#x} offset={meta_bss.offset:#x} addr={meta_bss.addr:#x}')
    bad = 0
    bss_total = 0
    bss_max_end = 0
    for s in sf.parsed_slices:
        for sec in s.sliceSecs:
            size = sec.end_offs - sec.start_offs
            if sec.sec_name == '.bss':
                bss_total += size
                bss_max_end = max(bss_max_end, sec.end_offs)
            if size < 0:
                bad += 1
                print(f'  NEGATIVE: {s.sliceName:<40} {sec.sec_name:<10} '
                      f'{sec.start_offs:#x}-{sec.end_offs:#x} size={size:#x} ({size})')
    print(f'  .bss slices sum       = {bss_total:#x}')
    print(f'  .bss highest end      = {bss_max_end:#x}')
    print(f'  .bss meta size        = {meta_bss.size:#x}')
    print(f'  accounting adds up    = {bss_total == meta_bss.size}')
    print(f'  negative sections     = {bad}')

    # per-section totals for all sections
    print('  --- per-section coverage ---')
    tot = {}
    for s in sf.parsed_slices:
        for sec in s.sliceSecs:
            tot.setdefault(sec.sec_name, 0)
            tot[sec.sec_name] += sec.end_offs - sec.start_offs
    for name, v in sorted(tot.items()):
        ms = sf.meta.sections[name].size
        flag = 'OK ' if v == ms else 'BAD'
        print(f'   {flag} {name:<10} covered={v:#x} meta={ms:#x}')
