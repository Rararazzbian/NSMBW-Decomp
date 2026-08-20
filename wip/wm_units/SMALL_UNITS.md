# Scouted small units — a queue of landing candidates

Nothing landed during a long session in which five agents ground
register-allocation walls on units of 0x1000-0x3000 bytes. Meanwhile the module
holds unlanded units of **0x30 bytes**. This file exists so an agent can be put
on a correctly-scoped small unit immediately, without spending a round deriving
bounds.

**Bounds here are validated, not guessed.** Each was checked with
`scout_unit.py` plus the two-sided ownership test:
- **not SHORT** — no code outside the range reads a pool the range owns;
- **not LONG** — nothing outside the range is called into it unexpectedly.

Re-derive with:
```
python wip/wm_units/scout_unit.py d_basesNP <lo> <hi>
python wip/wm_units/ctors_map.py  d_basesNP <PROFILE_PATTERN>
```

## Top candidate: the RIVER family

**`.text 0x12AD60 - 0x12B400` (0x6A0 bytes), NINE profiles, ONE translation unit.**

```
0x12AD60 RIVER_BARREL     0x12B020 RIVER_MGR       0x12B2A0 RIVER_PUKU
0x12AE10 RIVER_COIN       0x12B140 RIVER_PAIPO     0x12B350 RIVER_STARCOIN
0x12AEC0 RIVER_ITEM       0x12B1F0 RIVER_PAKKUN
0x12AF70 RIVER_LIFT
```

Why this is the best candidate on the board:
- **No `.ctors` entry at all** — the unit has NO static state, so none of the
  `__sinit` ordering, `.ctors`-count or pool-declaration-order complexity that
  has cost other units whole rounds applies here.
- **No `.bss`, and no own `.rodata`** — its only `.rodata` targets are DOL
  absolutes. Its `.data` is nine objects on a regular ~0xF0 stride
  (`0x3AF28`, `0x3B018`, `0x3B108` … `0x3B6A8`), one profile object each.
- **Both bounds clean**: no outside readers of its pools, and no external `.text`
  references into the range.
- 0x6A0 across nine profiles is roughly 0xBC each — classInit plus a small
  create and a method or two per type.

### RIVER: all nine profiles are structurally IDENTICAL

Read before assigning, so the agent starts from a map rather than a range:

```
every profile:  sizeof 0x3D0,  base dActorState_c
classInit (~0x50)  stwu; li r3,0x3D0; bl __nw__7fBase_cFUl; null-check;
                   bl __ct__13dActorState_cFv; lis/addi a pointer; stw it at +0x60
+0x50 (~0x60)      deleting-destructor wrapper: null-check;
                   bl __dt__13dActorState_cFv; bl __dl__7fBase_cFPv
```

`sizeof` is read off `li r3, 0x3D0` feeding `operator new` in each classInit —
exact, not inferred. The four DOL calls resolve to `__nw__7fBase_cFUl`
(`0x80162A00`), `__ct__13dActorState_cFv` (`0x80066FC0`),
`__dt__13dActorState_cFv` (`0x800671B0`), `__dl__7fBase_cFPv` (`0x80162A60`).

So: **roughly two functions per profile, eighteen total at 0x50-0x60 bytes each,
eight of the nine differing only in the pointer stored at `+0x60`.** Get one type
byte-identical and the rest follow almost mechanically — which is exactly why a
unit like this can reach N/N in a round where a 0x3000-byte unit cannot.

**Two questions to settle from the disassembly before authoring**, because the
answers change the source structure rather than a detail of it:
- **What is stored at `+0x60`?** On this project `+0x60` is the secondary vtable
  pointer of a `dBase_c : public fBase_c, public cOwnerSetMg_c` layout, produced
  automatically by ordinary C++ and never to be hand-rolled. But this class
  derives from `dActorState_c`, so verify rather than assume the same thing.
- **Nine distinct classes, or ONE class with nine `classInit` entry points?**
  Identical `sizeof` and identical base constructor across all nine is suspicious
  in a useful way. Whatever the classInits store at `+0x60` should settle it, and
  the answer is the difference between nine small class definitions and one.

## Also scoped and validated

| range | size | profiles | `.ctors` | notes |
|---|---|---|---|---|
| `0x1204E0-0x120F00` | 0xA20 | `PEACH_CASTLE_SEQUENCE_MGR` + `_MGR_OBJ` | 1 @ `0x120D00` | `sizeof 0xB8 : dActor_c`; singleton `lbl_2_bss_D8F8` |
| `0xF8980-0xF9B40` | 0x11C0 | `MINI_GAME_GUN_BATTERY_MGR` + `_MGR_OBJ` | 1 @ `0xF97D0` | `sizeof 0xF4 : dActor_c`; singleton `lbl_2_bss_C460` |

