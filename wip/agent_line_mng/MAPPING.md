# d_line_mng.cpp -- layout & state-framework reconstruction

Target: `.text` VA `0x800C0DC0`-`0x800C89A0` (offset `0xBA640`-`0xC2220` from
`.text` base `0x80006780`), 182 functions, `wiimj2d.dol`. Nothing landed for
this unit before this round. This round's job was the shared prerequisite --
class layout + state framework -- not full authorship.

All evidence below comes from disassembling `bin/dtkspl/obj/auto_03_800BFEBC_text.o`
(covers `0x800BFEBC`-`0x800C88A4`, containing all but the tail of this unit) and
`bin/dtkspl/obj/auto_03_800C88A4_text.o` (the tail: `sFStateID_c<dLineMng_c>`'s
own destructor and `isSameName`) plus `bin/dtkspl/obj/auto_sinit__d_line_mng_cp_text.o`
for `__sinit`. These three objects were disassembled and the 182 target function
bodies extracted verbatim into `wip/agent_line_mng/work/target.txt`, which is
registered as the harness workdir's `target.txt` at
`tools/auto_decomp/work/dol_bases_d_line_mng/target.txt`. All 182 named
functions were found -- zero misses -- confirming the address range and the
function list are exactly right.

## HEADLINE RESULT

**Declaring the class layout and the 25-state framework correctly, with the
constructor and empty stub bodies for the legitimately-empty state methods
(nothing else authored), took the unit from 0/182 to 67/182 matching
functions (36.8%).** This is MEASURED: `wip/agent_line_mng/work/named/d_line_mng.cpp`
compiles clean against the shadow header, and 67 of 182 target functions diff
byte-identical against it. Notably `__ct__10dLineMng_cFv` -- the class's own
constructor -- is one of the 67, which is strong independent confirmation the
layout below is right (a wrong member offset or a wrong `mStateMgr` sub-shape
would have shown up here first).

The 67 are: the constructor; all six template-instantiated destructors
(`sFStateStateMgr_c`, `sStateStateMgr_c`, `sFStateMgr_c`, `sStateMgr_c`,
`sFStateFct_c`, `sFState_c`); 25 `finalizeState_*` (every one, all empty);
`initializeState_Idle`/`finalizeState_Idle`/`executeState_Idle` and
`initializeState_FallDown`/`finalizeState_FallDown` (empty in the target); and
34 more weak template methods emitted purely from declaring
`sFStateStateMgr_c<dLineMng_c, sStateMethodUsr_FI_c, sStateMethodUsr_FI_c> mStateMgr`
(`changeState`, `executeState`, `getStateID`, `initializeState`,
`isSubState`, `returnState`, `getOldStateID`, `build`, `dispose`,
`initialize`/`execute`/`finalize` on `sFState_c`, `refreshState`,
`changeToSubState`, `getState`, `getNewStateID`, `getMainStateID`, all ten
`sStateMgr_c<...>` methods, all three `sFStateID_c<dLineMng_c>` per-owner
call trampolines, its destructor, and `isSameName`).

None of this was hand-authored beyond the constructor body and 5 trivially
empty state-method stubs -- the other 62 are pure framework emission.

Full match/no-match list: `wip/agent_line_mng/work/unmatched.txt` (115 not yet
matching) vs the 67 above.

## Correction to the brief: this is NOT a plain `sFStateMgr_c`

The brief said `d_pausewindow.cpp` "demonstrates the identical member shape."
It does not, and the difference is the single biggest structural finding of
this round.

`Pausewindow_c` has one member:
`sFStateMgr_c<Pausewindow_c, sStateMethodUsr_FI_c> mStateMgr;`

`dLineMng_c` has a **nested state-of-states manager**:
`sFStateStateMgr_c<dLineMng_c, sStateMethodUsr_FI_c, sStateMethodUsr_FI_c> mStateMgr;`

