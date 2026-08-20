# DUMMY_DOOR_CHILD + DUMMY_DOOR_PARENT -- function inventory

Coordinator-scoped unit, one translation unit, two profiles. `.text 0x77af0-0x77c50`, 0x160
bytes total, spanning EXACTLY the coordinator's own stated bounds -- confirmed independently
against `bin/dtk/d_basesNP_symbols.txt` (four consecutive symbols, `fn_2_77AF0`/`fn_2_77B40`/
`fn_2_77BA0`/`fn_2_77BF0`, sizes `0x4C`/`0x58`/`0x4C`/`0x58` summing to exactly `0x160`). No
bounds contradiction found; nothing to flag there.

**Tally: 4/4 matched.** Every differing line checked individually; every one is a reference to
this unit's own vtable (`lbl_2_data_1AA14`/`lbl_2_data_1AAF4`) that the target can't name (no
landed header exists anywhere for these classes) but the draft can via its own real mangled
`__vt__` name -- the established naming-only precedent, not assumed from a low count.

## Function inventory (all 4 matched)

| draft name | target | size | diff | notes |
|---|---|---|---|---|
| `daDummyDoorChild_c_classInit__Fv` (via `ACTOR_PROFILE`) | `fn_2_77AF0` | 19/19 | 2 | naming-only (own vtable symbol, 2 lines) |
| `__dt__18daDummyDoorChild_cFv` | `fn_2_77B40` | 22/22 | 0 | EXACT |
| `daDummyDoorParent_c_classInit__Fv` (via `ACTOR_PROFILE`) | `fn_2_77BA0` | 19/19 | 2 | naming-only (own vtable symbol, 2 lines) |
| `__dt__19daDummyDoorParent_cFv` | `fn_2_77BF0` | 22/22 | 0 | EXACT |

Reproduce:
```
python wip/wm_units/agent_dummy_door/build.py
python wip/wm_units/agent_dummy_door/difftool.py \
    wip/wm_units/agent_dummy_door/target_auto_00_000774BC_text.txt \
    wip/wm_units/agent_dummy_door/draft.txt \
    fn_2_<addr> <draft_symbol>
```
`build.py`'s own tail runs `verify_anon.py` against the full `0x77af0-0x77c50` range and an
object list, per the coordinator's own required format -- confirmed picked up by
`python wip/wm_units/order_sweep.py`, which reports `ok agent_dummy_door 4/4`.

## Gates

- **Function order**: GREEN. `verify_anon.py`'s own ascending-pairing (the only order check
  available for a real-named draft against anonymous targets -- `check_fn_order.py` cannot see
  it, confirmed by running it and reading its own "NOT CHECKED" explanation) reports the four
  target addresses paired to draft functions in strictly ascending order, matching the target's
  own `[CHILD-classInit, CHILD-dtor, PARENT-classInit, PARENT-dtor]` layout. Also confirmed via
  `order_sweep.py`'s own tree-wide sweep: `ok agent_dummy_door 4/4`.
- **`.ctors`**: GREEN, by absence. `ctors_map.py d_basesNP DUMMY_DOOR_CHILD` / `..._PARENT` both
  report no entry -- correctly predicted by the coordinator (no static state in this TU).
- **Vtable slot assignment**: GREEN for BOTH classes, checked independently with
  `check_vtable.py` (see below for the tool limitation this ran into and how it was worked
  around).

## Vtable decode (the actual content-determining work this round)

