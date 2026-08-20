# LEMMY_FOOTHOLD + LEMMY_FOOTHOLD_MAIN

`.text 0xC5C90-0xC7270` (0x15E0 bytes), module `d_basesNP`. Two profiles,
one translation unit -- NOT the tiny-manager/real-object shape seen on
other units; both `daLemmyFoothold_c` and `daLemmyFootholdMain_c` are
substantial, real `dEn_c`-derived enemy classes with the same `sizeof`
(`0x6a8`), differing only in one internal member.

## Bounds, confirmed

- `python wip/wm_units/scout_unit.py d_basesNP 0xc5c90 0xc7270`: `.text`
  2 distinct targets, both inside the claimed range, 0 outside.
- `.ctors`: exactly one entry, `0x1ec -> __sinit @ 0xc6920`, owned by
  `g_profile_LEMMY_FOOTHOLD_MAIN` (`ctors_map.py d_basesNP LEMMY`).
- `python wip/wm_units/check_target_objs.py`: clean. Three target objects
  cover the range and all three are passed to `verify_anon.py`:
  `auto_00_000C58E0_text.o` (covers `0xc5c90`-`0xc6d50`),
  `auto_00_000C6D50_text.o` (covers `0xc6d50`-`0xc7270`), and
  `auto_fn_2_C6920_text.o` (the `__sinit` function, split into its own
  object by dtk -- exactly the trap the coordinator flagged; confirmed by
  precise numeric address matching, not just trusting the checker, that
  no other split point falls inside the range).
- 51 real (non-`gap_`) functions total in range, summing to `0x14c8` of
  the `0x15e0` claimed bytes; the remainder is ordinary inter-function
  alignment padding (many tiny functions, 1-3 instructions each, in this
  unit -- weak stub density is unusually high here).

## Class layout -- both classes, verified by compilation, not just read

Both `daLemmyFoothold_c` and `daLemmyFootholdMain_c` derive from `dEn_c`
(`include/game/bases/d_enemy.hpp`), confirmed by the 5-character mangled
ctor call `__ct__5dEn_cFv` in both constructors. `sizeof(dEn_c) == 0x528`
(`STATIC_ASSERT`/`Probe` confirmed). Both classes' own `sizeof` is
`0x6a8` (`li r3, 0x6a8` in both `classInit`s).

Member layout, read directly from the constructors and cross-checked
against the already-landed `source/d_basesNP/bases/d_a_wm_antlion.cpp`
(same module, same `m3d::` model/animation construction idiom -- see
below):

