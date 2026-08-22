#!/usr/bin/env python3
# Reproduces the -0x10360 crash from the PRE-FIX slice file (45e72fe~1), both
# with the current guard in place and with the guard bypassed.
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'tools'))
sys.path.insert(0, str(REPO / 'tools' / 'utils'))

import slicelib  # noqa: E402
from slicelib import SliceFile, Slice, SliceSection, make_filler_slice  # noqa: E402
from utils.json_parser import from_json  # noqa: E402

BROKEN = REPO / 'scratch' / 'fix_slicerel' / 'd_basesNP.BROKEN.json'

print('--- (1) current slicelib on the PRE-FIX file ---')
try:
    slicelib.load_slice_file(BROKEN)
    print('  no exception -- guard did NOT fire')
except ValueError as e:
    print(f'  ValueError (guard fired): {e}')

print()
print('--- (2) PRE-FIX slicelib behaviour (guard removed) on the PRE-FIX file ---')


def load_no_guard(src: Path) -> SliceFile:
    slice_file = from_json(SliceFile, json.loads(src.read_text()))
    slice_file.path = src
    meta = slice_file.meta
    idx = 0
    curr = {n: s.offset for n, s in meta.sections.items() if s.size != 0}
    for sl in slice_file.slices:
        rng = {sec: (0, 0) for sec in curr}
        name = str(Path(sl.source).with_suffix('.o'))
        ps = Slice(name, sl.source, ccFlags=sl.compilerFlags, nonMatching=sl.nonMatching)
        for section, addrRange in sl.memoryRanges.items():
            info = meta.sections[section]
            begin, end = (int(x, 16) for x in addrRange.split('-'))
            ps.sliceSecs.append(SliceSection(section, info.index, begin, end, info.align))
            rng[section] = (curr[section], begin)   # NO GUARD -- original code
            curr[section] = end
        f = make_filler_slice(f'filler_{idx}.o', rng, meta)
        if f is not None:
            slice_file.parsed_slices.append(f)
            idx += 1
        slice_file.parsed_slices.append(ps)
    rng = {sec: (0, 0) for sec in curr}
    for n, off in curr.items():
        rng[n] = (off, meta.sections[n].size + meta.sections[n].offset)
    f = make_filler_slice(f'filler_{idx}.o', rng, meta)
    if f is not None:
        slice_file.parsed_slices.append(f)
    return slice_file


sf = load_no_guard(BROKEN)
neg = []
for s in sf.parsed_slices:
    for sec in s.sliceSecs:
        size = sec.end_offs - sec.start_offs
        if size < 0:
            neg.append((s.sliceName, sec.sec_name, sec.start_offs, sec.end_offs, size))
print(f'  negative-size sections: {len(neg)}')
for n in neg:
    print(f'    {n[0]:<20} {n[1]:<8} {n[2]:#x} -> {n[3]:#x}   size = {n[4]:#x}  ({n[4]})')

print()
print('  what the .bss NOBITS section header would get (extract_slice line 67):')
for n in neg:
    if n[1] == '.bss':
        print(f'    sh_size = {n[4]:#x}  -> struct.pack("<I", {n[4]}) raises')

print()
print('  PROGBITS fillers that silently slice to zero bytes, and the filler that')
print('  then re-emits the whole region:')
for s in sf.parsed_slices:
    for sec in s.sliceSecs:
        if s.sliceName.startswith('filler_') and sec.sec_name == '.text':
            size = sec.end_offs - sec.start_offs
            if size < 0 or size > 0x100000:
                print(f'    {s.sliceName:<16} .text {sec.start_offs:#x}-{sec.end_offs:#x} size={size:#x}')

print()
print('  per-section coverage under the pre-fix file:')
tot = {}
for s in sf.parsed_slices:
    for sec in s.sliceSecs:
        tot.setdefault(sec.sec_name, 0)
        tot[sec.sec_name] += sec.end_offs - sec.start_offs
for name, v in sorted(tot.items()):
    ms = sf.meta.sections[name].size
    print(f'    {"OK " if v == ms else "BAD"} {name:<8} covered={v:#x} meta={ms:#x}')
