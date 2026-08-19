# `d_basesNP` `.bss` singletons — verified reference

Look here before theorising about an unknown `.bss` label. Regenerate with:

```
python wip/wm_units/bss_classify.py d_basesNP            # census: which labels are what
python wip/wm_units/resolve_singleton.py d_basesNP 0xADDR # resolve one
```

## What a "singleton pointer" label is, and why it matters

**These labels are 4-byte POINTERS, not objects.** Code reaches their fields with
a double indirection:

```
lis  r3, lbl@ha
lwz  r5, lbl@l(r3)     <- the cell holds a POINTER
stb  r4, 0x544(r5)     <- 0x544 is a field of the POINTED-TO object
```

So a large displacement seen near one of these labels is **not** an offset into
`.bss` and **not** part of the label's own size. `lbl_2_bss_11B70` was recorded as
"a shared singleton whose type is genuinely UNIDENTIFIED" after an exhaustive
search for a class extending to `+0x55c`. No such class could exist: the label is
four bytes, and every one of those offsets belongs to a heap object.

## The table

Every row's `sizeof` is READ off the `li rN, SIZE` feeding `operator new` — not
derived from the highest field offset anyone happened to observe. Every row
marked verified had the stored register traced back to that allocation's result.

| label | owning profile | sizeof | class / base |
|---|---|---|---|
| `lbl_2_bss_11B70` | `COURSE_SELECT_MANAGER` | `0x570` | `: dBase_c`, `+ sStateMethodUsr_FI_c`, `+ dCourseSelectGuide_c @ +0xC8` |
| `lbl_2_bss_C778` | `MINI_GAME_WIRE_MESH_MGR_OBJ` | `0x708` | `: dActor_c` |
| `lbl_2_bss_5AE8` | `BGM_INTERLOCKING_DUMMY_BLOCK_MGR` | `0x400` | `: dActor_c`, `+ sStateMethodUsr_FI_c` |
| `lbl_2_bss_C460` | `MINI_GAME_GUN_BATTERY_MGR_OBJ` | `0x0F4` | `: dActor_c` |
| `lbl_2_bss_D8F8` | `PEACH_CASTLE_SEQUENCE_MGR_OBJ` | `0x0B8` | `: dActor_c` |
| `lbl_2_bss_FEE4` | `WM_KOOPASHIP` | `0x038` | `dWmRouteManager_c` |
| `lbl_2_bss_FEE0` | `WM_KOOPASHIP` | `0x018` | `dWmSpline_c` (ctor `(int, int, float)`) |

All seven: **dataflow VERIFIED.**

`dCourseSelectGuide_c` is the world map HUD and is **already decompiled** —
`include/game/bases/d_CourseSelectGuide.hpp`, `source/dol/bases/`.

WM_KOOPASHIP owns **two adjacent singletons**, `0xFEE0` and `0xFEE4`. The name
suggests a manager; the first one is a spline. Do not infer a type from the
profile name.

## NOT singletons — and why "two writes" was not enough

| label | owning profile | what it actually is |
|---|---|---|
| `lbl_2_bss_5B30` | `BIGHANA_MGR` | a plain `int` COUNTER — the "create" write is a decrement (`addi r0, r3, -1`) |
| `lbl_2_bss_D6EC` | `OBJ_WENDY` | a plain `int` STATE value — loaded, compared against 1, assigned 2 |

Both have exactly two write sites, so the create/destroy heuristic flagged both
as singleton pointers. **They are not pointers at all**, and hunting for a class
behind either would have found nothing — the same dead end that `0x11B70`
originally caused, arrived at from the opposite direction.

**The discriminator is dereference, not write count.** After the value is loaded
from the label, ask whether it is used as a BASE REGISTER:

```
lwz  r5, lbl@l(r3)
stb  r4, 0x544(r5)     <- r5 used as a base  => a POINTER, something is pointed at
```
```
lwz  r3, lbl@l(r10)
addi r0, r3, -1        <- r3 only arithmetic'd => a VALUE, nothing is pointed at
```

`bss_classify.py` now applies this test, so a two-write label only reports as a
singleton if its loaded value is actually dereferenced. All seven singletons
above still classify correctly; these two now report as plain values.

`lbl_2_bss_0` is `__global_destructor_chain`, not a singleton.

## Two traps this table exists to prevent

1. **The create is NOT the lower-addressed write.** WM_KOOPASHIP's destroy sits
   at the lower address. Identify the destroy as the store whose value comes from
   `li rN, 0`; everything else follows from getting that right.
2. **Resolve external `bl` targets through `bin/dtk/wiimj2d_symbols.txt`**, the
   full DOL map. `syms.txt` is our small curated list and does not contain these
   constructors. Searching the wrong map and finding nothing is not evidence of
   absence — that is precisely how `0x11B70` stayed "unidentified".