Read both target vtables directly from `target_auto_04_000132B0_data.txt`
(`lbl_2_data_1AA14`/`lbl_2_data_1AAF4`, both 0xD4 bytes = 53 real mangled entries). Every slot in
both resolves to a real, ALREADY-MANGLED `dActor_c`-inherited name EXCEPT ONE: vtable offset
`0x48` (slot 18 by the coordinator's own `(offset-0x08)/4+2` formula), which is `fn_2_77B40` for
CHILD and `fn_2_77BF0` for PARENT -- the one slot each class actually overrides.

That offset is exactly where `virtual ~fBase_c()` (`include/game/framework/f_base.hpp:181`)
first enters the vtable and gets overridden down the chain (`~dBaseActor_c()`
`include/game/bases/d_base_actor.hpp:44`, then `~dActor_c()`). This project's ABI uses ONE
combined destructor slot with a `(this, shouldFree)` flag argument (confirmed directly from
`fn_2_77B40`'s own disasm: `if (this) { dActor_c::~dActor_c(); if (shouldFree > 0)
fBase_c::__dl(this); } return this;`), not two separate complete-object/deleting-destructor
slots -- the coordinator's own general "a virtual destructor consumes TWO slots" hint did not
match what this specific vtable's data shows, so the DATA was trusted over the general hint.

**`create()` is NOT overridden by either class** (vtable slot 2/offset 0x08 is
`create__7fBase_cFv`, `dActor_c`'s own unmodified base, in BOTH vtables) -- so despite the
PARENT/CHILD naming, there is no `createChild__7fBase_cFUsP7fBase_cUlUc` call anywhere in this
unit to author. Checked specifically because the coordinator flagged it as the usual
parent/child pattern; grepped `source/` for a landed `createChild` precedent first, then
confirmed directly against the vtable data that neither class touches `create()` at all. This
is a finding that narrows the coordinator's own heuristic for this specific pair, not a bounds
contradiction.

Neither class adds a single member of its own beyond `dActor_c`. Confirmed two ways: (1) `new
className()`'s own `li r3, 0x398` alloc size, cross-checked against a REAL landed precedent,
`daObjCenter_c` in `include/game/bases/d_a_en_door.hpp` (`class daObjCenter_c : public dActor_c {
u8 m_392[0x39c - 0x392]; u8 mNo; };`) -- its own first added field starts at raw offset `0x392`,
so `dActor_c` itself ends there, and `0x398` is that same size rounded up to 8-byte alignment,
not extra fields; (2) every OTHER vtable slot in both dumps is a real `dActor_c`-inherited
mangled name, with no unexplained gap that would imply an added virtual or a changed layout.

## BINDING, not just bytes

Per the coordinator's explicit warning: an in-class default destructor compiles WEAK in this
compiler and gets deferred to the end of the translation unit (LIFO order), but the target's own
four functions are NOT deferred -- they sit in plain interleaved `[CHILD-classInit, CHILD-dtor,
PARENT-classInit, PARENT-dtor]` address order, exactly where STRONG, out-of-line,
definition-order functions land. Both destructors are declared in-class and DEFINED OUT OF LINE
(empty bodies, `daDummyDoorChild_c::~daDummyDoorChild_c() {}` etc.) specifically to force GLOBAL
binding and correct placement -- confirmed correct after the fact by the clean 4/4 ascending
pairing (a genuinely WEAK/deferred destructor would have shown up out of order or not paired at
all).

(Direct ELF-symbol weak/strong introspection was also tried on the target `.o`,
`bin/dtkspl/d_basesNP/obj/auto_00_000774BC_text.o`, using `check_sections.py`'s own `read_elf()`
-- every symbol in it reports non-weak, which is a property of how `dtk` labels a stripped/split
retail object, not real evidence either way. The ORDER-based argument above is the real evidence
and is what was actually relied on.)

## `.data` section ordering -- a real defect found and fixed, not cosmetic

The FIRST draft, with classes declared `daDummyDoorChild_c` then `daDummyDoorParent_c` (CHILD
first) but with vtable objects apparently emitted by the compiler in some OTHER internal order,
produced `__vt__19daDummyDoorParent_c` BEFORE `__vt__18daDummyDoorChild_c` in `draft.txt` --
backwards relative to the target's own `.data` layout (CHILD's vtable at `0x1AA14` sits below
PARENT's at `0x1AAF4`). This is the same "linker packs a section in emission order" rule already
established for `.bss`/`.text` elsewhere on this project, generalised here to compiler-synthesized
vtable objects -- and since nothing here has a `.ctors` entry to catch a static-init ordering bug,
a swapped `.data` order would otherwise have surfaced only as a REL byte mismatch at the actual
5-binary verify, not as anything visible from the per-function tally.

**Root cause, found by experiment: vtable emission order tracks CLASS DECLARATION order, not
out-of-line member-definition order.** The original draft defined BOTH classes' destructors (the
"key function" that normally anchors vtable emission in this style of compiler) in the SAME
CHILD-then-PARENT order as their class declarations, yet still emitted PARENT's vtable first --
so definition order was not the lever. Swapping ONLY the two `class ... { ... };` declarations
(PARENT declared first, CHILD second) while leaving every `ACTOR_PROFILE`/destructor-definition
line in its original position flipped the vtable emission order to CHILD-first, matching target,
with NO effect on the `.text` function order (confirmed still 4/4 ascending afterward) --
`.text` order is governed by definition-order of the actual function bodies, entirely
independent of class-declaration order, which only vtable emission order is sensitive to.

Verified with `check_vtable.py`, which has a real limitation for a two-class unit worth
recording for whoever reuses it: it grabs the FIRST `__vt__` object it finds anywhere in
`draft.txt` via a bare `re.search` (`wip/wm_units/check_vtable.py:64`), so on a file with two
vtables it silently checks the SAME draft vtable against whichever target label you pass,
producing a false "WRONG SLOT" when the target label doesn't match the vtable it happened to
grab. Confirmed by running it against BOTH target labels and seeing it report the SAME draft
vtable name both times; worked around by isolating each vtable's own `.obj`/`.endobj` block into
a standalone snippet (or, in the post-fix draft, by re-running with the file's own natural
now-correct order) -- both classes independently report **VTABLE CLEAN**.

## Profile data (`g_profile_DUMMY_DOOR_CHILD`/`_PARENT`) -- confirmed byte-identical

Beyond the vtable, checked the two `fProfile::fActorProfile_c` structs field-by-field against the
target dump:

| field | CHILD target | CHILD draft | PARENT target | PARENT draft |
|---|---|---|---|---|
| `mpClassInit` | `fn_2_77AF0` | (own classInit, naming-only) | `fn_2_77BA0` | (own classInit, naming-only) |
| `mExecuteOrder`/`mDrawOrder` (packed) | `0x00CC00CC` | `0x00CC00CC` | `0x00CB00CB` | `0x00CB00CB` |
| `mActorProperties` | `0x00000002` | `0x00000002` | `0x00000000` | `0x00000000` |

`ACTOR_PROFILE(DUMMY_DOOR_CHILD, daDummyDoorChild_c, 2)` / `ACTOR_PROFILE(DUMMY_DOOR_PARENT,
daDummyDoorParent_c, 0)` -- the `2`/`0` properties values read directly from the target's own
`.data`, not guessed; the order fields are the standard macro's own `fProfile::profName`
expansion and need no manual value.

## No shadow header needed

`dActor_c` (`include/game/bases/d_actor.hpp`) is a real, already-landed header covering
everything both classes need (base ctor/dtor, the full inherited vtable, `fBase_c::__nw`/`__dl`).
No shadow copy, no forward-declared placeholder class, no `shadow_include/` directory exists for
this unit.

## Status: READY FOR THE 5-BINARY VERIFY

4/4, order gate green, `.ctors` gate green (by confirmed absence), both vtables independently
CLEAN, profile data byte-identical field by field. Nothing parked, nothing deferred.