Evidence: the symbol map (`wip/agent_line_mng/all_symbols.txt`) carries BOTH
`sFStateStateMgr_c<10dLineMng_c,...>` / `sStateStateMgr_c<10dLineMng_c,12sFStateMgr_c,...>`
template instantiations AND `sFStateMgr_c<10dLineMng_c,...>` /
`sStateMgr_c<10dLineMng_c,...>` instantiations for this one class -- Pausewindow
only ever has the latter pair. Per `include/game/sLib/s_FStateStateMgr.hpp` and
`s_StateStateMgr.hpp`, `sFStateStateMgr_c<T,M1,M2>` wraps
`sStateStateMgr_c<T, sFStateMgr_c, M1, M2>`, which contains a `mainMgr` AND a
`subMgr` (both `sFStateMgr_c<T,M1>`/`sFStateMgr_c<T,M2>` -- identical here since
M1==M2==`sStateMethodUsr_FI_c`) plus a `currentMgr` pointer. The constructor's
disassembly shows this literally: it writes vtables and runs
`sStateMethodUsr_FI_c`'s real (never-inlined, out-of-line) constructor **twice**,
at `this+0x70` and `this+0xac`, then stores `currentMgr = &mainMgr` at `this+0xe8`.

`(vtable size - 8) / 4` slot count confirms the class hierarchy too:
`__vt__49sFStateMgr_c<...>` and `__vt__79sStateMgr_c<...>` are both `0x30`
(10 slots, matches `sStateMgrIf_c`'s 10 pure virtuals) while
`__vt__91sStateStateMgr_c<...>` and `__vt__77sFStateStateMgr_c<...>` are both
`0x40` (14 slots = the same 10 plus `sStateStateMgrIf_c`'s 4 additions:
`changeToSubState`, `returnState`, `isSubState`, `getMainStateID`).

**All 25 states use the plain, non-virtual `STATE_FUNC_DECLARE`/`STATE_DEFINE`
macro**, not `STATE_VIRTUAL_*`. Confirmed by the PMF-encoding rule from
HANDOFF.md (`{-1, fn_addr, 0}` = non-virtual): the `.data` state objects are
all instances of `sFStateID_c<10dLineMng_c>` (confirmed by the symbol map --
there is no `sFStateVirtualID_c<10dLineMng_c>` anywhere), and `sFStateID_c<T>`
itself is the ordinary, single, non-templated-per-state virtual dispatcher
declared in `s_FStateID.hpp`. This is also why `dLineMng_c` needs no vtable
of its own -- the polymorphism lives entirely inside `mStateMgr`.

## The constructor defers the first state to `init()`

The brief's assumption (mirroring Pausewindow's
`mStateMgr(*this, StateID_InitWait)`) does not hold here either.

`__ct__10dLineMng_cFv` is `Fv` -- no arguments, so it cannot receive a real
starting state. Its disassembly loads `&sStateID::null` (`null__8sStateID`)
and passes that as the `initializeState` argument to `sFStateStateMgr_c`'s
constructor -- **twice**, once for each of the two `sStateMethodUsr_FI_c`
sub-constructions (`mainMgr` and `subMgr`), both reusing the same
`lis r30, null__8sStateID@ha` value. So:

```cpp
dLineMng_c::dLineMng_c() : mStateMgr(*this, sStateID::null) {}
```