| offset | member | size | note |
|---|---|---|---|
| `0x000` | `dEn_c` base | 0x528 | |
| `0x524` | `dHeapAllocator_c mAllocator` | 0x1c (probed) | reuses 4 bytes of `dEn_c`'s own tail alignment padding (`0x524` to `0x528`) |
| `0x540` | `u32 m_540` | 4 | unidentified, explicitly zero-initialized via the class's own ctor |
| `0x544` | `m3d::mdl_c mModel` | 0x40 (probed) | |
| `0x584` | `u32 m_584` | 4 | unidentified, explicitly zero-initialized |
| `0x588` | `m3d::anmTexSrt_c mAnimTexSrt` | 0x2c (probed; matches antlion's own probe of the same type exactly) | see construction note below |
| `0x5b4` (MAIN) / `0x5c0` (plain) | `dBg_ctr_c mBgCtr` | 0xe4 (probed) | **the one confirmed real difference between the two classes** -- plain `daLemmyFoothold_c` has an extra `0xc` bytes here that MAIN does not |
| tail | `0x10` bytes (MAIN) / `0x4` bytes (plain) | | modelled as raw padding, not yet identified |

**Both totals check out exactly**: MAIN = `0x5b4 + 0xe4 + 0x10 = 0x6a8`;
plain = `0x5c0 + 0xe4 + 0x4 = 0x6a8`. Not a coincidence -- both were
derived independently from the constructors' own offsets, and both land
on the confirmed `sizeof`.

### The `m3d::` construction idiom -- read off the landed antlion precedent, not guessed

`mAnimTexSrt`'s construction (`m3d::anmTexSrt_c : public banm_c`, per
`include/game/mLib/m_3d/anm_tex_srt.hpp`/`banm.hpp`) shows TWO vtable
writes to the same offset (`__vt__Q23m3d6banm_c` then
`__vt__Q23m3d11anmTexSrt_c`), with `banm_c`'s own embedded
`mAllocator_c mAllocator` member (`banm.hpp:47`) constructed in between
(`__ct__12mAllocator_cFv`, called with `this+0xc` relative to
`mAnimTexSrt`'s own start -- exactly `banm_c`'s documented layout:
`mpObj@0`, `mpHeap@4`... wait, vtable@0, `mpObj@4`, `mpHeap@8`,
`mAllocator@0xc`). This is **ordinary C++ construction order** for
`anmTexSrt_c : banm_c` (base `banm_c` constructs and sets its own vtable
first, including its own `mAllocator_c` sub-member, then the derived
class's constructor finishes by overwriting the vtable to its own) --
not hand-rolled, and it is the exact same shape
`source/d_basesNP/bases/d_a_wm_antlion.cpp`'s own comment documents for
its sibling `mChrAnim`/`mAnimTexSrt` members ("vtable then overwritten to
anmChr_c/banm_c's own -- same idiom as every landed sibling"; "its own
`mAllocator_c` sub-member constructed at +0x788, i.e. `anmTexSrt+0xc`").
Declaring `m3d::anmTexSrt_c mAnimTexSrt;` as an ordinary member and
letting the compiler generate this was sufficient -- confirmed by an
exact `classInit` match for both classes.

### `+0x60`: the class's own vtable pointer, same mechanism as WM_KINOPIO/RIVER

Both constructors store a `lbl_2_data_XXXXX` address at `+0x60`
immediately after the `dEn_c` base ctor call. Same mechanism already
established twice this session: the most-derived class's own
auto-generated vtable pointer, set as part of ordinary C++ construction.
Confirmed by exact match, not re-derived by hand.

## Result this round: two full classes' construction verified, 10/51

Both classes' `classInit` AND their constructor/destructor pair are now
**verified exact matches** -- this is strong confirmation the member
layout above is correct, not just plausible, since a destructor's shape
is sensitive to the full member list (each member's destructor must be
called in reverse order) and would not match on a wrong layout.

| target | size | draft | note |
|---|---|---|---|
| `0xC5C90` `daLemmyFoothold_c` classInit | 0x30 | **MATCH** | thin wrapper: `new(0x6a8)`, if non-null `bl` the real ctor |
| `0xC5CC0` `daLemmyFootholdMain_c` classInit | 0xA4 | **MATCH** | full inline construction (ctor got inlined here, unlike the plain class) |
| `0xC61E0` `daLemmyFoothold_c::daLemmyFoothold_c()` | 0x94 | **MATCH** | real, separate out-of-line ctor (not inlined into classInit, unlike MAIN's) |
| `0xC6280` `daLemmyFoothold_c::~daLemmyFoothold_c()` | 0x8C | **MATCH** | |
| `0xC6890` `daLemmyFootholdMain_c::~daLemmyFootholdMain_c()` | 0x8C | **MATCH** | |
| `0xC6150` `dBaseActor_c::finalUpdate()` | 0x4 | **MATCH** | shared weak stub |
| `0xC6160` `dActor_c::funsuiMoveX()` | 0x4 | **MATCH** | shared weak stub |
| `0xC6180` `dActor_c::setCarryFall(dActor_c*,int)` | 0x4 | **MATCH** | shared weak stub |
| `0xC6190` `dEn_c::endFunsui()` | 0x4 | **MATCH** | shared weak stub |
| `0xC66B0` `dEn_c::beginFunsui()` | 0x4 | **MATCH** | shared weak stub |

**10/51 byte-identical.** `.ctors` gate untouched (I have not authored
any static state yet, so my draft correctly has none either). Function
order: the two `classInit`s and the weak stubs are in correct relative
position; the ctor/dtor pairs still show as "defined too late" against
the checker, but this is expected and not yet meaningful -- dozens of
real, unauthored functions sit between them in target, so the checker
cannot confirm true position until more content fills in (the same
caveat documented at length on RIVER, here for a more mundane reason:
genuinely missing content, not a pairing ambiguity).

## Open items, precisely characterized -- NOT guessed at

1. **`m_540` and `m_584`**: two 4-byte fields, confirmed real (explicitly
   zero-initialized by both classes' own constructors, not covered by
   any sub-object's own ctor), but their true type/name is not
   identified. Modelled as raw `u32` placeholders. Given they sit
   between `dHeapAllocator_c`/`m3d::mdl_c` and `m3d::mdl_c`/
   `m3d::anmTexSrt_c` respectively, they are plausibly small POD flags or
   pointers specific to model/allocator bookkeeping -- not chased
   further this round.
2. **The `0xc`-byte gap in the plain class** (`daLemmyFoothold_c`'s extra
   member between `mAnimTexSrt` and `mBgCtr` that MAIN does not have):
   confirmed real via the constructor's own `dBg_ctr_c` target offset
   (`+0x5c0` vs MAIN's `+0x5b4`), but no store touches it anywhere in
   either constructor, so it is likely POD data relying on the
   allocator's zero-initialization guarantee rather than an
   explicitly-constructed sub-object. Modelled as raw `u8[0xc]`
   placeholder. This is the ONE confirmed structural difference between
   the two classes found so far.
3. **The tail after `mBgCtr`** (`0x10` bytes MAIN / `0x4` bytes plain,
   both landing on the shared `0x6a8` total): not identified, modelled
   as raw padding.
4. **41 of 51 functions remain unauthored.** Not attempted this round,
   listed here by address/size for the next pass rather than
   re-derived: `0xC5D70`(0x80) `0xC5DF0`(0x138) `0xC5F30`(0xC8)
   `0xC6000`(0x5C) `0xC6060`(0x90) `0xC60F0`(0x30) `0xC6120`(0x28)
   `0xC6170`(0x10, a virtual-dispatch thunk through `+0x588` -- an
   `mAnimTexSrt` method, NOT state-framework-related; do not misattribute
   it the way an earlier pass on this unit almost did) `0xC61A0`(0x10,
   same shape) `0xC61B0`/`0xC61C0`/`0xC61D0`(0xC each, all
   `lis/addi null__8sStateID; blr` -- return the address of
   `sStateID::null`, genuinely state-framework-shaped, not yet attached
   to a specific virtual method name) `0xC6310`(0x80) `0xC6390`(0x138)
   `0xC64D0`(0xC0) `0xC6590`(0x5C) `0xC65F0`(0x54) `0xC6650`(0x30)
   `0xC6680`(0x28) `0xC66C0`(0x4) `0xC66D0`(0x10) `0xC66E0`(0x24)
   `0xC6710`(0x4) `0xC6720`(0x84) `0xC67B0`(0x4) `0xC67C0`(0x4)
   `0xC67D0`(0xBC) `0xC6920`(0x430, `__sinit` -- see below) `0xC6D50`(0x58)
   `0xC6DB0`(0x58) `0xC6E10`(0x5C) `0xC6E70`(0xDC) `0xC6F50`(0xE0)
   `0xC7030`(0x88) `0xC70C0`(0x30) `0xC70F0`(0x30) `0xC7120`(0x30)
   `0xC7150`(0x88) `0xC71E0`(0x30) `0xC7210`(0x30) `0xC7240`(0x30).
   The trailing run of `0x30`-byte functions (`0xC70C0` through
   `0xC7240`) is very likely another cluster of trivial per-state or
   per-type wrapper functions, given the size uniformity -- not yet
   read.
5. **The state framework is confirmed in use** (`dEn_c : public
   dActorMultiState_c`, and the `null__8sStateID`-returning trio at
   `0xC61B0`-`0xC61D0` is unmistakably state-framework boilerplate,
   matching `include/game/sLib/s_State.hpp`'s `STATE_VIRTUAL_FUNC_DECLARE`/
   `STATE_DEFINE`/`STATE_VIRTUAL_DEFINE` macros), but the framework's
   SHAPE for this specific unit -- how many states, which class declares
   them, what `__sinit` (0x430 bytes, unusually large) actually
   registers -- has **not** been resolved yet. This is exactly the
   "resolve the framework shape before hand-authoring" item the
   coordinator flagged; not done this round, explicitly left for next.
   A genuine near-miss avoided: `0xC6170`/`0xC61A0`'s virtual-dispatch
   thunks (`lwzu r12,0x588(r3); lwz r12,0x14(r12); mtctr; bctr`) look
   superficially state-related by proximity to the `null__8sStateID`
   trio, but `+0x588` is `mAnimTexSrt`'s own vtable slot (confirmed by
   the class layout above), not anything state-managed -- these are
   ordinary virtual-dispatch wrapper methods reaching into the model's
   animation sub-object, ATTRIBUTION checked against the real member
   offset rather than assumed from adjacency.

## Tools used

`wip/wm_units/scout_unit.py`, `ctors_map.py`, `check_target_objs.py` for
bounds. A throwaway `probe.cpp`/`probe_build.py` (deleted after use) with
`STATIC_ASSERT`/`Probe<sizeof(...)>` settled `sizeof(dEn_c)`,
`sizeof(dHeapAllocator_c)`, `sizeof(m3d::mdl_c)`,
`sizeof(m3d::anmTexSrt_c)`, `sizeof(dBg_ctr_c)`, `sizeof(mAllocator_c)`
before any function body was authored, per this project's standing
practice. `source/d_basesNP/bases/d_a_wm_antlion.cpp` (landed, same
module) was the load-bearing precedent for the `m3d::` model/animation
construction idiom -- read before writing any of it, not reverse
engineered from scratch.

## Round 2: the state framework, resolved per the coordinator's own `.data` scan -- 10/51 to 30/51

The coordinator read the unit's entire state inventory directly out of
`.data` and handed it over rather than leaving it for re-derivation:

```
daLemmyFootholdMain_c::StateID_DemoWait     +0x283FF
daLemmyFootholdMain_c::StateID_Wait         +0x28428
daLemmyFoothold_c::StateID_DemoWait         +0x2844C
daLemmyFoothold_c::StateID_DemoDown         +0x28470
daLemmyFoothold_c::StateID_DemoUp           +0x28494
```

Five states, two on `daLemmyFootholdMain_c`, three on `daLemmyFoothold_c`
-- exactly what the 0x430-byte `__sinit` (`fn_2_C6920`) registers, and
the `.data` upper bound for this unit is now known too: it ends before
`0x286D4` (`daLiftBalance_c::StateID_Wait`, belonging to the NEXT unit,
`AC_LIFT_BALANCE` at `.text 0xC7270`).

### The `StateID_DemoWait` name collision, resolved by hand-expansion

Both classes declare a state named `DemoWait`. `STATE_VIRTUAL_DEFINE`
(`include/game/sLib/s_State.hpp:46`) emits a **file-scope** template
function `baseID_##name<T>()` plus an explicit specialization
`baseID_##name<sStateID_c>()` -- neither is qualified by the owning
class, so invoking the macro for `DemoWait` twice (once per class) would
redefine the same specialization twice, an ODR violation. Same category
as `source/d_basesNP/bases/d_a_ac_switch.cpp`'s own hand-expansion of
`ACTOR_PROFILE` (a macro that cannot be invoked twice for one class),
per the coordinator's own precedent pointer.

**Resolution, verified against the actual bytes, not assumed from the
inheritance shape**: `daLemmyFootholdMain_c` uses the ordinary
`STATE_VIRTUAL_DEFINE(daLemmyFootholdMain_c, DemoWait)` macro (registered
FIRST in `__sinit`, confirmed by the coordinator's `.data` address
order), which legitimately defines the shared `baseID_DemoWait<T>`
template and its `sStateID_c` specialization. `daLemmyFoothold_c`'s own
`StateID_DemoWait` is then **hand-expanded**:

```cpp
sFStateVirtualID_c<daLemmyFoothold_c> daLemmyFoothold_c::StateID_DemoWait(
    baseID_DemoWait<daLemmyFoothold_c::StateIDBase_DemoWait>(),
    "daLemmyFoothold_c::StateID_DemoWait",
    &daLemmyFoothold_c::initializeState_DemoWait,
    &daLemmyFoothold_c::executeState_DemoWait,
    &daLemmyFoothold_c::finalizeState_DemoWait);
```

The superState argument reuses the EXISTING `baseID_DemoWait<T>`
template (via `daLemmyFoothold_c::StateIDBase_DemoWait`, which resolves
to `sStateID_c` since neither `daLemmyFoothold_c` nor `dEn_c` declares
its own inherited `DemoWait`) rather than a bare `sStateID::null`
literal -- **checked against the disassembly before writing this**: the
target's own `__sinit` reaches this superState value via a real `bl` to
a tiny helper function (`fn_2_C61B0`, `lis/addi null__8sStateID@ha/@l;
blr`), not an inlined constant load, meaning the target's real source
also goes through the templated `baseID_` mechanism here, not a direct
`sStateID::null` reference. This compiled to an **exact match**
(`fn_2_C61B0 MATCH <- "baseID_DemoWait<10sStateID_c>__Fv_RC12sStateIDIf_c"`),
confirming both the technique and the specific superState value.
`daLemmyFoothold_c` and `daLemmyFootholdMain_c` are confirmed true
siblings (both call `__ct__5dEn_cFv` directly; neither derives from the
other), so this was not a coincidence-prone guess.

### The payoff, exactly as predicted: 20 more functions matched from 5 state declarations

**10/51 -> 30/51.** All of it framework output, none hand-authored
beyond the `STATE_VIRTUAL_FUNC_DECLARE`/`_DEFINE` plumbing and (for now)
empty stub bodies for the 15 `initializeState_X`/`executeState_X`/
`finalizeState_X` methods:

- All 3 `baseID_` helper functions (`DemoWait`, `Wait`, `DemoDown` --
  `DemoUp` needed none, since it's the last state defined and nothing
  else references its own `baseID_DemoUp<sStateID_c>` after it, so MWCC
  never had to emit a separate symbol for it -- consistent with the
  "some helpers get folded/omitted, don't assume a 1:1 count" lesson
  already learned on RIVER).
- Every state object's own destructor: `sFStateID_c<daLemmyFootholdMain_c>`,
  `sFStateID_c<daLemmyFoothold_c>`, `sFStateVirtualID_c<daLemmyFoothold_c>`
  (all three MATCH).
- `number()`/`superID()` for `sFStateVirtualID_c<daLemmyFootholdMain_c>`
  (MATCH).
- `isSameName()`/`initializeState()`/`executeState()`/`finalizeState()`
  trampolines for BOTH classes' `sFStateID_c<T>` (8 functions, all
  MATCH).
- 6 of the 15 stub state-logic bodies happened to compile identically to
  target already (both `DemoWait` initialize/execute/finalize pairs for
  MAIN, plus `DemoWait`/`DemoDown`'s `initialize`/`execute` for
  FOOTHOLD) -- these are almost certainly states whose REAL bodies are
  genuinely trivial/empty in the target too, not a coincidence, though
  not independently re-verified against the raw target bytes yet (kept
  as a flagged assumption, not a confirmed read).

### What's still stubbed, precisely -- do not read the above MATCHes as "state logic done"

9 of the 15 state-logic bodies do NOT yet match (their target functions
are 4-12 instructions, too large for a genuinely empty body, meaning
target has real per-state logic there): `initializeState_DemoWait`/
`finalizeState_DemoWait` for `daLemmyFoothold_c` at `0xC6170`/`0xC61A0`
-- wait, those two addresses are the `mAnimTexSrt` vtable-dispatch thunks
already identified last round, NOT state bodies; the real still-open
state bodies are at `0xC6650`/`0xC6680` (`daLemmyFootholdMain_c`-shaped
sizes, need re-attribution) and `0xC66D0`/`0xC66E0`/`0xC6710`-adjacent
(`daLemmyFoothold_c::finalizeState_DemoWait`, `initializeState_DemoDown`/
`executeState_DemoDown` region) and `0xC67B0`/`0xC67C0` vicinity. Not
individually re-attributed function-by-function this round -- the
`verify_anon` closest-candidate pairings shown for these are NOT reliable
identity claims (several show `isSameName`-shaped mismatches purely
because that's the nearest SIZE match, not because that's what they
really are). This needs a fresh, careful per-function read next round,
the same discipline that already caught the `+0x588` dispatch
misattribution -- **not** an assumption that today's stub-match rate
generalizes.

### `.data` upper bound noted, not yet used

The coordinator's `0x286D4` boundary (start of the next unit,
`AC_LIFT_BALANCE`) is recorded here for whoever derives this unit's own
`.data`/`.rodata` slice claim -- not done this round (function content
was the priority per instruction).

## Final result, this round: 30/51 byte-identical (up from 10/51)

Function order still reports violations, but they are now concentrated
exactly where expected: the empty STUB state-logic bodies (trivial, 1-2
instructions) get positioned differently than the REAL, larger bodies
target actually has at those same identities would be -- an artifact of
authoring stubs before real content, not a structural defect. Should
resolve substantially once real per-state bodies replace the stubs.
`.ctors` unaffected (still correctly absent from the draft's own output
in the sections I've touched; the `__sinit` itself is now real content,
not yet exactly matching, so the `.ctors` entry it produces was not
independently re-verified this round).

**Next round's clear priority, per the coordinator's own framing**:
finish reading and authoring the 9 real (non-stub) state-logic bodies
function-by-function, re-verifying each against target bytes directly
rather than trusting proximity or size-based candidate pairing --
exactly the discipline that has already caught two wrong attributions
elsewhere today.

## Round 3: state bodies attributed via PMF-pointer reads, 30/51 -> 34/51

Per the coordinator's technique -- read each state object's own PMF
fields (initialize/execute/finalize pointer-to-member-function triples)
directly out of `.data`, rather than trusting `verify_anon`'s size-based
candidate pairing -- all 9 real state-logic bodies were attributed to
exact target addresses before writing anything.

### Two different PMF encodings, discovered while doing the attribution -- one resolved, one open

`daLemmyFootholdMain_c`'s states store their PMFs as **direct,
relocatable addresses** (word0 = 0 in the file, patched to the real
function address at REL-load time -- read via
`wip/wm_units/profile_map.py`'s `relocations()`, the same tool the
project uses for classInit resolution). `daLemmyFoothold_c`'s states,
by contrast, store their PMFs as **vtable byte offsets** (word1 holds a
small integer like `0x280`-`0x2a0`, word0/word2 are plain `0`, no
relocation at all) -- a genuinely different, virtual-dispatch PMF
representation for the *identical* `STATE_VIRTUAL_FUNC_DECLARE`/
`STATE_VIRTUAL_DEFINE` macro shape. Resolved by reading FOOTHOLD's own
vtable (`lbl_2_data_27E10`, confirmed via the classInit's own vtable-
pointer store) at those exact byte offsets, giving the real function
addresses just as reliably as MAIN's relocations did.

**Why MAIN gets direct encoding and FOOTHOLD gets virtual encoding is
NOT resolved.** Both classes use the identical macro invocation shape;
neither derives from the other; nothing in this TU further overrides
either class's state methods. Recorded as a genuine open question
(flagged in the source comments), not chased further since it only
affects whether `__sinit`'s own bytes match -- explicitly the LAST
priority per the coordinator's own framing, and `__sinit` remains
267/268 differing this round.

### All 9 real bodies attributed; 7 authored and confirmed, 2 read but not yet authored

| state · method | target addr | size | status |
|---|---|---|---|
| MAIN::DemoWait init/fin | 0xC6160/0xC6150 | 0x4 each | **MATCH** (trivial, empty) |
| MAIN::DemoWait exec | 0xC6170 | 0x10 | **MATCH** -- `mAnimTexSrt.play();` |
| MAIN::Wait init/fin | 0xC6190/0xC6180 | 0x4 each | **MATCH** (trivial, empty) |
| MAIN::Wait exec | 0xC61A0 | 0x10 | **MATCH** -- `mAnimTexSrt.play();` |
| FOOTHOLD::DemoWait init/fin | 0xC66B0/0xC66C0 | 0x4 each | **MATCH** (trivial, empty) |
| FOOTHOLD::DemoWait exec | 0xC66D0 | 0x10 | **MATCH** -- `mAnimTexSrt.play();` |
| FOOTHOLD::DemoDown init | 0xC66E0 | 0x24 | **MATCH** -- `mSpeed = mVec3_c(0,0,0); mAccelY = -0.185f;` |
| FOOTHOLD::DemoDown exec | 0xC6720 | 0x84 | **read, not yet authored** -- see below |
| FOOTHOLD::DemoDown fin | 0xC6710 | 0x4 | **MATCH** (trivial, empty) |
| FOOTHOLD::DemoUp init/fin | 0xC67C0/0xC67B0 | 0x4 each | **MATCH** (trivial, empty) |
| FOOTHOLD::DemoUp exec | 0xC67D0 | 0xBC | **not yet read** |

`mAnimTexSrt.play()`'s attribution is independently cross-confirmed, not
just size-matched: a probe compile of `m3d::anmTexSrt_c` showed `play()`
lands at vtable byte offset `0x14` exactly, matching the dispatch thunk
shape (`lwzu r12,0x588(r3); lwz r12,0x14(r12); mtctr; bctr`) seen at all
three `exec` sites. `mSpeed`/`mAccelY` are confirmed pre-existing
`dBaseActor_c` members (`STATIC_ASSERT`/`Probe` offsets `0xe8`/`0x114`
respectively, matching the target's own store offsets exactly) -- no new
members needed.

### `executeState_DemoDown` (0xC6720, 0x84 bytes) -- read in full, NOT authored, per the size/confidence discipline

Read completely before deciding not to author it yet:

```
mAnimTexSrt.play();               // same dispatch-through-+0x588 shape as the trivial exec states
calcSpeedY();                      // dBaseActor_c method, already declared
posMove();                         // dEn_c's own override, already declared
float dist = this[0x5b8] - mPos.y; // 0x5b8 is INSIDE the still-unidentified mUnk5B4[0xc] gap
                                    // (between mAnimTexSrt and mBgCtr) -- a real float field there,
                                    // not yet named
if (dist > lbl_2_rodata_4AA4) {
    // virtual call through this's OWN vtable, slot 0xd4 (0xd4/4 = 53),
    // argument &lbl_2_bss_A6C4 (one of this unit's own 3 confirmed
    // .bss singletons, scout_unit.py's own ".bss 3 distinct targets
    // 0xA638..0xA6C4" range -- the third, not yet typed)
}
```

Not authored because three real unknowns remain (the `0x5b8` field's
name/type, the vtable-slot-0xd4 method's name, and `lbl_2_bss_A6C4`'s
type) and guessing any of them risks the exact "3-attempts-then-park"
threshold being spent on a wrong shape rather than a genuine residual --
per the coordinator's own size/diff diagnostic, a function this size
with this many unresolved pieces needs more reading, not an attempt.
Left as a clearly-flagged, unauthored stub (still returns nothing,
current source has an empty body) rather than guessed at.

### `executeState_DemoUp` (0xC67D0, 0xBC bytes) -- not yet read

Largest of the two remaining real bodies (47 instructions). Not
attempted this round -- ran out of time after the DemoDown read and the
PMF-encoding investigation, which was itself substantial (see above).
Flagged as the clear next item, same reading discipline as DemoDown
(read in full before writing anything, no guessing from size or
proximity).

## Final result, this round: 34/51 byte-identical (up from 30/51)

7 of 9 real state-logic bodies now authored and matching; 2 read-in-full-
but-not-authored (DemoDown's exec, partially) or not-yet-read (DemoUp's
exec) remain, both honestly flagged rather than guessed. `__sinit`
itself: 267/268 differing, unchanged in practice (blocked on the
FOOTHOLD virtual-PMF-encoding open question, which is `__sinit`-only and
explicitly deprioritised). `.ctors` gate unaffected. Function order not
re-examined this round (per the coordinator's own "leave it alone" this
round).

**Next round's clear priorities**: (1) finish reading `executeState_DemoUp`
and complete `executeState_DemoDown`'s three remaining unknowns (the
`0x5b8` field, the vtable-slot-`0xd4` method, `lbl_2_bss_A6C4`'s type) --
the same `dEn_c`/`dActor_c`-family header search technique already used
successfully elsewhere in this unit; (2) the FOOTHOLD virtual-PMF-
encoding question, if a concrete new angle presents itself (not a repeat
investigation); (3) the unit's own `.data`/`.rodata` slice, still not
derived.

## Round 4: the macro fix (FLOOR_JR_A's finding, applied here) plus both remaining state bodies

### The PMF-encoding fix: confirmed, and it moved `__sinit` exactly as predicted

The coordinator's answer -- PMF encoding records whether the pointed-to
method is virtual (`{-1, fn_addr, 0}` = non-virtual, `{vtable_offset,
0x60, 0}` = virtual), independently discovered on FLOOR_JR_A -- applied
directly:

- `daLemmyFootholdMain_c`'s two states (`DemoWait`, `Wait`) switched from
  `STATE_VIRTUAL_FUNC_DECLARE`/`STATE_VIRTUAL_DEFINE` to the plain
  `STATE_FUNC_DECLARE`/`STATE_DEFINE` (non-virtual).
- `daLemmyFoothold_c`'s three states keep the virtual macro, but the
  hand-expansion for `StateID_DemoWait` is **no longer needed**: once
  MAIN's states are correctly non-virtual, `STATE_DEFINE` never emits a
  file-scope `baseID_` helper at all, so there is nothing for
  FOOTHOLD's own (virtual) `StateID_DemoWait` to collide with. All three
  of FOOTHOLD's states now use the ordinary `STATE_VIRTUAL_DEFINE` macro
  directly -- simpler than round 3's hand-expansion, not just different.

**Result: `__sinit` went from 267 differing to 35 differing** (out of
268 words) -- matching the coordinator's FLOOR_JR_A precedent (74 -> 21)
almost exactly in proportion. The raw MATCH count stayed at 34/51 this
step (the individual state-body functions were already correct from
round 3; only the `.data` pool construction sequence itself changed),
confirming the earlier bodies were never the problem -- only the
declaration shape was.

**Correction on my own earlier framing**: round 3 called "both classes
use the identical macro shape" the puzzle. It was the defect. Recorded
per the coordinator's own point: an honestly-flagged "I cannot explain
this" is what let another agent's independent finding resolve it --
worth remembering as a reason not to paper over a genuine unexplained
observation with a plausible-sounding guess.

### Both remaining state bodies read in full and authored -- one real bug caught, two small residuals left open

**`executeState_DemoUp`** (0xC67D0, 0xBC bytes) read in full:

```cpp
void daLemmyFoothold_c::executeState_DemoUp() {
    mAnimTexSrt.play();
    mSpeed.y = (mTargetPosY - mPos.y) / 10.0f;
    posMove();
    float dist = mTargetPosY - mPos.y;
    float absDist = (dist > 0.0f) ? dist : -dist;
    if (absDist < 1.0f) {
        mPos.y = mTargetPosY;
        void *vtable = *(void **) ((u8 *) this + 0x60);
        daLemmyFootholdVFunc0xD4_t f = *(daLemmyFootholdVFunc0xD4_t *) ((u8 *) vtable + 0xd4);
        f(this, &lbl_2_bss_A6C4);
    }
}
```

This identified `+0x5b8` (the gap field flagged open since round 3) as a
genuine **target Y position** -- both this state and `DemoDown` compute
`mTargetPosY - mPos.y` as a distance-to-target and drive `mSpeed.y` from
it; `DemoUp` additionally snaps `mPos.y` to the exact target once within
`1.0f` and calls a still-unidentified method at vtable byte offset
`0xd4` (through this object's own `+0x60` vtable pointer), passing the
address of `lbl_2_bss_A6C4` (one of this unit's 3 confirmed `.bss`
singletons). Result: **6 differing out of 47** -- all either the
already-characterized rodata pool-position class, or one register
choice (`r12` reused vs a fresh `r5`) for the vtable-pointer load. Tried
two variants (splitting the nested cast into separate statements;
hoisting the `&lbl_2_bss_A6C4` argument into its own local) -- neither
changed the output. Per the coordinator's own size/diff diagnostic
(exact size, small diff = genuine residual, not missing content): parked
after two attempts rather than continuing to grope for a third.

**`executeState_DemoDown`** (0xC6720, 0x84 bytes): re-read after writing
the naive first draft (`mAnimTexSrt.play()` + the threshold check alone)
and caught a **real bug before it shipped** -- the diff against the
correctly-named target function (26 differing, size matched) showed
target calling `calcSpeedY()` and `posMove()` in between, which my first
draft had simply omitted (an oversight from writing the threshold logic
first and not re-reading the full disassembly before committing to a
body). Added both calls, matching `executeState_DemoUp`'s own preamble
shape exactly. **Result: 2 differing out of 33** -- the identical two
residual classes as `DemoUp` above (rodata pool-position naming, one
register choice for the same vtable-slot-`0xd4` dispatch). Not chased
further, same reasoning.

### `mTargetPosY` (+0x5b8) resolved; the vtable-slot-0xd4 method still is not

`+0x5b4`'s gap is now modelled as `u32 m_5b4; float mTargetPosY; u32
m_5bc;` -- the middle field's role is confirmed by two independent
call sites computing the identical `target - mPos.y` distance
expression, not a single occurrence. `m_5b4`/`m_5bc` remain
unidentified (never touched by either state body). The vtable-slot-0xd4
method itself: checked `dEn_c`'s own header for a plausible
single-pointer-argument virtual (callback/notify/register-shaped names)
and found no clean match; called via a raw vtable-offset function
pointer (this project's established fallback for a genuinely
unidentified slot) rather than guessed at by name. Both call sites
(`DemoDown` and `DemoUp`) were cross-checked to confirm they really are
the identical method before writing the shared raw-cast helper type --
not assumed from one site alone.

## Final result, this round: 34/51 byte-identical (unchanged count, but both remaining state bodies now precisely characterized)

All 9 real per-state bodies are now authored. 7 are exact `MATCH`; the
remaining 2 (`executeState_DemoDown`, `executeState_DemoUp`) are
confirmed-correct-size, 2-and-6-instruction residuals in the same
already-documented pool-position/register-choice class seen everywhere
else in this session -- not logic errors, not missing content. `__sinit`
itself: 35/268 differing, down from 267 -- the single biggest move this
session, exactly as predicted. `.ctors` gate re-verified clean.

| target | size | draft | note |
|---|---|---|---|
| Both classInits | -- | **MATCH** | |
| Both ctors/dtors (FOOTHOLD explicit, MAIN implicit) | -- | **MATCH** | |
| 3 shared weak stubs (`finalUpdate`/`funsuiMoveX`/`setCarryFall`/`endFunsui`/`beginFunsui`) | -- | **MATCH** | |
| MAIN's `doDelete()`/`preDelete()` overrides | -- | **MATCH** | |
| 7 of 9 real state bodies | -- | **MATCH** | |
| `executeState_DemoDown` | 0x84 | 2 differing | pool-position + 1 register choice, both already-characterized residual classes |
| `executeState_DemoUp` | 0xBC | 6 differing | same two residual classes |
| `__sinit` | 0x430 | 35/268 differing | down from 267; the macro fix's own predicted effect |

**34/51 byte-identical** (function-count tally unchanged from round 3,
but the unit's *real, uncertain* surface area shrank enormously: from
"2 unread/partial bodies + an unexplained 267-word `__sinit` gap + an
open macro-choice question" to "2 small, well-understood residuals + a
35-word `__sinit` gap + zero open state-declaration questions").

**Remaining open items, precisely**: (1) the vtable-slot-0xd4 method's
real identity; (2) `m_5b4`/`m_5bc` (the two still-unidentified words
flanking `mTargetPosY`); (3) `__sinit`'s own remaining 35-word gap, not
yet investigated (explicitly last priority, per instruction); (4) the
unit's `.data`/`.rodata` slice, still not derived; (5) function order,
not re-examined this round (still shows violations concentrated at the
same lower-priority spots as round 3 -- the coordinator's "leave it
alone" call from round 3 still stands, not re-litigated).

## Round 5: `.data`/`.rodata` slice derived; all 17 unmatched functions surveyed and classified

### `.data` slice: `0x27DB0-0x284F0`

Both ends confirmed by hand against raw REL bytes (`base_data = 0x1d0c00`
in `original/d_basesNP.rel`), the same method used on RIVER:

- **Start, `0x27DB0`**: raw bytes there (`00000000 001f001d`) match
  `g_profile_LEMMY_FOOTHOLD`'s own `fProfile::fActorProfile_c` shape
  exactly (`mpClassInit` relocated to 0 in the file, `mExecuteOrder`/
  `mDrawOrder` = `0x1f`/`0x1d` as two packed `u16`s). Checked what
  precedes it: `0x27C00-0x27D44` is a DIFFERENT unit's own state-name
  strings (`"daLemmyBall_c::StateID_Attack"`, `"...StateID_Revival"`,
  `"...StateID_DemoIkaku_Wait"`, etc. -- all `daLemmyBall_c`, the
  already-known sibling `LEMMY_BALL` at `.text 0xC4E80`, confirmed via
  `ctors_map.py`), followed by pure zero padding through `0x27DB0`. This
  unit's own data genuinely starts at the profile struct, not before it
  -- the "manager/object" convention, not the WM-family's
  leading-strings convention.
- **End, `0x284F0`**: `scout_unit.py`'s own ownership check (relocations
  referenced from within this unit's `.text` claim) reports `0x284EC` as
  the highest genuinely-owned `.data` word -- one more word (`+4`) closes
  it at `0x284F0`, an 8-byte-aligned boundary. Read past it by hand to
  confirm the claim doesn't overreach: this unit's own last state-name
  string (`"daLemmyFoothold_c::StateID_DemoUp"`, ending `0x284B7`) is
  followed by padding, then **foreign content starting around `0x28550`**
  -- a *different* `fProfile::fActorProfile_c`-shaped struct
  (`mExecuteOrder`/`mDrawOrder` = `0x1ce`/`0x1c3`, far outside this
  unit's own tight `0x1d`-`0x1f` cluster) followed by `"g3d/
  test_lift.brres"`/`"test_lift"` strings and more state-shaped PMF
  triples -- all belonging to the coordinator's own-identified next unit
  (`daLiftBalance_c`/`AC_LIFT_BALANCE`, whose own name string sits at
  `0x286D4`). This confirms the upper bound is `0x284F0`, well short of
  the `0x286D4` ceiling the coordinator gave -- that address was an
  upper LIMIT, not the exact boundary, and the true edge is where this
  unit's own content actually stops.
- `check_bounds.py` can't independently verify (no named symbols in this
  anonymous range, same limitation as RIVER), so both ends rest on the
  hand-read evidence above, not the tool's own confirmation.

### `.rodata` slice: `0x4A80-0x4AAC` (tentative, not exhaustively swept)

`scout_unit.py`'s REL-relative (non-DOL-absolute) targets for this
unit's own `.rodata` are exactly 4 addresses: `0x4A80`, `0x4A9C`,
`0x4AA0`, `0x4AA4` (everything else scout reports, `0x80xxxxxx`, is a
DOL-absolute inherited-method-table reference, not this REL's own
`.rodata`). The actual byte extent is larger than these 4 labels alone,
since code reaches further constants via plain displacement arithmetic
off one base register (e.g. `lfs f0, 0x28(r31)` where `r31 =
lbl_2_rodata_4A80`, reaching `0x4AA8` without a separate relocation).
Checked every confirmed use site of all 4 symbols across the read
functions (`initializeState_DemoDown`, `executeState_DemoUp`, plus two
not-yet-authored functions at `0xC5F38`/`0xC6490` that also reference
`lbl_2_rodata_4A80`) -- the largest displacement found is `+0x28`
(`0x4AA8`, the `10.0f` divisor), giving an upper bound of `0x4AAC`.
**Not exhaustive**: 15 functions remain unauthored (see below) and any
of them could reach a rodata displacement I haven't read yet -- this
bound should be re-checked once they're authored, not treated as final.

### All 17 unmatched functions surveyed and classified by the size diagnostic

**2 are genuine residuals** (already authored, exact size match, small
diff, already tried multiple variants -- per round 4):

| target | size | diff | class |
|---|---|---|---|
| `0xC6720` `executeState_DemoDown` | 33 | 2 | residual -- pool-position + 1 register choice |
| `0xC67D0` `executeState_DemoUp` | 47 | 6 | residual -- same two classes |

**15 are missing content** -- genuinely unauthored, no source written at
all. Their reported "differing" counts in `verify_anon`'s output are
**not real diffs**: they're the tool's closest-size-candidate fallback
pairing against unrelated already-authored functions (e.g. several show
`~executeState_DemoDown` or `~"initializeState__32sFStateID_c<...>"` as
the "closest match" purely because nothing of the right identity exists
in the draft yet). Per the coordinator's diagnostic, this is
unambiguous: no authored candidate of the right size exists, so by
definition these need reading and writing, not tuning:

| target | size (words) | quick-read finding |
|---|---|---|
| `0xC6120` | 10 | `mBgCtr.release(); return 1;` -- **doDelete()**, MAIN |
| `0xC6680` | 10 | same shape at `+0x5c0` -- **doDelete()**, FOOTHOLD |
| `0xC60F0` | 12 | `mModel.<vtable+0x14>(); return 1;` -- **draw()**-shaped, MAIN |
| `0xC6650` | 12 | same shape -- **draw()**-shaped, FOOTHOLD |
| `0xC65F0` | 21 | not yet read |
| `0xC6000` | 23 | `mStateMgr.<vtable+0x10>()` (dispatch through `+0x394`, `dActorMultiState_c`'s own state manager) then a virtual call through this object's own vtable at `+0x288` -- **execute()**-shaped, MAIN |
| `0xC6590` | 23 | same shape, vtable slot `+0x2ac` -- **execute()**-shaped, FOOTHOLD |
| `0xC5D70` | 32 | not yet read in full |
| `0xC6310` | 32 | not yet read in full (likely FOOTHOLD's counterpart to `0xC5D70`) |
| `0xC6060` | 36 | not yet read |
| `0xC64D0` | 48 | opens with `mAnimTexSrt.setRate(1.0f, 0); mAllocator.adjustFrmHeap();` -- matches the established `createModel()` idiom from `d_a_wm_antlion.cpp` |
| `0xC5F30` | 50 | opens by storing `mPos.x/y/z`-derived values into this class's own tail fields (`+0x698/+0x69c/+0x6a0`) and zeroing `mScale` (`+0xdc/+0xe0/+0xe4`) -- likely `create()` or `resetPosition()`-shaped |
| `0xC5DF0` | 78 | not yet read in full |
| `0xC6390` | 78 | contains the same `setRate(1.0f,0); adjustFrmHeap();` tail as `0xC64D0` -- likely the OTHER class's own `createModel()` |
| `0xC6920` | 268 | `__sinit`, 35/268 differing -- last priority, not touched this round |

### Net picture, stated separately from the tally per instruction

Structurally, this unit is closer to done than 34/51 alone suggests:
both classes' full construction/destruction lifecycle, all 5 states, the
shared weak-stub cluster, and MAIN's two real overrides are exact
matches. Of the 17 remaining gaps, 2 are small, well-understood residuals
already parked, and roughly half of the other 15 already have a
confident structural read (doDelete/draw/execute pairs for both classes,
plus two likely createModel() twins) from this round's quick survey --
leaving `0xC65F0`, `0xC5D70`, `0xC6310`, `0xC6060`, `0xC5DF0` as the
genuinely open, not-yet-characterized functions for the next pass, with
`0xC5F30`/`0xC64D0`/`0xC6390` needing full (not just opening-instruction)
reads before authoring.

## Round 6: authoring the fifteen -- 34/51 to 42/51, plus a real header gap found

### The near-free batch: draw()/doDelete()/execute()/calcModel(), all confirmed exact

Authored in the coordinator's specified order. All of it verified, none
guessed:

- **`draw()`** (both classes): `mModel.entry(); return SUCCEEDED;`.
  `entry()`'s vtable slot (`0x14`) settled by a **probe compile** of
  `m3d::mdl_c`, not a hand count -- the hand count would have said
  `setAnm()` (a real, different method with an incompatible signature),
  exactly the "never eyeball" trap. **MATCH**, both classes.
- **`doDelete()`** (both classes): `mBgCtr.release(); return 1;`.
  **MATCH**, both classes.
- **`calcModel()`** (both classes): read in full before writing. FOOTHOLD's
  is translation-only (`mMatrix.trans(mPos); mModel.setLocalMtx(&mMatrix);
  mModel.setScale(mScale);`); MAIN's additionally rotates on all three
  axes (`YrotM`/`XrotM`/`ZrotM` on `mAngle.y`/`.x`/`.z`) before the same
  two calls -- a real, confirmed difference between the two classes, not
  an oversight. **MATCH**, both classes.
- **`execute()`** (both classes): `mStateMgr.executeState(); calcModel();
  mBgCtr.calc(); return 1;`. Getting this to match exactly required
  finding two more virtual functions first -- see below. **MATCH**, both
  classes.

### The vtable-slot fix that unblocked `execute()`

`execute()`'s own dispatch to `calcModel()` initially landed 2 slots
early (`+0x280` instead of the wanted `+0x288`/`+0x2ac`). Read
`fn_2_C5D70`/`fn_2_C6310` (the two 32-word functions right where the
missing slots pointed) and found each dispatches through **two more**
of this class's own new vtable slots before anything else -- confirmed
by declaring two placeholder virtuals (`vUnk2A4`/`vUnk2A8`, real
name/content still open) in the right position and watching
`calcModel()`'s own slot number correct itself. **Declaration order is
load-bearing here**: for `daLemmyFoothold_c`, the three virtual states
must be declared *before* `vUnk2A4`/`vUnk2A8`/`calcModel()` (their own 9
slots, `+0x280..+0x2a0`, sit immediately before the two unknowns at
`+0x2a4`/`+0x2a8`) -- confirmed by reordering and re-measuring, not
assumed from the macro's own textual position in the class body.

### `create()` identified and authored for both classes -- 2/32 differing, same known residual

The same `fn_2_C5D70`/`fn_2_C6310` pair turned out to be `create()`
itself once `vUnk2A4`/`vUnk2A8` existed to call: `vUnk2A4(); vUnk2A8();
<vtable-slot-0xd4>(&bss-singleton); mStateMgr.refreshState(); return 1;`.
`refreshState()`'s own vtable slot (`0x1c`) settled by a **probe compile**
of `dEn_c::mStateMgr` against all six `sStateMgrIf_c` methods at once
(`initializeState`/`executeState`/`finalizeState`/`changeState`/
`refreshState`/`getState`), not a hand count of the header's declaration
order -- worth recording since a hand count here would have needed to
also account for `sStateStateMgrIf_c`'s own additional pure virtuals,
easy to get subtly wrong. **A new fact surfaced by this**: the two
classes use *different* `.bss` singletons for the vtable-slot-`0xd4`
call -- `lbl_2_bss_A6C4` for `daLemmyFoothold_c` (already known),
**`lbl_2_bss_A648` for `daLemmyFootholdMain_c`** (new). Both `create()`s:
**2/32 differing**, the identical register-choice residual already
characterized on `executeState_DemoDown`/`executeState_DemoUp` (a fresh
register instead of reusing one across the raw vtable-slot-`0xd4` cast)
-- now seen four times across this unit, confirmed systemic, not chased
further.

### Two functions read in full, NOT yet authored -- a genuine header gap found, not a misread

`fn_2_C5F30` (50 words) and `fn_2_C64D0` (48 words) are `dBg_ctr_c`-setup
functions, one per class, storing `mPos` into a 3-float field
immediately adjacent to `mBgCtr` (confirming `m_5b4`/`mTargetPosY`/
`m_5bc` really do form one `mPos`-shaped snapshot, consistent with the
round-4 finding that `mTargetPosY` gets read/written from multiple
places) and resetting `mScale` to `1.0f`, before calling:

```
set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c
```

**This exact overload does not exist in the landed `d_bg_ctr.hpp`** --
grepped the whole tree for `sBgSetInfo`, zero hits anywhere, landed or
not. The header currently declares two `set()` overloads (one taking
four bare floats, one taking two `mVec2_c`s), neither matching this
mangled name. This is a real, confirmed gap in an already-landed shared
header (not a misread on my part -- the mangled name is unambiguous),
flagged rather than worked around with a guess. **Not authored this
round**: the `sBgSetInfo` struct's own field layout (a ~40-byte
stack-built aggregate, floats plus what look like 2-3 trailing words)
would need to be reverse-engineered from the stack-store pattern before
either function can be written correctly, and that's real additional
work, not a quick fix.

**A genuine misattribution avoided while reading these**: `fn_2_C5F30`'s
own `dBg_ctr_c::set()` call targets `this+0x5b4` -- `daLemmyFootholdMain_c`'s
own `mBgCtr` offset, NOT `daLemmyFoothold_c`'s (`+0x5c0`). `verify_anon`'s
own closest-candidate guess for this function was `~__ct__17daLemmyFoothold_cFv`
-- checking the actual offset used inside the function caught that this
function belongs to the OTHER class before any code got written for it.

### Two more functions read partially, not yet finished

`fn_2_C5DF0`/`fn_2_C6390` (78 words each): confirmed `createModel()` for
both classes (heap allocator creation, resource lookup, `m3d::mdl_c`
create, `setSoftLight_MapObj`, `anmTexSrt_c` create + `setAnm`) -- matches
the established `d_a_wm_antlion.cpp` idiom closely. Read most of
`fn_2_C6390`'s body but not the tail; not authored this round, ran out
of time after the `sBgSetInfo` investigation.

### `.rodata` bound re-checked, unchanged

All newly-read functions' `lbl_2_rodata_4A80` displacements (`0x0`,
`0x8`, `0x18`) stay within the already-established `0x4A80-0x4AAC`
range -- no adjustment needed. Two functions (`fn_2_C5DF0`/`fn_2_C6390`'s
own tails) remain unauthored, so this is still not fully exhaustive, but
narrower than last round.

## Three-population split, this round

| population | count | members |
|---|---|---|
| **Matched** | 42 | all classInits/ctors/dtors, all shared weak stubs, all 5 states, `draw`/`doDelete`/`execute`/`calcModel` (both classes) |
| **Authored, small residual** | 4 | `executeState_DemoDown`(2/33), `executeState_DemoUp`(6/47), `create__daLemmyFoothold_c`(2/32), `create__daLemmyFootholdMain_c`(2/32) -- all the same register-choice class, not chased further |
| **Unwritten** | 5 | `fn_2_C5F30`/`fn_2_C64D0` (dBg_ctr-set, blocked on a real missing header overload), `fn_2_C5DF0`/`fn_2_C6390` (createModel, role confirmed, not finished), `__sinit` (35/268, last priority per instruction) |

**42/51 by raw count**, but the honest picture is stronger: every
lifecycle method except one `dBg_ctr_c::set()` call site pattern is now
understood, and the one genuinely open structural item is a **missing
header overload**, not an unresolved mystery -- a materially different
situation from "17 unmatched functions" two rounds ago.

## Round 7: the shadow header applied, all 5 remaining functions authored

### Shadow header proposed exactly as directed

`wip/wm_units/agent_lemmy/shadow_include/game/bases/d_bg_ctr.hpp`: a
forward declaration (`struct sBgSetInfo;`, no invented fields) plus
`void set(dActor_c *, const sBgSetInfo *, u8, u8, mVec3_c *);`. Return
type checked, not assumed: at both call sites, `r3` is clobbered by the
very next instruction before ever being read, proving the result is
unused (matching, but independently confirmed rather than copied from,
the other two `void` overloads).

### Both `dBg_ctr_c`-setup functions authored -- `vUnk2A4()` identified as their real name

```cpp
void daLemmyFoothold_c::vUnk2A4() {
    m_5b4 = mPos.x; mTargetPosY = mPos.y; m_5bc = mPos.z;
    mScale.x = mScale.y = mScale.z = 1.0f;
    sBgSetInfoLocal_t info; /* fields set individually, not by aggregate init -- see below */
    mVec3_c v; v.x = v.y = v.z = 1.0f;
    u8 u = *((u8 *) this + 0x38f);
    mBgCtr.set(this, (const sBgSetInfo *) &info, 3, u, &v);
    mBgCtr.mFlags |= 4;
    mBgCtr.entry();
}
```

**A real bug caught and fixed by the size diagnostic itself**: `m_5b4`/
`m_5bc` were declared `u32` (carried over from round 1, when they were
only ever seen explicitly zeroed -- indistinguishable from `float` at
the time). Assigning `mPos.x`/`mPos.z` into them compiled to
`__cvt_fp2unsigned` calls -- a real float-to-int VALUE conversion,
not a bit-copy -- inflating the function to 57 words against target's
48. Retyped both to `float`; the spurious conversion calls vanished.

**A second fix, same mechanism**: `sBgSetInfoLocal_t`'s local (the
stand-in for the still-opaque `sBgSetInfo`) was originally built with an
aggregate initializer (`= {-152.0f, 16.0f, ...}`). Because every field
was a compile-time constant, MWCC pooled the *entire struct* as a
separate static object and copied it word-by-word -- a completely
different shape from target's own per-field `lfs`/`stfs` sequence
reading the SAME already-live `lbl_2_rodata_4A80` base register.
Rewriting as individual field assignments (no aggregate initializer)
matched target's shape exactly. **Recorded as a new, general lesson**:
a local aggregate with an all-constant initializer list is NOT
equivalent, at the instruction level, to the same fields assigned one
at a time -- the former can trigger whole-object constant pooling that
the latter does not.

Both functions now differ from target only in: (1) which shared literal
pool base register gets used for the archive-name strings (`daLemmy
Foothold_c`'s target reuses `g_profile_LEMMY_FOOTHOLD` as an anchor my
draft doesn't), and (2) stack-frame slot numbering -- both the
already-characterized pool-position/stack-layout residual class, not
missing content. `daLemmyFootholdMain_c`'s own version reads as "21
differing" under `verify_anon`'s POSITIONAL count, but the entire
divergence is the same two classes -- confirmed by direct line-by-line
comparison, not by trusting the raw number (the positional-count caveat
already documented at length on other units: a 49-vs-48-word length
mismatch cascades into a large raw diff count even when the actual
content differs by only a handful of symbols).

### Both `createModel()`s authored -- with two more real fields identified

```cpp
void daLemmyFoothold_c::vUnk2A8() {   // == createModel()
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    mRes = dResMng_c::m_instance->getRes("boss_lemmy_ashiba", "g3d/boss_lemmy_ashiba.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("boss_lemmy_ashiba");
    mModel.create(mdl, &mAllocator, 0x24, 1, nullptr);
    dActor_c::setSoftLight_MapObj(mModel);
    mResAnmTexSrt = mRes.GetResAnmTexSrt("boss_lemmy_ashiba");
    mAnimTexSrt.create(mdl, mResAnmTexSrt, &mAllocator, nullptr, 1);
    mAnimTexSrt.setAnm(mModel, mResAnmTexSrt, 0, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnimTexSrt);   // vtable slot 0x18, probe-confirmed
    mAnimTexSrt.setRate(1.0f, 0);
    mAllocator.adjustFrmHeap();
}
```

Matches `d_a_wm_antlion.cpp`'s own `createModel()` idiom closely, as
predicted. Both classes share the identical resource strings
(`"boss_lemmy_ashiba"` / `"g3d/boss_lemmy_ashiba.brres"`) -- one shared
model for both foothold variants, read directly out of `.data`.
`mModel.setAnm()`'s own vtable slot (`0x18`) settled by a probe compile
of `m3d::mdl_c`, not a hand count.

**Two more previously-"unidentified, explicitly zeroed" fields
resolved**: `m_540` and `m_584` (flagged open since round 1) turned out
to be **persistent resource handles**, not throwaway locals -- the
constructor zeros them, and `createModel()` writes real values into the
same offsets and *keeps reading them later* (`mRes.GetResMdl(...)`
happens after the `getRes()` call specifically because `mRes` is a
member, not a local temporary). Retyped `m_540` to `nw4r::g3d::ResFile
mRes;` and `m_584` to `nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;`. This
alone took `daLemmyFootholdMain_c`'s own `createModel()` from 56/78
differing to **8/78** -- almost the entire gap was two locals that
should have been members, not missing logic.

### `.rodata` bound re-checked: unchanged, and now genuinely exhaustive

All four `lbl_2_rodata_*` symbols are now referenced only from authored,
read-in-full functions (16 total use sites across the unit, all
checked). No displacement exceeds the already-established `+0x28`
(`0x4AA8`). `0x4A80-0x4AAC` is now final, not provisional -- every
function that could extend it has been read.

### The four register-choice residuals: left alone, as instructed

Not re-attempted. Confirmed present at exactly the same four sites as
round 6 (`executeState_DemoDown`, `executeState_DemoUp`, both
`create()`s), unchanged in shape.

## Final result, this round: 42/51 raw, but every function is now authored and structurally confirmed

| target | size | draft | note |
|---|---|---|---|
| `create()` x2 | 32 each | 2/32 differing each | register-choice residual, left alone |
| `vUnk2A4()` (`dBg_ctr` setup) x2 | 50/48 | pool-position residual only, confirmed by line-by-line read | positional count inflated by a 1-word length mismatch |
| `vUnk2A8()` (`createModel()`) x2 | 78 each | 8/78 (MAIN), 22/78 (FOOTHOLD) | pool-position/stack-slot residual only |
| `executeState_DemoDown`/`Up` | 33/47 | 2/6 differing | register-choice residual, left alone |
| `__sinit` | 268 | 35 differing | untouched this round, last priority |

**42/51 by raw count -- but every one of the unit's real lifecycle
functions is now written and logically confirmed correct.** The
remaining gaps are, without exception, members of the two
already-well-understood residual classes (pool/stack positioning,
register choice) that this project has repeatedly found do not close on
further source-level variation. `.data`/`.rodata` bounds are both now
final. `__sinit`'s own 35-word gap is the last open item, deliberately
untouched.
