# FLOOR_JR_B -- function inventory

Single-profile unit. `.text 0x841e0-0x84290`, 0xB0 bytes.

**Bounds confirmed independently:**
```
python wip/wm_units/scout_unit.py d_basesNP 0x841e0 0x84290
python wip/wm_units/ctors_map.py  d_basesNP FLOOR_JR_B
```
`scout_unit.py` reports: `profiles whose classInit falls inside: g_profile_FLOOR_JR_B`,
`.data 1 distinct target 0x1cc3c..0x1cc3c`, `.ctors none`. `ctors_map.py` confirms no
`.ctors` entry.

Two-sided ownership check (own script): swept every relocation pair in `d_basesNP` for (a)
any `.text` caller outside `[0x841e0,0x84290)` branching in, and (b) any reader of `0x1cc3c`
outside the range. Both empty -- clean, sole owner both directions.

**Tally: 2/2 matched, byte-identical modulo symbol names.** (Only two functions exist in
this range -- confirmed from the target disassembly: `fn_2_84290` is the very next
function, and it is `FLOOR_JR_C`'s own classInit, i.e. this unit's own claim is exactly
0xB0 bytes with zero slack on either end.)

## Function inventory (both matched)

| draft name | target | size | notes |
|---|---|---|---|
| `daFloorJrB_c_classInit__Fv` (via `ACTOR_PROFILE`) | `fn_2_841E0` | 19/19 | standard expansion, chains through `fn_2_83660` (FLOOR_JR_A's ctor), not `dActor_c`'s |
| `__dt__12daFloorJrB_cFv` | `fn_2_84230` | 22/22 | one-slot flag-arg destructor, empty body, chains through `fn_2_836E0` |

Profile struct (`g_profile_FLOOR_JR_B`) also confirmed byte-identical: target
`fn_2_841E0 / 0x012D013D / 0x00000000`, draft the same three words
(`ACTOR_PROFILE(FLOOR_JR_B, daFloorJrB_c, 0)` -- properties `0`, read directly off the
target's own third word).

Reproduce:
```
python wip/wm_units/agent_floor_jr_b/build.py
python wip/wm_units/agent_floor_jr_b/difftool.py \
    wip/wm_units/agent_floor_jr_b/target_auto_00_0008405C_text.txt \
    wip/wm_units/agent_floor_jr_b/draft.txt \
    fn_2_<addr> <draft_symbol>
```
`build.py`'s own tail runs `verify_anon.py` against `0x841e0-0x84290` and the object list,
per the coordinator's required format; picked up by `order_sweep.py`, which reports
`ok agent_floor_jr_b 2/2`.

## THE ACTUAL FINDING THIS ROUND: this is NOT a self-contained dActor_c unit

Unlike dummy_door/BRANCH, FLOOR_JR_B does not derive from `dActor_c` directly. Its own
constructor calls `fn_2_83660`, not `__ct__8dActor_cFv` -- an address that sits inside a
**separate, much larger, NOT-YET-LANDED unit**:

```
python wip/wm_units/scout_unit.py d_basesNP 0x834ac 0x8405c
```
reports `.text 0x834ac-0x8405c` (0xBB0 bytes), `profiles whose classInit falls inside:
g_profile_FLOOR_JR_A`, and **its own `.ctors` entry** (`0x144 -> __sinit at .text
0x83de0`) plus its own `.bss`/45-target `.rodata`. This is FLOOR_JR_A, confirmed by
address, not inferred: `fn_2_83660` (FLOOR_JR_B's own ctor call target) and `fn_2_836E0`
(FLOOR_JR_B's own dtor call target) both fall inside this span. FLOOR_JR_A's own classInit
(`fn_2_83630`, read fresh from a new dump of
`bin/dtkspl/d_basesNP/obj/auto_00_000834AC_text.o`) shows the identical `li r3, 0x8a8; bl
__nw__7fBase_cFUl; ...; bl fn_2_83660` shape as FLOOR_JR_B's own classInit, confirming
`sizeof(daFloorJrA_c) == 0x8a8` -- the SAME constant FLOOR_JR_B's own `new` allocates, so
FLOOR_JR_B adds NO members of its own; it exists purely to override one destructor slot.
This is the dummy_door CHILD/PARENT pattern one level further out, with the shared base
itself unlandeded.

**This makes FLOOR_JR_B a genuine LANDING-ORDER DEPENDENCY on FLOOR_JR_A** -- the same
shape already recorded in this project's own history (`WM_KILLERBULLET` opened blocked on
`WM_KILLER`). Modelled via a shadow header,
`wip/wm_units/agent_floor_jr_b/shadow_include/game/bases/d_a_floor_jr_a.hpp`:

```cpp
class daFloorJrA_c : public dEn_c {
public:
    virtual ~daFloorJrA_c();          // declared only -- see below
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void initializeState_DieFall();
    virtual void executeState_DieFall();
    virtual void finalizeState_DieFall();
    virtual void unk_83810();          // placeholder -- see vtable-tail note
    virtual void unk_838C0();
    virtual void unk_83B00();
    u8 mUnknown524[0x8a8 - 0x524];     // padding only, real fields unknown
};
```

Every one of these ten methods is DECLARED, never DEFINED, in this unit. **This is
deliberate and load-bearing, not an oversight** -- an in-class/omitted destructor compiles
WEAK and gets synthesised INLINE inside THIS unit's own TU (a first draft without the
explicit `virtual ~daFloorJrA_c();` declaration produced exactly that: a spurious
`__dt__12daFloorJrA_cFv, weak` function in `draft.txt`). A weak symbol defined only in an
un-landed region gets PLACED at link time -- the identical failure mode this project's own
`HANDOFF.md` records for the sandpillar unit's un-landed-region blocker. Declaring it
out-of-line with no body forces it external instead, avoiding FLOOR_JR_B accidentally
supplying FLOOR_JR_A's own destructor body.

**Not routed through the `R_2_1_` free-function convention.** That convention (confirmed
in `HANDOFF.md`, sections `.text`/`.bss`/`.data`) renames an explicit `bl` target or data
reference that THIS unit's own code writes -- it cannot rename a C++ member function's
compiler-chosen mangled symbol (`create__11daFloorJrA_cFv` etc), so it does not apply to
inherited vtable slots. FLOOR_JR_B's own TWO functions do not call any of these ten methods
directly (only FLOOR_JR_B's own destructor slot and the class's size/ctor/dtor addresses
matter to them), so **both verify byte-identical regardless** -- but the vtable OBJECT this
unit owns (`lbl_2_data_1CC3C`) carries ten external, currently-unresolved relocations, and
the REAL 5-binary link will not succeed until FLOOR_JR_A is authored as a real TU defining a
class of this exact name and these exact signatures.

## Gates

- **Function order**: GREEN. `verify_anon.py`'s own build.py run shows no order complaint;
  `order_sweep.py` reports `ok agent_floor_jr_b 2/2`.
- **`.ctors`**: GREEN, by absence -- `ctors_map.py d_basesNP FLOOR_JR_B` reports no entry.
- **Vtable slot assignment**: GREEN on SLOT COUNT and on the ONE slot this unit actually
  owns. `check_vtable.py` against `lbl_2_data_1CC3C` (target, 161 slots / 0x28C bytes) vs
  `__vt__12daFloorJrB_c` (draft, now also 161/0x28C after adding the three tail-placeholder
  virtuals) reports **VTABLE CLEAN**, flagging the ten FLOOR_JR_A-owned slots as
  `inherited, from <addr> -- check whether a header defines it inline` (informational, not
  an error -- exactly what an unlandeded base's slots should read as). A first draft
  without the three tail virtuals read `SLOT COUNT DIFFERS by -3` against this same
  target -- caught and fixed before claiming clean, not left as a residual.

## The three-slot vtable tail: a second finding beyond the base-class dependency

`lbl_2_data_1CC3C` does not end where `dEn_c`'s own virtual chain ends
(`yoshifumiEffect__5dEn_cFP8dActor_c`, `include/game/bases/d_enemy.hpp:220`, the LAST
virtual `dEn_c` itself declares). Three more slots follow --
`fn_2_83810`/`fn_2_838C0`/`fn_2_83B00`. A vtable only grows past its base's final slot when
the DERIVED class declares brand-new virtuals of its own, so these three belong to
FLOOR_JR_A specifically, not to any inherited/overridden dEn_c method. Their real
names/signatures are unrecoverable from within this unit's own scope (FLOOR_JR_B never
calls them) -- stubbed as `unk_83810`/`unk_838C0`/`unk_83B00` purely to hold the slot COUNT
and RELATIVE ORDER, which is all this unit's own correctness depends on.

## Naming

No pooled string names either class (checked both target dumps). `daFloorJrB_c` follows
the established `g_profile_<PROFILE>` -> `da<CamelCase>_c` convention; `daFloorJrA_c` is
the same convention applied to the sibling profile `FLOOR_JR_A`, used here only as a shadow
placeholder for the coordinator's own future authoring of that unit -- if that unit lands
under a different real name, this shadow header (and its `#include` in
`d_a_floor_jr_b.cpp`) is the only thing that needs to change.

## Status: FUNCTION-LEVEL 2/2, BUT NOT LINK-READY

Both of FLOOR_JR_B's own functions are individually verified byte-identical, and the
vtable's shape (slot count, and the one slot this unit owns) is independently checked
clean. **I am NOT reporting this as ready for the 5-binary verify** -- doing so would waste
it, since the link is expected to fail on ten unresolved externals belonging to FLOOR_JR_A
until that unit is authored. This is a genuine landing-order dependency, not a defect in
this draft, and is reported plainly rather than force-fit (the coordinator's own manta/river
rounds set this precedent -- report bounds/dependency surprises directly rather than picking
a convenient story). Once FLOOR_JR_A lands under its real name/signatures, this unit's own
two functions should need no further changes -- only the shadow header's `#include` swapping
for the real one.
