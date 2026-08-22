"""Per-function verification for the circle-motion family in d_line_mng.

For each assigned function: word counts, the union gate (bytes / canonical
text), and EVERY pooled constant on both sides decoded to its actual value.
Reuses the object/disassembly poolcheck.py already produced in _poolcheck/.
"""
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
import poolcheck
import pool

WORK = os.path.join(os.path.dirname(os.path.abspath(__file__)), '_poolcheck')
TXT = os.path.join(WORK, 'd.txt')
OBJ = os.path.join(WORK, 'd.o')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')

FNS = [
    'move_on_circle_speedset__10dLineMng_cFff',
    'move_on_circle1__10dLineMng_cFff',
    'move_on_circle2__10dLineMng_cFff',
    'move_on_circle3__10dLineMng_cFff',
    'move_on_circle4__10dLineMng_cFff',
    'CalcAdjustPosY__10dLineMng_cFff',
    'start_line_move__10dLineMng_cFv',
    # callers of the family -- same constant-pool risk class
    'circle_nextpos_set__10dLineMng_cFRC7mVec2_cf',
    'initializeState_Circle__10dLineMng_cFv',
    'executeState_Circle__10dLineMng_cFv',
    'initializeState_Circle2x2Leftup__10dLineMng_cFv',
    'executeState_Circle2x2Leftup__10dLineMng_cFv',
    'initializeState_Circle2x2Rightup__10dLineMng_cFv',
    'executeState_Circle2x2Rightup__10dLineMng_cFv',
    'initializeState_Circle2x2LeftDown__10dLineMng_cFv',
    'executeState_Circle2x2LeftDown__10dLineMng_cFv',
    'initializeState_Circle2x2RightDown__10dLineMng_cFv',
    'executeState_Circle2x2RightDown__10dLineMng_cFv',
    'initializeState_Circle4x4Rightup__10dLineMng_cFv',
    'executeState_Circle4x4Rightup__10dLineMng_cFv',
    'initializeState_Circle4x4LeftUp__10dLineMng_cFv',
    'executeState_Circle4x4LeftUp__10dLineMng_cFv',
    'initializeState_Circle4x4LeftDown__10dLineMng_cFv',
    'executeState_Circle4x4LeftDown__10dLineMng_cFv',
    'initializeState_Circle4x4RightDown__10dLineMng_cFv',
    'executeState_Circle4x4RightDown__10dLineMng_cFv',
]

draft = poolcheck.parse_fns(TXT)
target = poolcheck.parse_fns(TARGET)
dpool = poolcheck.object_pool(OBJ)
dol = pool.load()

for name in FNS:
    t = target.get(name)
    d = draft.get(name)
    print('=' * 78)
    if t is None:
        print(f'{name}: NOT IN TARGET')
        continue
    if d is None:
        print(f'{name}: MISSING FROM DRAFT (target {len(t)}w)')
        continue
    bytes_eq = [b for b, _ in t] == [b for b, _ in d]
    canon_eq = (harness.canonicalise([x for _, x in t])
                == harness.canonicalise([x for _, x in d]))
    print(f'{name}')
    print(f'  target {len(t)}w   draft {len(d)}w   '
          f'bytes_equal={bytes_eq}  canonical_equal={canon_eq}')
    if len(t) != len(d):
        continue
    n = 0
    for i, ((_, tt), (_, dt)) in enumerate(zip(t, d)):
        tm, dm = poolcheck.POOL_REF.match(tt), poolcheck.POOL_REF.match(dt)
        if not tm and not dm:
            continue
        n += 1
        if not tm or not dm:
            print(f'  [{i:3d}] ONE-SIDED pool ref: target={tt!r} draft={dt!r}')
            continue
        if tm.group(1) != dm.group(1):
            print(f'  [{i:3d}] WIDTH MISMATCH: {tt!r} vs {dt!r}')
            continue
        width = 4 if tm.group(1) == 'lfs' else 8
        va, tv = poolcheck.retail_value(tm.group(2), width, dol)
        raw = dpool.get(dm.group(2))
        dv = poolcheck.decode(raw, width) if raw else None
        flag = 'OK ' if tv == dv and tv is not None else '*** DIFFERS ***'
        print(f'  [{i:3d}] {tm.group(1)} retail 0x{va:08X} = {tv!r:24} '
              f'draft = {dv!r:24} {flag}')
    print(f'  {n} pooled load(s)')
