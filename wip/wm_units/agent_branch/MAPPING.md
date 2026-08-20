# BRANCH -- function inventory

Single-profile, single-class unit. `.text 0x676f0-0x677b0`, 0xC0 bytes.

**Bounds confirmed independently:**
```
python wip/wm_units/scout_unit.py d_basesNP 0x676f0 0x677b0
python wip/wm_units/ctors_map.py  d_basesNP BRANCH
```
`scout_unit.py` reports: `profiles whose classInit falls inside: g_profile_BRANCH`,
`.data 1 distinct target 0x17704..0x17704`, `.ctors none`. `ctors_map.py` confirms no
`.ctors` entry for BRANCH.

Two-sided ownership check (own script, not one of the coordinator's canned tools): swept
every `(section,addr)->(section,addr)` relocation pair in `d_basesNP` for (a) any `.text`
caller OUTSIDE `[0x676f0,0x677b0)` branching INTO the range, and (b) any reader of
`0x17704` (the vtable) OUTSIDE the range. Both come back empty -- clean, sole owner both
directions.

**Tally: 3/3 matched, byte-identical modulo symbol names.**

## Function inventory (all 3 matched)

| draft name | target | size | notes |
|---|---|---|---|
| `daBranch_c_classInit__Fv` (via `ACTOR_PROFILE`) | `fn_2_676F0` | 19/19 | standard `ACTOR_PROFILE` expansion |
| `create__10daBranch_cFv` | `fn_2_67740` | 2/2 | `return FAILED;` (PACK_RESULT_e = 2) |
| `__dt__10daBranch_cFv` | `fn_2_67750` | 22/22 | one-slot flag-arg destructor, empty body |

Reproduce:
```
python wip/wm_units/agent_branch/build.py
python wip/wm_units/agent_branch/difftool.py \
    wip/wm_units/agent_branch/target_auto_00_00067344_text.txt \
    wip/wm_units/agent_branch/draft.txt \
    fn_2_<addr> <draft_symbol>
```
`build.py`'s own tail runs `verify_anon.py` against the full `0x676f0-0x677b0` range and
the object list, per the coordinator's required format; picked up by
`python wip/wm_units/order_sweep.py`, which reports `ok agent_branch 3/3`.

## Gates

- **Function order**: GREEN. `verify_anon.py`'s own build.py run showed no "FUNCTION ORDER
  IS WRONG" complaint (`check_fn_order.py` cannot check a real-named draft -- confirmed by
  running it directly and reading its own "NOT CHECKED" explanation). `order_sweep.py`
  reports `ok agent_branch 3/3`.
- **`.ctors`**: GREEN, by absence -- `ctors_map.py d_basesNP BRANCH` reports no entry,
  matching the coordinator's own prediction (no static state in this TU).
- **Vtable slot assignment**: GREEN. `check_vtable.py` against `lbl_2_data_17704` (target)
  vs `__vt__10daBranch_c` (draft) reports **VTABLE CLEAN**, both 51 slots / 0xD4 bytes.
  (Single class in this file, so the tool's known "grabs the FIRST `__vt__` object via a
  bare `re.search`" limitation from dummy_door's own MAPPING does not apply here -- there
  is only one vtable to find.)

## Vtable decode

Read directly from `target_auto_04_000132B0_data.txt`, `lbl_2_data_17704` (0xD4 bytes = 53
raw words = 51 named/checked slots after the two leading zero words). Every slot resolves
to a real, already-landed `dActor_c`-inherited name EXCEPT TWO:

- **offset 0x08** (slot 2, `create__7fBase_cFv`'s slot in the unmodified base) is
  `fn_2_67740`: `li r3, 0x2; blr` -- `return fBase_c::FAILED;` (`PACK_RESULT_e`:
  `NOT_READY=0, SUCCEEDED=1, FAILED=2`, from `include/game/framework/f_base.hpp:44-48`).
  BRANCH's own `create()` step always fails/no-ops; whatever actually manages a BRANCH
  instance's lifecycle is not in this unit.
- **offset 0x48** (slot 18, where `virtual ~fBase_c()` first enters the vtable chain --
  the SAME slot dummy_door's own destructor override used) is `fn_2_67750`: the same
  one-slot, flag-argument `(this, shouldFree)` destructor ABI already established by
  dummy_door -- `if (this) { dActor_c::~dActor_c(); if (shouldFree > 0)
  fBase_c::__dl(this); } return this;`, read directly off `fn_2_67750`'s own disasm, not
  assumed.

`create()`'s override here is a NEW finding beyond dummy_door's own precedent (dummy_door's
two classes overrode ONLY the destructor slot) -- checked specifically because the extra
`fn_2_67740` entry sat exactly at the `create` slot, then confirmed by reading its own
2-instruction body and cross-referencing `f_base.hpp`'s `PACK_RESULT_e` enum.

`new daBranch_c()`'s own `li r3, 0x398` alloc size matches `sizeof(dActor_c)` exactly --
the SAME constant dummy_door's own classInit used -- so daBranch_c adds no members of its
own.

## Naming

No pooled string names this class (checked: `grep -n BRANCH` on both the target `.text`
and `.data` dumps finds only the `g_profile_BRANCH`/`lbl_2_data_17704` object-name labels
dtk itself assigns, not a game string). `daBranch_c` follows this project's established
`g_profile_<PROFILE>` -> `da<CamelCase>_c` convention (confirmed against `DUMMY_DOOR_CHILD`
-> `daDummyDoorChild_c` etc.); `create__10daBranch_cFv`/`__dt__10daBranch_cFv` (mangled
name lengths) match this guess and the vtable slot addresses resolved cleanly against it,
which is the actual evidence -- the class name itself is otherwise unconfirmable from data
alone, same caveat dummy_door's own naming carried.

## No shadow header needed

`dActor_c` (`include/game/bases/d_actor.hpp`) is a real, already-landed header covering
everything this class needs. No `shadow_include/` directory exists for this unit.

## Status: READY FOR THE 5-BINARY VERIFY

3/3, order gate green, `.ctors` gate green (by confirmed absence), vtable independently
CLEAN. Nothing parked, nothing deferred.