`init()` sets the real first state later, via a **virtual call**
(`mStateMgr.changeState(StateID_Idle)`) at the very end of its body -- confirmed
by loading `mStateMgr`'s vtable pointer (`this+0x6c`) and calling through slot
offset `0x18`, which by the project's `(offset-8)/4` slot rule is slot 4 =
`changeState` (order: dtor=0, initializeState=1, executeState=2,
finalizeState=3, changeState=4, matching `sStateMgrIf_c`'s declared order).
The same rule independently explains `move()`'s call through slot offset
`0x10` (slot 2 = `executeState`).

This was compiled and matched: `__ct__10dLineMng_cFv` is byte-exact with
exactly this constructor body (see "Headline result" above).

## Class layout (`sizeof(dLineMng_c) == 0xEC`, COMPILED and verified)

All offsets below were verified with a compiled `offsetof` probe against the
shadow header (`wip/agent_line_mng/work/probe_offsets.cpp`, using a
temporarily-public copy of the header so `offsetof` can see private members --
the shipped shadow header keeps them private). `sizeof(dLineMng_c) == 0xEC`
was separately confirmed with a template `Probe<N>` specialisation
(`wip/agent_line_mng/work/probe.cpp`, compiles for `N=0xEC`, and a control run
with `N=0xE8` reproduces the expected MWCC error `illegal use of incomplete
struct/union/class`, proving the check actually discriminates).

| offset | size | member | evidence |
|---|---|---|---|
| `0x00` | `0x38` | `mVec2_c mDirVec[7]` | ctor: `bl __construct_array` with `__ct__7mVec2_cFv`/`__dt__7mVec2_cFv`, elemSize `0x8`, count `0x7`, base = `this+0`. `init_term_ck_pos()` zeroes slots `[0..2]` and copies a lazily-initialised static 8-float table into slots `[3..6]` |
| `0x38` | `0x8` | `mVec2_c mSpeed` | zeroed in `init()`; by far the most-referenced field across every state's execute method (per-frame delta, unconfirmed exact role) |
| `0x40` | `0x8` | `mVec2_c mPos` | `GetPos()`/`SetPos()`, both byte-exact matched in isolation reads/writes here |
| `0x48` | `0x8` | `mVec2_c mOldPos` | set to a copy of the initial pos in `init()`; `move()` overwrites it from `mPos` AFTER calling `executeState()` -- a previous-frame snapshot |
| `0x50` | `0x8` | `mVec2_c mUnitBasePos` | computed once in `init()`: `(f32)(int)(pos/smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X` per axis (grid-snap). Heaviest-used field in the unit (55-56 references) |
| `0x58` | `0x8` | `mVec2_c` (name open) | **NOT touched by `init()` at all** -- no store anywhere in its disassembly. First written inside the unnamed `fn_800C31C0` (0x894 bytes); read by `circle_nextpos_set` and two more unnamed helpers (`fn_800C3B20`, `fn_800C3B60`) |
| `0x60` | `0x4` | `f32 mBaseSpeed` | `SetBaseSpeed()`, `change_dir()`, set from `init()`'s `f32 speed` param |
| `0x64` | `0x2` | `u16 mAngle` | zeroed in `init()`; `acm_angle()` reads it, adds/subtracts `0x4000` by `mReverse`, masks to 16 bits |
| `0x66` | `0x1` | `u8 mType` | `init()`'s `u8 param` argument, stored verbatim |
| `0x67` | `0x1` | `u8` (name open) | zeroed at the very END of `init()`, AFTER the `changeState` call returns. `move()` skips `executeState()` when this is nonzero -- reads as a "skip this frame" gate |
| `0x68` | `0x1` | `u8 mReverse` | `change_dir()` flips it and negates `mBaseSpeed` together; `acm_angle()`/`SetBaseSpeed()` both branch on it |
| `0x69` | `0x1` | `u8 mLineType` | low byte of `init()`'s `int lineType` param, stored BEFORE `init_term_ck_pos()` runs |
| `0x6a` | `0x2` | (implicit padding) | alignment for `mStateMgr`'s vtable pointer; not a named member |
| `0x6c` | `0x80` | `sFStateStateMgr_c<dLineMng_c, sStateMethodUsr_FI_c, sStateMethodUsr_FI_c> mStateMgr` | see above; ends the object at `0xec` |

No allocation site (global instance, array, or owning-class member) for
`dLineMng_c` was found anywhere in `bin/dtk/wiimj2d_symbols.txt` -- this is a
gap, not a contradiction: it means `sizeof` could not be cross-checked against
an independent second source, only against the constructor's own writes (which
never touch anything past `0xec`) and the compiled probe. Flagging this as the
one open structural question, per the "report contradictions/gaps, don't
paper over them" rule.

**Owning class is not this class.** The commit history in this checkout
(`dBg_ctr_c::set third overload + sBgSetInfo`) and the class's own purpose
(line/rail movement math) both point at a background-control actor owning a
`dLineMng_c` member, but that TU has not been decompiled and no reference to
`dLineMng_c` from outside its own `.text` range was found. Not investigated
further this round -- out of scope for "this class's own layout."

## `mDirVec[7]`'s lazy-static table

`init_term_ck_pos()` (0x94 bytes) is worth flagging for whoever authors it: it
contains a **function-local `static` array with a guard bool**, the classic
Meyers-singleton codegen shape (hidden bool checked/set around the
one-time-fill block). The table is 8 floats (4 `mVec2_c` pairs), and after the
guarded block runs (or is skipped on later calls) the SAME table is copied into
`mDirVec[3..6]` unconditionally. `mDirVec[0..2]` are zeroed unconditionally on
every call, not just the first. Exact float values not extracted this round --
low effort, high value for whoever authors this function next.

## Unnamed file-scope functions (7 total, not class members)

These carry no mangled class-scope name (`fn_XXXXXXXX` placeholders from dtk),
sandwiched between named `dLineMng_c` methods in `.text`, so they are almost
certainly `static` (internal-linkage) free functions in `d_line_mng.cpp`, not
private members -- consistent with the brief's note about "self-contained
`mVec2_c` geometry" functions.

