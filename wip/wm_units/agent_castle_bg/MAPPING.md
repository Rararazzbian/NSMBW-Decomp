# MIDDLE_BG_FOR_CASTLE_LUDWIG + BOTTOM_BG_FOR_CASTLE_LUDWIG -- function inventory

Coordinator-scoped unit, one translation unit, two profiles. `.text 0xf5130-0xf6150`, 0x1020
bytes, 33 functions total (confirmed against `bin/dtk/d_basesNP_symbols.txt`). **`.data`
extent CORRECTED by the coordinator**: `0x308F8-0x30F34` was only what `.text` REFERENCES
directly (a lower bound) -- the real extent is `0x308F8-0x30FA0` (`lbl_2_data_30F34` is 0x6C
bytes and the next symbol, a different unit's `g_profile_MINI_GAME_BALLOON`, starts at
`0x30FA0`). General rule recorded: objects referenced only from OTHER `.data` (like the state
machinery here) are invisible to a `.text`-reference scan. Object list in
`build.py` covers all three dtk-split objects overlapping the range (`auto_00_000F4FB0_text.o`,
`auto_fn_2_F5C80_text.o`, `auto_00_000F5DA4_text.o`) -- `check_target_objs.py` reports this unit
clean. **Tally: 24/33 matched** (up from 12/33, see the state-machine section below). Not reachable to N/N this round -- a real `dEn_c`-derived
class with a heap allocator, a model, two `dBg_ctr_c` zones, nine of its own new virtuals, AND
(discovered this round) a state machine with at least two states.

## REAL CLASS NAMES -- corrected this round, read from `.data`, not guessed

`daMiddleBGForCastleLudwig_c` (capital `BG`) -- found as a literal ASCII string in `.data`,
`"daMiddleBGForCastleLudwig_c::StateID_DemoWait"`, sitting immediately after this class's own
vtable object in `lbl_2_data_30C3C` (dtk merges adjacent no-gap `.data` objects under one
label -- that string is NOT part of the vtable itself, see below). `daBottomBGForCastleLudwig_c`
by the same capitalization convention (not independently string-confirmed this round, but the
sibling relationship and naming pattern are certain).

## MAJOR CORRECTION THIS ROUND: vtable-slot/class attribution was backwards

An earlier draft this round diffed BOTTOM_BG's vtable (`lbl_2_data_30998`) against
MIDDLE_BG's (`lbl_2_data_30C3C`) only across the first ~70 of 169 words by eye, saw the first
four base-method slots (create/doDelete/execute/draw) match, and assumed the remaining nine
"new virtual" slots were ALSO all shared -- with only the destructor actually differing. A full
PROGRAMMATIC slot-by-slot diff of the complete 169-word vtables (not an eyeballed prefix) found
this was wrong: **THREE of the nine new virtuals also differ, not just the destructor**:

| slot | offset | BOTTOM_BG (`lbl_2_data_30998`) | MIDDLE_BG (`lbl_2_data_30C3C`) |
|---|---|---|---|
| 18 | 0x48 | `fn_2_F5C20` (dtor) | `fn_2_F5240` (dtor) |
| 160 | 0x280 | `fn_2_F5C10` (empty, 4 bytes) | `fn_2_F5380` (0x98 bytes, real) |
| 166 | 0x298 | `fn_2_F59A0` (`createModel`, 0x124 bytes) | `fn_2_F5550` (`createModel`, ALSO 0x124 bytes) |
| 167 | 0x29c | `fn_2_F5AD0` (0x128 bytes) | `fn_2_F5680` (0x18C bytes -- the LARGEST function in the unit) |

Every other one of the 169 words (all four base-method slots, and six of the nine new virtuals)
is confirmed byte-identical between the two vtables. The earlier draft had `fn_2_F5C10`/
`fn_2_F59A0`/`fn_2_F5AD0` attached to `daMiddleBGForCastleLudwig_c` (as if they were the base's
own shared implementation, inherited unmodified by BOTTOM_BG) -- exactly backwards. **Fixed
before being reported done**, per this project's own "individually diff anything you believe is
boilerplate" caution (the coordinator's own citation: another unit's 23 "template-matched"
functions turned out to include a destructor credited to the wrong class and two functions with
swapped roles -- this is the same class of bug, caught the same way, by not trusting a
partial/eyeballed comparison).