## LANDED from this queue

| unit | range | result |
|---|---|---|
| `d_a_dummy_door.cpp` | `.text 0x77AF0-0x77C50` (0x160) | **4/4, five binaries green** |
| `d_a_peach_castle_sequence.cpp` | `.text 0x1204E0-0x120F00` (0xA20) | **44/44, five binaries green** |

Both from this queue, both in a session where five agents on 0x1000-0x3000-byte
units landed nothing. The queue is the reason.

## Scoped and validated, assigned or ready

| range | size | profiles | `.ctors` | notes |
|---|---|---|---|---|
| `0xF8980-0xF9B40` | 0x11C0 | `MINI_GAME_GUN_BATTERY_MGR` + `_MGR_OBJ` | 1 @ `0xF97D0` | OBJ is `: dBase_c`, `sizeof 0xF4` |
| `0x12AD60-0x12B400` | 0x6A0 | nine `RIVER_*` | none | nine DISTINCT classes, `sizeof 0x3D0 : dActorState_c` |
| `0xF5130-0xF6150` | 0x1020 | `MIDDLE_BG_` + `BOTTOM_BG_FOR_CASTLE_LUDWIG` | 1 @ `0xF5C80` | own `.rodata 0x5BC0`, `.data 0x308F8-0x30F34`, `.bss 0xC1A0-0xC1AC` |
| `0x7D400-0x7D5E0` | 0x1E0 | six `AC_*` switches | none | **whole range references ONE `.data` object** — six classes would need six, so either all six share a class or the bounds are wrong. Settle empirically before authoring. |
| `0x841E0-0x84290` | 0xB0 | `FLOOR_JR_B` | none | one `.data` object `0x1CC3C` |
| `0x676F0-0x677B0` | 0xC0 | `BRANCH` | none | one `.data` object `0x17704` |

## The manager/object pair is a SHAPE, not a coincidence

Five of these are a 0x30-byte manager immediately followed by a much larger
object: `PEACH_CASTLE_SEQUENCE`, `MINI_GAME_GUN_BATTERY`, `MINI_GAME_WIRE_MESH`,
`JR_FLOOR_FIRE`, `LEMMY_FOOTHOLD`, `MIDDLE_BG`/`BOTTOM_BG_FOR_CASTLE_LUDWIG`.

The manager's whole body is "allocate the object"; it reaches exactly one
external address, `operator new`. **Two things follow, both learned the hard
way:**
- **Pool-overlap grouping CANNOT join them** — the manager references no internal
  pools at all. What joins them is the single `.ctors` entry spanning both, plus
  the manager's `createChild` call.
- **The MANAGER is usually `: dActor_c` and the OBJECT usually is NOT.** Two
  separate agents caught me recording the manager's base class against the
  object, on two different units. Any tool that walks outward from a singleton
  store will meet this pair and can attribute the wrong constructor.

## Unscoped leads, smallest first

`0xA8470` JR_FLOOR_FIRE_MGR · `0xC5C90` LEMMY_FOOTHOLD ·
`0xD1450` AC_LIFT_REMOCON_SEESAW · `0xF5130` MIDDLE_BG_FOR_CASTLE_LUDWIG ·
`0xFC8D0` MINI_GAME_WIRE_MESH_MGR (its `_OBJ` singleton is `sizeof 0x708 : dActor_c`) ·
`0x152010` AC_WATER_MOVE ·
the `AC_*` switch run at `0x7D400-0x7D5E0` (six 0x50 profiles, very likely one TU)

## The trap that broke the automatic grouping

Grouping consecutive profiles by SHARED POOL ADDRESSES fails on the commonest
small-unit shape: **a tiny manager whose entire body is "allocate the object"
references no internal pools at all.** `PEACH_CASTLE_SEQUENCE_MGR` is 0x30 bytes
and reaches exactly ONE address — `operator new`. It shares nothing with the
object it creates, so pool overlap says "two units" when it is one.

What actually groups that pair is **the single `.ctors` entry spanning both**,
plus the manager's `createChild__7fBase_cFUsP7fBase_cUlUc` call. So use pool
overlap as evidence, never as the decision, and check `.ctors` ownership and the
create relationship before splitting a manager from its object.

A slice is per TRANSLATION UNIT, not per profile, and mis-scoping from a profile
boundary has cost this project expensively twice (WM_ANTLION with both ends
wrong; WM_ANTLION_MNG at ~79 functions when it is 22).
