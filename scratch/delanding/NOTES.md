# De-landing investigation — slices/d_basesNP.json, commit 01bdab9

## Verdict in one line

**Deliberate and correct, and documented — just documented three commits late.**
Commit `6dbeaf3` ("Tree is GREEN") explicitly records the slice removals, names
the four shelved units, gives a defect per unit, and states that the repair was
swept into `01bdab9`/`5348871` by a `git add -A` while a build agent was
mid-flight. There is no silent regression to recover.

The briefing's premise was wrong on three counts:
- it was **five** entries removed, not three;
- `d_a_dummy_door.cpp` was **not** dropped — it was split into `_child`/`_parent`,
  both of which are in the slice file today;
- two units the briefing did not know about (`d_a_peach_castle_sequence`,
  `d_a_wm_sandpillar`) were also shelved.

## The de-landing set

| unit | fate | in slices today |
|---|---|---|
| d_a_dummy_door.cpp | SPLIT into `_child.cpp` + `_parent.cpp` (5348871) | yes (as two) |
| d_a_floor_jr_b.cpp | SHELVED | no |
| d_a_peach_castle_sequence.cpp | SHELVED | no |
| d_a_wm_manta.cpp | SHELVED | no |
| d_a_wm_sandpillar.cpp | SHELVED | no |

Entry counts: 45e72fe 31 -> 01bdab9 26 (-5) -> 5348871 28 (+2) = HEAD 28.

## Measurements taken this round (read-only)

All four sources still compile clean with `module='d_basesNP'`.
Objects in `scratch/delanding/`.

check_bounds (no overlap with anything landed since, for any of the four):
- manta: `.data` opens on `g_profile_WM_MANTA` -> wm-family rule says 0x34 high (1 problem)
- floor_jr_b: 2 problems, both the profile heuristic (not a wm unit — likely noise)
- peach_castle_seq: 1 problem, same profile heuristic
- sandpillar: **BOUNDS PLAUSIBLE**

check_sections:
- manta: `.data` claim 0x98 vs 0xb0 of strong symbols — **REAL DEFECT**
- floor_jr_b: SECTIONS CLEAN
- peach_castle_seq: `.bss` UNDER 0x4 — something missing
- sandpillar: SECTIONS CLEAN

Undefined externals (`scratch/delanding/undef.py`):
- floor_jr_b: 154, incl. `__vt__12daFloorJrA_c`, `__dt__12daFloorJrA_cFv`,
  `create__/execute__/draw__/doDelete__12daFloorJrA_cFv`. FLOOR_JR_A has **no
  source file and no slice entry** anywhere in the tree. Hard blocker.
- manta 50, peach_castle_seq 79, sandpillar 91 (mostly resolved by landed units).
- sandpillar has `R_2_1_171400` — an alias-db FUN-address placeholder.

## build.ninja

**IN SYNC.** mtime 23:48 (newer than both slice files); all 182 slice sources
across the five slice files appear in it; zero sources compiled by ninja that
are absent from a slice file; contains `d_a_dummy_door_child/_parent` and does
NOT contain the four shelved units. `build.ninja` is gitignored (`*.ninja`).
Working tree `slices/` is clean vs HEAD.