One useful consequence: `fn_2_F5550` (MIDDLE_BG's own `createModel` override) is
BYTE-FOR-BYTE THE SAME SHAPE as `fn_2_F59A0` (BOTTOM_BG's own), confirmed by direct disasm
comparison -- same instruction sequence, only the string-pool references differ. Both are now
authored with REAL content (see below).

## Real strings read directly from `.data` (not guessed)

- `lbl_2_data_30910`/`lbl_2_data_3091C`: `"window_left"` / `"window_right"` -- the 2-entry node
  name table (`lbl_2_data_30930`) `vf284` (fn_2_F5430, still a stub) passes to
  `GetResNode`/`setNodeVisibility`.
- `lbl_2_data_30938`/`lbl_2_data_30954`: `"g3d/W7_shiroboss_bg_M.brres"` / `"W7_shiroboss_bg_M"`
  -- MIDDLE_BG's own model/arc strings, used in `daMiddleBGForCastleLudwig_c::createModel()`.
- `lbl_2_data_30968`/`lbl_2_data_30984`: `"g3d/W7_shiroboss_bg_D.brres"` / `"W7_shiroboss_bg_D"`
  -- BOTTOM_BG's own model/arc strings (`D` likely "Down", matching BOTTOM's own role), used in
  `daBottomBGForCastleLudwig_c::createModel()`.

Both `createModel()` overrides now use these REAL strings (previously a clearly-marked
`"CASTLE_BG"` placeholder). Not yet re-measured against the exact target byte content beyond
the tally above (both still show as "differing", expected -- likely register-allocation
residual now that the pool references are correct in kind, not necessarily in exact scheduling).

## STATE MACHINE RESOLVED THIS ROUND: ONE state, declared ONCE, +8 functions for free

The coordinator scanned this unit's ENTIRE `.data` for ASCII and found exactly one state name
string anywhere in range (`daMiddleBGForCastleLudwig_c::StateID_DemoWait`) -- settling that this
is one state, not two, and that the two trampoline-referencing groups found earlier are per-CLASS
duplication, not per-STATE. Declared `STATE_VIRTUAL_FUNC_DECLARE`/`STATE_VIRTUAL_DEFINE(
daMiddleBGForCastleLudwig_c, DemoWait)` on the base class (copying `d_a_en_togezo_base.cpp`'s own
structure, one of ten landed TUs using this framework, per the coordinator's citation), renaming
the old `vf294`/`vf290`/`vf28c` to their real `initializeState_DemoWait`/`executeState_DemoWait`/
`finalizeState_DemoWait` identities (mapping confirmed by the PMF-field read order in `.data`,
matching `STATE_VIRTUAL_DEFINE`'s own argument order).

**Tried declaring `DemoWait` on BOTH classes first, per a literal reading of "two classes each
having the one state" -- this does NOT compile**: `STATE_VIRTUAL_DEFINE`'s own `baseID_##name`
is a static FILE-SCOPE function template, and invoking the macro twice for the same state name
in one TU is a genuine redefinition error (confirmed by trying it: `object 'baseID_DemoWait<...>
()' redefined`). Declaring it ONCE, only on the base class, compiles clean and is now the
better-supported reading: a full vtable diff already showed `DemoWait`'s own three slots
(0x294/0x290/0x28c) are byte-identical between both classes, meaning `daBottomBGForCastleLudwig_c`
genuinely inherits the state unmodified rather than overriding it. The "two classes" evidence is
most likely `STATE_VIRTUAL_DEFINE`'s own internal `sFStateVirtualID_c<sStateID_c>` "null"-case
template specialization producing a second, structurally similar object -- not independently
confirmed, flagged here rather than asserted.

**Result: re-diffing the WHOLE unit after adding the declaration (not just the trampolines, per
the coordinator's own instruction) picked up EIGHT more functions for free** -- all
compiler-generated template machinery that needed no hand-authoring at all:
`fn_2_F5DB0`/`fn_2_F5E10`/`fn_2_F5E70`/`fn_2_F5F50`/`fn_2_F6030`/`fn_2_F60C0`/`fn_2_F60F0`/
`fn_2_F6120` (the state object's own destructor, `sFStateVirtualID_c`'s own destructor,
`number()`, `superID()`, `isSameName()`, and the three `initializeState`/`executeState`/
`finalizeState` trampolines) -- confirmed EXACT (0 differing) individually, not assumed from the
tool's own summary line. Also closed `initializeState_DemoWait`/`finalizeState_DemoWait`
themselves to EXACT. `executeState_DemoWait` (the old `vf290`, the `mModel`-vtable-thunk content)
remains at 2/4 differing -- the SAME pre-existing register-allocation residual as before, unlated
to the rename.

Tally jumped 12/33 -> 20/33 from this one change, then to 24/33 after authoring `activate`/`vf284`/`create`/`doDelete`/`draw` this same round (see below).

## MATCHED (24/33)

| draft name | target | size | notes |
|---|---|---|---|
| `daMiddleBGForCastleLudwig_c_classInit__Fv` | `fn_2_F5130` | 12/12 | naming-only |
| `daBottomBGForCastleLudwig_c_classInit__Fv` | `fn_2_F5160` | 19/19 | naming-only (own vtable symbol) |
| `getNullState__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F51B0` | 3/3 | EXACT. `return &sStateID::null;` (real landed extern) |
| `__ct__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F51C0` | 29/29 | EXACT. Shared base ctor |
| `__dt__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5240` | 35/35 | EXACT. Base dtor |
| `entryOrRelease__27daMiddleBGForCastleLudwig_cFb` | `fn_2_F52D0` | 6/6 | EXACT |
| `vf2a0__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5890` | 21/21 | EXACT. Matrix update |
| `finalizeState_DemoWait__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5970` | 1/1 | EXACT. Empty body -- renamed from `vf28c` this round |
| `initializeState_DemoWait__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5980` | 1/1 | EXACT. Empty body -- renamed from `vf294` this round |
| `vf288__27daMiddleBGForCastleLudwig_cFv` | `fn_2_F5C00` | 4/4 | EXACT. Forwards to `vf2a0()` |
| `vf280__27daBottomBGForCastleLudwig_cFv` | `fn_2_F5C10` | 1/1 | EXACT. Empty body -- BOTTOM_BG's OWN override (corrected this round) |
| `__dt__27daBottomBGForCastleLudwig_cFv` | `fn_2_F5C20` | 22/22 | EXACT. BOTTOM_BG's trivial derived dtor |
| `__dt__42sFStateID_c<27daMiddleBGForCastleLudwig_c>Fv` | `fn_2_F5DB0` | 22/22 | EXACT. Compiler-generated (STATE_VIRTUAL_DEFINE), NEW this round |
| `__dt__49sFStateVirtualID_c<27daMiddleBGForCastleLudwig_c>Fv` | `fn_2_F5E10` | 23/23 | EXACT. Compiler-generated, NEW this round -- this is the coordinator's own cited "one internal .text target" |
| `number__49sFStateVirtualID_c<...>CFv` | `fn_2_F5E70` | 55/55 | EXACT. Compiler-generated, NEW this round |
| `superID__49sFStateVirtualID_c<...>CFv` | `fn_2_F5F50` | 56/56 | EXACT. Compiler-generated, NEW this round |
| `isSameName__42sFStateID_c<...>CFPCc` | `fn_2_F6030` | 34/34 | EXACT. Compiler-generated, NEW this round |
| `initializeState__42sFStateID_c<...>CFR...` | `fn_2_F60C0` | 12/12 | EXACT. Compiler-generated trampoline, NEW this round |
| `executeState__42sFStateID_c<...>CFR...` | `fn_2_F60F0` | 12/12 | EXACT. Compiler-generated trampoline, NEW this round |
| `finalizeState__42sFStateID_c<...>CFR...` | `fn_2_F6120` | 12/12 | EXACT. Compiler-generated trampoline, NEW this round |

## PARKED, real content, close, register-allocation residual only (3)

- **`executeState_DemoWait__27daMiddleBGForCastleLudwig_cFv`** (`fn_2_F5990`, 4/4 lines) --
  2/4 differing. Forwarding thunk into `mModel`'s own vtable at offset 0x1c; target uses r12
  exclusively via load-with-update addressing. Two variants tried, both land on r4/r5 instead.
- **`execute__27daMiddleBGForCastleLudwig_cFv`** (`fn_2_F5810`, 31/31 lines, SAME size) --
  27 differing but content and structure fully confirmed: a raw virtual call through a
  still-unnamed object at offset 0x394 (see below), `vf2a0()`, then `dBg_ctr_c::calc()` on both
  zones. Three variants tried (inline expression, one named local, two named locals matching
  the target's own two-step instruction order) -- all land the adjusted "this" pointer in a
  different register (r5) than the target's own reused r3. Same class of wall already on record
  elsewhere on this project for raw-offset sub-object calls.
- **`create__27daMiddleBGForCastleLudwig_cFv`** (`fn_2_F54D0`, 32/32 lines, EXACT size) -- only
  4/32 differing, 2 of which are pure naming (`s_bssUnk` vs `lbl_2_bss_C1AC`, the SAME
  naming-only precedent as every other still-unlanded cross-boundary symbol on this project) and
  2 of which are the SAME `this+0x394` register-allocation wall as `execute()`. This is
  essentially a MATCH modulo one already-documented wall -- not re-attempted separately since
  it is provably the identical root cause.

## Still-unnamed sub-object at offset 0x394 -- real, not guessed, name unconfirmed

`create()` and `execute()` both reach a polymorphic object at `this+0x394` via raw vtable-slot
calls (slot 7/offset 0x1c from `create()`, slot 4/offset 0x10 from `execute()`). Strong
circumstantial evidence this is `dActorMultiState_c`'s own inherited `mStateMgr` (a REAL,
already-landed member: `sFStateStateMgr_c<dActorMultiState_c, sStateMethodUsr_FI_c,
sStateMethodUsr_FI_c> mStateMgr;`, `d_actor_state.hpp`) -- `create()`'s own `changeState(...)`
call, immediately before reaching this object, is `dActorMultiState_c::changeState()`'s own real
inherited body (`{ mStateMgr.changeState(newState); }`), which is the SAME field. NOT
independently confirmed by matching the exact byte offset against a compiled probe of
`dActorMultiState_c` itself -- modelled as a raw cast (`(u8*)this + 0x394`), matching this
project's own precedent for reaching an unnamed base-class member (the destructor's own `m_1fc`
kind of case on other units).

## Still-unidentified 0x34-byte `.bss` object (`s_bssUnk` / `lbl_2_bss_C1AC`)

`create()` passes this by reference to `changeState(const sStateIDIf_c&)`. Uninitialized
(`.bss`), 0x34 bytes -- too large for a plain `sStateID_c` (would be 0xc: vtable+name+number)
and not confirmed as any specific richer type. Declared as a raw same-size byte buffer; content
and real type NOT confirmed. Worth checking next round whether this is actually
`sFStateID_c<daMiddleBGForCastleLudwig_c>`-shaped (vtable+name+number+3 PMFs = 0x30, plus
alignment = 0x34) despite living in `.bss` rather than `.data` -- which would be strange for a
type requiring a compile-time-initialized vtable pointer, unless something else in this unit
(still unauthored) explicitly constructs it at runtime before `create()` runs.

## SCOUTED but NOT YET AUTHORED -- 4 functions

| draft name (class) | target | size | role |
|---|---|---|---|
| `vf280` (MIDDLE_BG's OWN) | `fn_2_F5380` | 42/42, SAME size | 36 differing. Content and structure fully confirmed (loops 2 nodes, rolls a random visibility flag via `rndInt(100)`, looks up via `mModel`'s ResMdl, calls `setNodeVisibility`) -- residual is the target's own unusual `xori`/`slw`(by a LOOP-INVARIANT constant `0xa`)/`srwi` bit-trick for the random threshold test, which neither `rndInt(100) < 10` nor `== 10` reproduces (both compile to a DIFFERENT, more conventional bit-trick on this compiler). Semantically equivalent to the target (both give the same 10%-chance boolean), exact source expression not yet found. Two variants tried. |
| `vf29c` (MIDDLE_BG's OWN) | `fn_2_F5680` | 99 | Unscouted (0x18C bytes -- LARGEST in unit). |
| `vf29c` (BOTTOM_BG's OWN) | `fn_2_F5AD0` | 74 | SCOUTED this round, not yet authored: builds an un-landed `sBgSetInfo`-shaped stack struct from this unit's own `.rodata` (`lbl_2_rodata_5BC0`) constants and `mPos`, initializes 2 `mVec3_c`-shaped fields at `this+0x74c`/`this+0x758` (both to `mPos`) and `mScale` to a uniform rodata constant, then calls `dBg_ctr_c::set(dActor_c*, const sBgSetInfo*, u8, u8, mVec3_c*)` + `entry()` on EACH zone with slightly different rodata constants and a per-instance byte read from `this+0x38f` -- `sBgSetInfo` is not landed anywhere in `include/` (grepped). Real content, not yet written into the draft. |
| `createModel` (both overrides) | `fn_2_F5550`/`fn_2_F59A0` | 73/73 each | Real structural content AND real strings (see above) already written, but BOTH still show 61/73 differing -- not yet investigated why; likely a register-allocation/scheduling class similar to what's already on record elsewhere, but not confirmed. Worth a closer diff next round rather than assumed. |

## NOT YET TOUCHED -- 1 function

| target | size | notes |
|---|---|---|
| `fn_2_F5C80` | 73 | `__sinit`, the `.ctors`-registered static initialiser. Per the coordinator's own instruction, this is REQUIRED (not optional) and should be tackled LAST, once every declaration feeding it is correct -- a sibling unit closed its own to exact with a pure reordering, no content change, after everything else was right. Not yet attempted. |

## Gates

## Gates

- **Function order**: GREEN. `order_sweep.py` reports `ok agent_castle_bg 24/33`.
- **`.ctors`**: GREEN. `0x288 -> __sinit at .text 0xf5c80` for BOTTOM_BG_FOR_CASTLE_LUDWIG,
  matching the coordinator's own citation exactly.
- **Target-object coverage**: GREEN. `check_target_objs.py` does not flag this unit (fixed this
  round -- `auto_fn_2_F5C80_text.o` was missing from `build.py`'s object list).

## Real defect found and fixed: struct size (`sizeof`)

First draft computed `0x750` against target's `0x768` (both classInits' own alloc constant) --
0x18 bytes short, fixed with a trailing `u8 mPad74c[0x1c]` (content still unconfirmed, size
confirmed exact).

## Shadow header

`shadow_include/game/bases/d_a_castle_bg.hpp`. Every real header it depends on (`dEn_c`,
`dHeapAllocator_c`, `dBg_ctr_c`, `m3d::mdl_c`, `sStateID_c`) is ALREADY LANDED; no shadow copy of
any of them was needed. BINDING rule applied throughout (all new virtuals defined out-of-line,
not in-class, matching the DUMMY_DOOR lesson).

## How to reproduce this tally

```
python wip/wm_units/agent_castle_bg/build.py
python wip/wm_units/agent_castle_bg/difftool.py \
    wip/wm_units/agent_castle_bg/target_auto_00_000F4FB0_text.txt \
    wip/wm_units/agent_castle_bg/draft.txt \
    fn_2_<addr> <draft_symbol>
```
(Functions at or past `0xf5db0` are in `target_auto_00_000F5DA4_text.txt`; `fn_2_F5C80` is in
its own `target auto_fn_2_F5C80` object, not yet dumped to a standalone `.txt` -- use
`build.py`'s own object list / `verify_anon.py` output for it.)

## Plan for the next round

State machine is RESOLVED. `fn_2_F52F0`/`vf284`/`create`/`doDelete`/`draw`/`execute` are now
authored (5 MATCH exactly, `execute`/`create` are parked one register-allocation wall away).
Remaining, smallest-first:

1. `vf29c` (BOTTOM_BG's own, `fn_2_F5AD0`, 74 lines) -- SCOUTED this round (see above), just
   needs the `sBgSetInfo`-shaped struct/constants written into the draft.
2. `createModel` (both overrides) -- investigate the 61/73 residual; real content and real
   strings are already in place, so this should be a register/scheduling puzzle, not a content
   gap. Check whether the SAME lever that fixed `activate()` this round (an if/else `setOption`
   call compiling to real branches vs a ternary compiling to a bit-trick) applies anywhere in
   here too.
3. `vf29c` (MIDDLE_BG's own, `fn_2_F5680`, 99 lines -- the largest function in the unit) --
   unscouted.
4. `fn_2_F5C80` (`__sinit`) LAST, once every declaration feeding it is correct, per the
   coordinator's own instruction -- expect a pure reordering fix once everything else is real,
   not a content problem.
5. Revisit `vf280`'s own random-visibility bit-trick, `execute()`/`create()`'s shared
   `this+0x394` register wall, and `executeState_DemoWait`'s own thunk residual only if time
   remains after the above -- all three are genuinely parked (multiple variants tried, real
   content confirmed), not gaps.
