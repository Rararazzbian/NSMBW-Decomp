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

**Corrected.** An earlier version presented every row as "dataflow VERIFIED". That
verification was UNSOUND for five of seven rows — see the correction note below —
and one row named the wrong base class. `sizeof` is now separated into what is
MEASURED at the storing site and what is INFERRED from the calling function.

| label | owning unit | sizeof | how | class / base |
|---|---|---|---|---|
| `lbl_2_bss_FEE0` | `WM_KOOPASHIP` | `0x018` | **measured** | `dWmSpline_c`, ctor `(int,int,float)` |
| `lbl_2_bss_FEE4` | `WM_KOOPASHIP` | `0x038` | **measured** | `dWmRouteManager_c` |
| `lbl_2_bss_11B70` | `COURSE_SELECT_MANAGER` | `0x570` | inferred | `: dBase_c`, `+ sStateMethodUsr_FI_c`, `+ dCourseSelectGuide_c @ +0xC8` |
| `lbl_2_bss_C778` | `MINI_GAME_WIRE_MESH_MGR_OBJ` | `0x708` | inferred | `: dActor_c` |
| `lbl_2_bss_5AE8` | `BGM_INTERLOCKING_DUMMY_BLOCK_MGR` | `0x400` | inferred | `: dActor_c`, `+ sStateMethodUsr_FI_c` |
| `lbl_2_bss_D8F8` | `PEACH_CASTLE_SEQUENCE_MGR_OBJ` | `0x0B8` | inferred | `: dActor_c` |
| `lbl_2_bss_C460` | `MINI_GAME_GUN_BATTERY_MGR_OBJ` | `0x0F4` | inferred | **`: dBase_c`** (was wrongly recorded as `dActor_c`) |

**"measured"** means the `operator new` is in the same function as the pointer
store and the stored register traces to it. **"inferred"** means the storing
function allocates nothing — the object arrives as an argument — and the size
comes from the CALLING function's allocation. That is the ordinary
classInit-allocates / constructor-stores-`this` pattern and the number is very
probably right, but it is one inference deep and should be re-derived before a
class layout is built on it.

## The correction, and why it matters more than the numbers

The `MINI_GAME_GUN_BATTERY` row said `: dActor_c`. **It is `dBase_c`.** An agent
authoring the unit caught it from the mangled names, and the check is decisive on
its own: **`sizeof(dActor_c)` is `0x398`, which is larger than the whole `0xF4`
object.** A class cannot be smaller than its own base.

The tool went wrong in two ways, both now fixed:

1. **It searched a fixed byte window backwards from the store**, which silently
   crossed a `blr` into an unrelated function. `MINI_GAME_GUN_BATTERY_MGR_OBJ`'s
   classInit allocates `0xF4` and RETURNS; the singleton store lives in the
   MANAGER's constructor, a different function that allocates nothing. The window
   spanned both and attributed the manager's `__ct__8dActor_cFv` to the object.
2. **Its register trace only handled `mr` moves**, so an overwrite by
   `li`/`lis`/`lwz` did not kill the tracked register. In that same function
   `li r3, 0x164` then `bl createChild` means **the value actually stored is a
   CHILD OBJECT, not the allocation** — and r3 still looked live.

The allocation search is now scoped to the storing function, register-killing
instructions are honoured, and a function boundary ends the trace. When the
storing function allocates nothing the tool says `sizeof NOT DETERMINED from this
site` and reports the caller's allocation explicitly labelled INFERRED.

**The lesson is about the shape of the mistake.** The tool had a warning for
"allocation is far from the store" and it did not fire here, because the
allocation was NEAR — just in a different function. A distance heuristic was
standing in for a correctness check, and it read as confirmation. **A proxy that
usually correlates with correctness will eventually disagree with it, and it is
most convincing exactly when it is wrong.**

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