| address | size | notes |
|---|---|---|
| `0x800C15B0` | `0x1C` | `void fn(mVec2_c *arr, const mVec2_c *src, int idx)` shape: `arr[idx] = *src` for an 8-byte-stride array. Args in r3/r4/r5, no `this` semantics -- reads as a plain C-style helper, not a method |
| `0x800C1EE0` | `0x90` | not yet read |
| `0x800C31C0` | `0x894` | **by far the largest function in the unit** (2196 bytes) -- larger than any named method. First writer of the `0x58` `mVec2_c` field. Read this first if picking up circle-movement authorship |
| `0x800C3B20` | `0x3C` | reads offset `0x50`/`0x54` (`mUnitBasePos`) |
| `0x800C3B60` | `0x3C` | reads offset `0x54` |
| `0x800C3BA0` | `0x48` | not yet read |
| `0x800C3BF0` | `0x20` | not yet read |

## What remains (115 of 182)

- `__sinit_\d_line_mng_cpp` (1): **close but not exact**. Compiled with all 25
  `STATE_DEFINE`s in `.bss`/`.text` declaration order (verified identical
  between the two), size came out `1220` words against a target of `1193` --
  27 off, and the target uses `_savegpr_27`/`_restgpr_27` (registers 27-31)
  where the draft uses more direct stack stores. This reads as a scheduling
  difference from `STATE_DEFINE` invocation order in the `.cpp`, not a
  structural error -- the state COUNT and shape are right, only the ordering
  needs tuning. Confirmed the `.bss` object order (`Idle, FallDown, Left45,
  Right45, Side, Height, CornerHeightLine, CornerSideLine, Left30Left,
  Left30Right, Right30Left, Right30Right, Left60Up, Left60Down, Right60Down,
  Right60Up, Circle, Circle2x2Leftup, Circle2x2Rightup, Circle2x2LeftDown,
  Circle2x2RightDown, Circle4x4Rightup, Circle4x4LeftUp, Circle4x4LeftDown,
  Circle4x4RightDown`) exactly matches the `.text` function order, so that is
  almost certainly the real `STATE_DEFINE` order to use -- it is what
  `draft.cpp` already uses and it did NOT close `__sinit`, so the remaining
  gap is something else (maybe interleaving `STATE_DEFINE` with other `.data`,
  per the project's other state-framework units).
- 7 unnamed helpers (table above) -- unauthored.
- 107 named `dLineMng_c` methods needing real bodies, including all 25
  `initializeState_*`/`executeState_*` pairs that are not trivially empty
  (`executeState_FallDown` onward -- `initializeState_Idle` and both
  `FallDown` triples besides `executeState_FallDown` were already empty and
  matched for free).

Two return types are explicitly UNPROVEN and left `void` as a placeholder in
the shadow header, per the CFront-mangling rule (return types are never in the
mangled name) -- `CalcAdjustPosY(f32,f32)` and `getLineUnitNo(f32,f32)`
(the latter calls `dBc_c::getUnitType`/`getUnitKind`, both of which take a
trailing `u8` and mask their result to 8 bits, suggesting `u8` or `bool`, but
this is an inference, not a register-allocation test, so it is NOT asserted).
`acm_angle()`'s return type is also unproven, `void` as a placeholder --
whatever calls it will need the compile-both-ways test.

## Files

- `wip/agent_line_mng/shadow_include/game/bases/d_line_mng.hpp` -- the proposed
  header (does not touch `include/`).
- `wip/agent_line_mng/work/target.txt` -- all 182 target function bodies,
  extracted from the three split objects above, registered as
  `tools/auto_decomp/work/dol_bases_d_line_mng/target.txt` for harness use.
- `wip/agent_line_mng/work/draft.cpp` / `named/d_line_mng.cpp` -- the
  67/182-matching draft (constructor + 25 `STATE_DEFINE`s + empty stub bodies
  only). The `named/` copy is compiled under the real target filename because
  `__sinit_\<filename>_cpp`'s mangled name embeds the source filename.
- `wip/agent_line_mng/work/unmatched.txt` -- the 115 still-open functions.
- `wip/agent_line_mng/work/probe.cpp`, `probe_offsets.cpp` -- the compiled
  `sizeof`/`offsetof` assertions cited above.
- `wip/agent_line_mng/all_symbols.txt`, `text_range.txt` -- raw symbol-map
  extracts this round was built from.
