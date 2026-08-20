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
