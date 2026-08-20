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

## Unscoped leads, smallest first

`0xA8470` JR_FLOOR_FIRE_MGR · `0xC5C90` LEMMY_FOOTHOLD ·
`0xD1450` AC_LIFT_REMOCON_SEESAW · `0xF5130` MIDDLE_BG_FOR_CASTLE_LUDWIG ·
`0xFC8D0` MINI_GAME_WIRE_MESH_MGR (its `_OBJ` singleton is `sizeof 0x708 : dActor_c`) ·
`0x77AF0` DUMMY_DOOR_CHILD + `0x77BA0` DUMMY_DOOR_PARENT ·
`0x841E0` FLOOR_JR_B · `0x676F0` BRANCH · `0x152010` AC_WATER_MOVE ·
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
