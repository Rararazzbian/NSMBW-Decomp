# WM_KINOPIO -- daWmKinopio_c

`.text 0x16c150-0x16d290`, 19 functions (17 in `target_auto_00_0016C124_text.txt`
+ the `.ctors` function `fn_2_16D1E0` + its registered callback `fn_2_16D270`,
both in the other two dump files), module `d_basesNP`. Base class
`dWmDemoActor_c` (confirmed: ctor's `bl __ct__14dWmDemoActor_cFv`).

## Result: 7/19 byte-identical this round (fresh unit, first round)

| target | size | draft | note |
|---|---|---|---|
| `fn_2_16C150` classInit | 0x30 | **MATCH** | `ACTOR_PROFILE(WM_KINOPIO, daWmKinopio_c, 0)`; `li r3,0x1bc` confirms `sizeof == 0x1bc` before anything else was authored |
| `fn_2_16C180` ctor | 0x3C | **MATCH** | empty body; base ctor + auto secondary-vtable pointer only |
| `fn_2_16C1C0` dtor | 0xB0 | **MATCH** | `if (mpMdlMng) delete mpMdlMng;` -- the redundant explicit null check (delete already null-checks) is load-bearing: without it the target's doubled `beq` to the same target is missing one instruction |
| `fn_2_16C3A0` draw | 0x40 | **MATCH** | `mpMdlMng->draw(); DrawShadow(true); return SUCCEEDED;` |
| `fn_2_16C3E0` doDelete | 0x8 | **MATCH** | trivial `return 1;` |
| `fn_2_16C5C0` resetStep | 0xC | **MATCH** | `m_190 = 0;` |
| `fn_2_16C5D0` unusedStub | 0x4 | **MATCH** | empty body, PTMF-table placeholder |
| `fn_2_16C270` resetInit | 0x78 | not authored | characterized |
| `fn_2_16C2F0` execute | 0xA4 | not authored | characterized |
| `fn_2_16C3F0` createModel | 0xA0 | not authored | characterized |
| `fn_2_16C490` calcModel | 0xA0 | not authored | characterized |
| `fn_2_16C530` resetPosition | 0x90 | not authored | characterized |
| `fn_2_16C5E0` processCutsceneCommand | 0x230 | not authored | characterized |
| `fn_2_16C810` stepCutscene70 | **0x834** | not authored | largest function found this session by a wide margin; a 20-entry jump-table state machine, see Negatives |
| `fn_2_16D050` checkAnmLoop | 0xB0 | not authored | characterized |
| `fn_2_16D100` startJump | 0x84 | not authored | characterized |
| `fn_2_16D190` checkSpawnGate | 0x4C | not authored | characterized |
| `fn_2_16D1E0` `.ctors` init | 0x84 | not authored | characterized, see Negatives |
| `fn_2_16D270` `.ctors` callback | 0x1C | not authored | fully understood (destroys a `dWmLib::ForceInCourseList_t[1]`), trivial to author, not yet done for lack of time |

## Class layout -- measured, and mostly already inherited

`sizeof(daWmKinopio_c) == 0x1bc`, confirmed by `classInit`'s own `li
r3,0x1bc` (checked before writing anything else, per standing practice) and
independently by chaining `STATIC_ASSERT`/`Probe<N>` compiles against the
REAL headers already in `include/`.

**The single biggest finding this round**: the dtor destructs objects at
`this+0x13c` (`__dt__16mHeapAllocator_cFv`) and `this+0x158`
(`__dt__Q23m3d6smdl_cFv`) directly. A naive reading would treat these as
new members. A `Probe`/`offsetof` compile against `dWmDemoActor_c` (already
fully declared in `include/game/bases/d_wm_demo_actor.hpp`) settled it
immediately: `dWmDemoActor_c::mHeapAllocator` is at `0x13c` and
`dWmDemoActor_c::mModel` is at `0x158` -- **both are dWmDemoActor_c's own
protected members**, not new fields. Same for the "mystery" offsets found
mid-disassembly at `0x100` (`dBaseActor_c::mAngle`), `0x124`
(`dBaseActor_c::mVisible`), and `0x139` (`dWmDemoActor_c::mIsCutEnd`) --
all independently confirmed inherited via the same probe chain
(`dBaseActor_c` ends at `0x128`, `dWmActor_c` adds `mClipSphere` and ends at
`0x138`, `dWmDemoActor_c` adds `m_00`/`mIsCutEnd`/`mHeapAllocator`/`mModel`/
`mSvMdl`/`mTargetPos`/`mScaleCurr`/`mScaleDelta`/`mScaleTarget`/
`mScaleDelay` and ends at `0x184`). This is the "unaccounted constructor
call is an embedded member before it is a new field" rule, now confirmed a
third time and this time for two separate members and three separate
"mystery offset" resolutions at once.

`daWmKinopio_c`'s own new members therefore span exactly `[0x184, 0x1bc)`
-- 0x38 bytes, 14 four-byte slots, no remainder:

```cpp
class daWmKinopio_c : public dWmDemoActor_c {   // sizeof == 0x1bc
    u32 m_184;                // unobserved
    dPyMdlMng_c *mpMdlMng;    // @0x188 -- confirmed: operator new(0xc), dPyMdlMng_c(ModelType_e) ctor,
                               //           dtor calls delete through its own vtable
    u32 m_18c;                 // unobserved
    int m_190;                  // step/PTMF-table index; resetStep() sets it 0
    u32 m_194;                   // unobserved
    int m_198;                    // cutscene sub-state, set to 0xe/0xf inside stepCutscene70
    mVec3_c m_19c;                  // target position, written by processCutsceneCommand
    int m_1a8;                       // main state index (0-19), the stepCutscene70 jump-table key
    int m_1ac;                        // loop counter, wraps at 1000 -> 0
    u32 m_1b0;                         // unobserved
    u8 m_1b4; u8 pad[3];                // byte flag
    dWmActor_c *m_1b8;                   // dWmActor_c::construct(0x28f, this, 0x10000, 0, 0) result -- a spawned child actor
};
```

`dPyMdlMng_c` (used via `mpMdlMng`) is already fully declared in
`include/game/bases/d_player_model_manager.hpp` with every method this
unit calls (`create`, `draw`, `play`, `calc(mVec3_c,mAng3_c,mVec3_c)`,
the `ModelType_e`/`SceneType_e` constructor argument), so nothing needed
to be guessed there either.

## Per-function role summary (from direct disassembly reading)

- `resetInit` (`fn_2_16C270`): calls `createModel()`, copies `mPos` into a
  second position-ish field at `0x128-0x134` (**not yet reconciled with the
  `[0x184,0x1bc)` new-member map above -- these offsets were read under the
  OLD, now-obsolete belief that daWmKinopio_c added members below 0x184;
  given the layout work above, `0x128` is `dWmActor_c::mClipSphere`, so this
  write is almost certainly through `mClipSphere`'s own fields, not a new
  member -- flagged as an open reconciliation, not re-verified this
  round**), then calls `resetPosition()`, then sets `mScale` from a rodata
  constant.
- `execute` (`fn_2_16C2F0`): if a cutscene is playing, dispatches
  `processCutsceneCommand` through the secondary vtable (slot `0x60`);
  otherwise dispatches through a PTMF table (`this+0x190`, entries `0xC`
  bytes, matching the established idiom) indexed by `m_190`. Always calls
  `checkAnmLoop()` then `resetInit()` at the end -- **this last call looks
  wrong for an `execute()` (re-running init every frame); worth
  re-verifying the target address is really `fn_2_16C270` before trusting
  this reading**.
- `createModel` (`fn_2_16C3F0`): `operator new(0xc)` + placement `dPyMdlMng_c(ModelType_e=4)`,
  `create(1,1,1)`, `CreateShadowModel(...)` (two data-table string args),
  then conditionally (`checkSpawnGate()`) spawns a child actor via
  `dWmActor_c::construct(0x28f, this, 0x10000, nullptr, nullptr)` into `m_1b8`.
- `calcModel` (`fn_2_16C490`): `CalcShadow(k1,k2)`, builds `mAng3_c`/`mVec3_c`/
  a 3-short struct from `mPos`/`mScale`/`mAngle` (`0x100`), calls
  `mpMdlMng->calc(...)` then `mpMdlMng->play()`.
- `resetPosition` (`fn_2_16C530`): resets `mPos`, calls a virtual dispatch
  through `mpMdlMng->mpMdl`'s own vtable (deep double-indirect, not yet
  matched to a named `dPyMdlBase_c` method), clears `m_1b4`, calls `resetStep()`.
- `processCutsceneCommand` (`fn_2_16C5E0`): standard `(int cutsceneCommandId,
  bool isFirstFrame)` signature; on command `0x70`+first-frame, computes an
  interpolated target position from two `daWmMap_c::GetPos()` node lookups
  scaled by rodata constants, stores it to `m_19c`; on every call with
  command `0x70`, calls `stepCutscene70()`; otherwise sets `mIsCutEnd`.
- `stepCutscene70` (`fn_2_16C810`, **0x834 bytes**): `calcSpeed()`/`posMove()`
  (both `dBaseActor_c` methods), footstep-sound triggering via
  `isFootStepTiming()`, then a 20-entry (`m_1a8` in `[0,0x13]`) jump-table
  dispatch (`jumptable_2_data_45D14`) into per-state movement/animation
  logic using paired-single (`ps_`/`psq_`) float math extensively. This is
  the single largest function encountered in this project this session --
  not attempted; would need multiple dedicated rounds on its own.
- `checkAnmLoop` (`fn_2_16D050`): if `m_1a8` in `[2,9]`, checks the model's
  current animation frame against a threshold; if past it, dispatches a
  virtual call (through `mpMdlMng->mpMdl`, vtable slot `0x84`) and
  increments `m_1ac` (wrapping at 1000).
- `startJump` (`fn_2_16D100`): `daWmMap_c::GetNodePos()`, scales the result,
  calls `dWmDemoActor_c::_initDemoJumpBase(...)` (already declared), clears
  `m_1b4`.
- `checkSpawnGate` (`fn_2_16D190`): `dWmLib::IsSingleEntry() && !fn_800FCB30(0)`
  (the latter an unnamed cross-module DOL call, same category as
  `fn_80103420` seen in `d_a_wm_kinoko_base.cpp`).
- `fn_2_16D1E0` (`.ctors`): constructs a file-local `dWmLib::ForceInCourseList_t`
  (already declared in `include/game/bases/d_wm_lib.hpp`, alongside the
  real `sc_ForceList[]` -- our unit needs its OWN separate instance, not
  that array) whose `mNodeName`/`mLevelNode` point at this unit's own
  leading anonymous 5-byte strings (`lbl_2_data_45C80`="F7C0",
  `lbl_2_data_45C88`="W7C0" -- confirmed via `tools/relfile.py`, a
  `.data`-internal relocation from the object's own fields, not a `.text`
  reference) and whose `mLevel` field is patched at load time from the
  runtime global `dCsvData_c::c_CASTLE_ID` (not a compile-time constant,
  which is why this needs dynamic init at all), then registers the object
  for cleanup via `__register_global_object(..., fn_2_16D270, ...)`.
  `fn_2_16D270` itself (the destructor callback) is a trivial
  `__destroy_arr(&obj, ~ForceInCourseList_t, 0x24, 1)` tail call -- fully
  understood, not yet transcribed into the draft for lack of remaining
  time this round.

## Data layout -- measured via relocations

`g_profile_WM_KINOPIO` at `.data 0x45cc0` (size `0xc`, standard), its
`classInit` field relocating directly to `fn_2_16C150` -- confirmed with
`tools/relfile.py` exactly as for WM_ITEM. `check_bounds.py`'s ownership
check flags it as "referenced from 0x12660" (outside every claimed range)
-- this is the profile-registry-table false positive documented for
`g_profile_*` objects (the tool's own docs note shared/profile objects are
legitimately referenced from elsewhere); overridden on the same evidence
basis as WM_ITEM.

Unlike WM_ITEM, **this unit's `.data` genuinely does open on the two
anonymous `sc_ForceList`-shaped 5-byte strings** (`lbl_2_data_45C80`
="F7C0", `lbl_2_data_45C88`="W7C0"), matching `check_bounds.py`'s family
rule as designed -- confirmed by the `.data`-internal relocation from the
`ForceInCourseList_t` object's own `mNodeName`/`mLevelNode` fields, not
assumed from the heuristic alone.

`.data` ends at `0x45dd8`, exactly where the NEXT unit's own leading
5-byte-string pair begins (`lbl_2_data_45DD8` = "F7C0" again, a different
neighbour's own copy) -- a clean, symbol-boundary-exact stop confirmed by
`lbl_2_data_45D68` (the secondary vtable, size `0x70`) ending precisely
there.

`.rodata 0x8b10-0x8ba0` is one contiguous table (`lbl_2_rodata_8B10`
through `_8B58`, dtk keeps these as separate symbols but every reference in
the target uses one shared base register per function, the same pattern
established on WM_ITEM) -- not yet consolidated into a single named array
in the draft since only the already-matching functions have been authored.

## Proposed slice

```json
{
  "source": "d_basesNP/bases/d_a_wm_kinopio.cpp",
  "memoryRanges": {
    ".text": "0x16c150-0x16d290",
    ".data": "0x45c80-0x45dd8",
    ".rodata": "0x8b10-0x8ba0",
    ".bss": "0xfeb0-0xfec0",
    ".ctors": "0x40c-0x410"
  }
}
```

`check_bounds.py d_basesNP` on this claim: `.text`/`.rodata`/`.bss`/`.ctors`
report clean; `.data`'s only flag is the `g_profile_*` ownership false
positive explained above.

## Negatives

1. **`fn_2_16C810` (`stepCutscene70`, 0x834 bytes) was not attempted.** It
   is by a wide margin the largest function encountered this session --
   larger than most entire units landed so far. It is a 20-state jump-table
   dispatch with heavy paired-single float math per state. Authoring it
   byte-exact is realistically a multi-round effort on its own; attempting
   a rushed first pass this round risked producing a large amount of
   low-confidence code rather than adding real value. Characterized (jump
   table location, dispatch key, general shape of a few states) but not
   transcribed.
2. **8 more functions are characterized from direct disassembly reading but
   not authored or verified against the target**: `resetInit`, `execute`,
   `createModel`, `calcModel`, `resetPosition`, `processCutsceneCommand`,
   `checkAnmLoop`, `startJump`, `checkSpawnGate`. Each has a specific,
   evidence-based role description above; none of these descriptions have
   been compiled and diffed, so treat them as informed hypotheses, not
   measurements.
3. **`resetInit`'s write to offsets `0x128-0x134`** was read BEFORE the
   layout-clarifying probe work settled that `0x128` is
   `dWmActor_c::mClipSphere`, not a new `daWmKinopio_c` member. The
   function's role description above has not been reconciled with that
   correction -- flagged explicitly rather than silently left inconsistent.
4. **`execute()`'s apparent call to `resetInit()` on every frame** (read
   directly from the disassembly, unconditional at the tail of
   `fn_2_16C2F0`) looks semantically wrong for a per-frame `execute()` --
   re-running full model/position initialization every frame would be
   unusual. Flagged as needing re-verification (address-to-name mapping
   double-check) rather than accepted at face value.
5. **The `.ctors` object (`fn_2_16D1E0`) constructs a `dWmLib::ForceInCourseList_t`
   whose exact field values** (`mNodeWorld`, `mWorld`, `mEntrance` --
   compile-time constants read directly from `.data`, not yet cross-checked
   against `game_constants.h`'s `WORLD_*` enum for the exact symbolic
   values) are read as raw integers (`6`, `6`, `4`) from the target's `.data`
   dump but not yet mapped to named constants.
6. **Positive, confirmed**: the redundant explicit null check before
   `delete mpMdlMng;` in the dtor (`if (mpMdlMng) { delete mpMdlMng; }`,
   even though `delete` on a null pointer is already a no-op) is
   load-bearing -- removing it drops one instruction and breaks the match.
   Recorded as a reusable pattern: MWCC does not eliminate a
   provably-redundant null check that appears explicitly in the source,
   even immediately before an operation that performs the identical check
   itself.

## Tools used

`tools/relfile.py`'s `Rel` class (module-2 self-relocations) settled: the
`classInit` self-relocation for the profile-ownership override, the
`.data`-internal relocation proving `sc_ForceList`-shape ownership for
`fn_2_16D1E0`'s object, and the exact `.text`/`.data`/`.rodata`/`.bss`
reference ranges used to derive the proposed slice. `STATIC_ASSERT`/
`Probe<N>` compiles against the real, already-declared
`dWmDemoActor_c`/`dBaseActor_c`/`dWmActor_c`/`daWmKinopio_c` headers settled
the entire class layout question in two short probe files
(`probe.cpp`/`probe2.cpp`/`probe3.cpp` in this directory) before any
function body was written, per "classInit's `li r3,<size>` is a free
`sizeof` check -- use it before authoring anything else."

## Round 2: authoring the twelve characterized functions

**Result: 13/19 byte-identical**, up from 7/19. `create`, `execute`,
`createModel`, `calcModel` all reached MATCH this round in addition to the
7 already landed. `resetPosition` and `checkAnmLoop` are very close (3 and
34 differing respectively, the latter's count inflated by a `verify_anon`
mispairing -- see below). `fn_2_16D1E0`/`fn_2_16D270` (the `.ctors` pair)
remain open, `fn_2_16C5E0` (processCutsceneCommand) and `fn_2_16D100`
(startJump) were not attempted, and `fn_2_16C810` was correctly left
alone per instruction.

### Both flagged inconsistencies resolved -- and they were the SAME bug

1. **`0x128-0x134` is `mClipSphere.set(mPos, 100.0f)`, not a new member.**
   `dWmActor_c::mClipSphere` (already declared in `d_wm_actor.hpp`) sits at
   exactly `0x128`, confirmed by the earlier `Probe` compile. The four
   stores (`mPos.x/y/z` into `mClipSphere`'s embedded `mCenter`, then a
   rodata constant into `mRadius`) are exactly `mSphere_c::set(const
   mVec3_c&, float)`'s shape -- matches
   `d_a_wm_kinoko_base.cpp`'s own `mClipSphere.set(mPos, 120.0f);`
   precedent line for line, just a different radius (`100.0f`).
2. **The "unconditional per-frame `resetInit()` call" was a genuine
   misread**, exactly as suspected: re-reading the raw bytes at
   `0x0016C374` showed `bl fn_2_16C490` (`calcModel`), not `fn_2_16C270`.
   `execute()` is `checkAnmLoop(); calcModel();` at its tail, not
   `checkAnmLoop(); resetInit();` -- which also resolves the "why does
   `execute` re-run full init every frame" oddity, because it doesn't.

Both turned out to trace back to one earlier mistake: the `0x128` reading
was made before the layout-probe work, and the mis-transcribed call
target was never re-checked against it. Re-reading target bytes directly,
rather than trusting an earlier note, fixed both.

### `create()` -- the odd `li r3,0x1` was a real, structural clue

`fn_2_16C270` sets `r3 = SUCCEEDED` mid-function (scheduled early, held
across the `mScale` stores, never re-loaded before `blr`) -- the same
"free information in a return value that never gets an explicit `return`
near the end" shape seen elsewhere this session. This settled its real
identity: it is not a bespoke `resetInit()` helper, it **is** the actual
`virtual int create()` override (`dWmDemoActor_c::create()` is itself
already declared, `return SUCCEEDED;`). Renamed accordingly.

### A new, reusable finding: dPyMdlBase_c's real vtable is offset by +2 from a naive declaration-order count

Two virtual dispatches through `mpMdlMng->mpMdl` (`resetPosition`'s
`setBodyAnm`-shaped call at target slot `0x5c`, `checkAnmLoop`'s
`setRate`-shaped call at target slot `0x84`) both compiled, on the first
attempt (hand-counting `include/game/bases/d_player_model_base.hpp`'s
declaration order from the destructor at slot 0), to a slot **exactly 8
bytes (2 slots) higher** than the target. The discrepancy was identical
in both cases, which is what made it diagnosable rather than two
unrelated bugs: `dPyMdlBase_c`'s compiled vtable reserves 2 slots for the
destructor (the usual scalar/vector-deleting-destructor pair on this
ABI), not the 1 a naive count assumes. Correcting for the +2 offset
pointed at the right methods by name (`setAnm(int,float,float,float)` for
slot `0x5c`, `setFrame(float)` for slot `0x84`) rather than at a
raw-offset function-pointer call -- both compiled to the target's exact
slot number once identified. Recorded because this is a general property
of `dPyMdlBase_c`'s vtable, not specific to either call site, and will
recur for any future virtual dispatch through `mpMdl`.

### A new negative: including a shared header can pull in an unrelated unit's static initializer

`dWmLib::ForceInCourseList_t` and `dWmLib::IsSingleEntry()` both live in
`include/game/bases/d_wm_lib.hpp` -- but that header ALSO declares, at
namespace scope, `static ForceInCourseList_t sc_ForceList[] = {...}`
(castle's own entry) with a dynamic initializer
(`dCsvData_c::c_CASTLE_ID` is not a compile-time constant). Because that
initializer has side effects (`__register_global_object`), the compiler
cannot prove it dead even when this TU never references it, and it gets
silently included in the draft's own `__sinit`. Confirmed directly:
compiling with the real header produces an `__sinit` that does BOTH
`sc_ForceList`'s registration and this unit's own, back to back, doubling
the function's size versus target. **Tried the fix (hand-declaring a
local mirror of `ForceInCourseList_t` and `IsSingleEntry()` instead of
including the header) and it made the specific function WORSE, not
better**: without the real header present, MWCC did not generate the
`__register_global_object` call at ALL for this unit's own object --
apparently something else in the header (not yet identified) is needed
for the compiler to recognize `daWmKinopio_c`'s own `sForceList` as
needing dynamic registration, not just its non-constant initializer.
Reverted to the real include, which at least gets `fn_2_16D270` (the
generated `__arraydtor` callback) to MATCH, and left `fn_2_16D1E0` open
rather than guess further. This needs a proper investigation next round,
not a repeat of either tried variant.

### `resetPosition`'s remaining 3 differing: a rodata constant-pooling order question, not yet solved

Four `.rodata` constants (`-500.0f`, `5.0f`, `0.5f`, `0.8f`, at target
addresses `0x8b48/0x8b4c/0x8b50/0x8b54` -- wait, confirmed via direct
read as `0x8b10+0x38/0x3c/0x40/0x44`) are shared between `resetPosition`
and `calcModel`. Declaring them as named `static const float` constants
in the target's own memory order did not reproduce the target's pooling
order (the compiler still placed them by first-use-across-the-TU order,
not declaration order) -- the same open question flagged on WM_ITEM's
`__sinit`/`cycleAnm`, now seen a second time on a different unit,
reinforcing that it is a genuine `-O4`-family MWCC constant-pool
scheduling behavior rather than a per-unit fluke.

## Updated result table

| target | size | draft | note |
|---|---|---|---|
| `fn_2_16C150` classInit | 0x30 | **MATCH** | |
| `fn_2_16C180` ctor | 0x3C | **MATCH** | |
| `fn_2_16C1C0` dtor | 0xB0 | **MATCH** | |
| `fn_2_16C270` create | 0x78 | **MATCH** | renamed from `resetInit`; real `virtual int create()` override |
| `fn_2_16C2F0` execute | 0xA4 | **MATCH** | tail calls `checkAnmLoop(); calcModel();`, not `resetInit()` |
| `fn_2_16C3A0` draw | 0x40 | **MATCH** | |
| `fn_2_16C3E0` doDelete | 0x8 | **MATCH** | |
| `fn_2_16C3F0` createModel | 0xA0 | **MATCH** | |
| `fn_2_16C490` calcModel | 0xA0 | **MATCH** | |
| `fn_2_16C530` resetPosition | 0x90 | 3 differing | rodata pooling order only |
| `fn_2_16C5C0` resetStep | 0xC | **MATCH** | |
| `fn_2_16C5D0` unusedStub | 0x4 | **MATCH** | |
| `fn_2_16C5E0` processCutsceneCommand | 0x230 | not attempted | |
| `fn_2_16C810` stepCutscene70 | 0x834 | **left alone** | per instruction |
| `fn_2_16D050` checkAnmLoop | 0xB0 | 34 differing (self-paired) | evaluation-order/register-holding shape not yet matched; vtable slot now correct |
| `fn_2_16D100` startJump | 0x84 | not attempted | uncertain struct-typed 2nd parameter, needs the (unauthored) caller in `fn_2_16C810` to pin down |
| `fn_2_16D190` checkSpawnGate | 0x4C | **MATCH** | |
| `fn_2_16D1E0` `.ctors` init | 0x84 | 32 differing | see negative above |
| `fn_2_16D270` `.ctors` callback | 0x1C | **MATCH** | auto-generated `__arraydtor` from the `sForceList` static |

**13/19 byte-identical this round (up from 7/19).**

## Round 3: the .ctors investigation (unresolved) plus re-verification

**Score unchanged at 13/19** -- the .ctors fix did not land, so `resetPosition`
and `checkAnmLoop` were re-measured but not improved.

### `.ctors` entry count confirmed as a real, decisive signal -- but the fix did not reproduce

The coordinator's diagnosis is accepted and evidenced: this unit's target
`.ctors` section has exactly **one** entry, so the real source cannot
include `d_wm_lib.hpp` (whose own `sc_ForceList` static would add a
second). Three variants of "declare only what's used" were tried this
round, all keeping the same 7-field `dWmLib::ForceInCourseList_t` layout
and a bare `namespace dWmLib { bool IsSingleEntry(); }` declaration
instead of the header:

1. Plain hand-declared struct (no destructor stated explicitly).
2. Same, with an explicit empty `~ForceInCourseList_t() {}`.
3. (implicitly tested via 1/2) struct nested inside the anonymous namespace
   alongside `sForceList` vs. at `namespace dWmLib` scope.

**All three produce the same result**: MWCC stops emitting
`__register_global_object`/the guarded dynamic-init shape entirely for
`sForceList` -- it becomes a bare, unguarded sequence of stores, and
`fn_2_16D270` (the array-destructor callback) never gets generated at all.
This is not simply "the compiler doesn't see a non-trivial destructor" --
`mVec3_c` has a user-declared (if empty-bodied) destructor either way,
which is what makes a class non-trivially-destructible by the standard's
rules, and that fact is identical whether the struct comes from the real
header or the hand-rolled mirror. Whatever triggers MWCC to route a static
object through `__register_global_object` instead of a bare store was not
identified this round.

**Reverted to including `d_wm_lib.hpp`** (the state that scored 13/19):
it reproduces `fn_2_16D270` exactly, at the cost of `fn_2_16D1E0` doing
`sc_ForceList`'s registration work in addition to its own (net: one
function still open, versus zero MATCH-worthy functions in any of the
three no-include variants). This is recorded as an open, unresolved
question for the next round -- not a dead end to retry with the same
three shapes again, since none of them worked.

### `resetPosition` (3) and `checkAnmLoop` (34) re-measured, unchanged

Per instruction, both were re-checked (their neighbours `create`,
`execute`, `createModel`, `calcModel` all newly fixed this session) in
case either would move on its own. Neither did -- confirmed independent
of the `.ctors` question and of each other. `resetPosition`'s 3
(6 raw `difftool.py` lines, one being a benign symbol name) is entirely
the `.rodata`-pooling-order question already characterized: the shared
constants `-500.0f`/`5.0f`/`0.5f`/`0.8f` pool in FUNCTION-DEFINITION order
(`calcModel`'s `0.5f`/`0.8f` first, since `calcModel` is defined earlier
in the file than `resetPosition`), not in the target's real memory order
(`-500.0f`/`5.0f`/`0.5f`/`0.8f`). Reordering the two functions themselves
would fix the pool order but break `.text` definition-order matching
(both are strong, address-fixed symbols) -- so this is a real conflict
between two ordering constraints, not a simple oversight. `checkAnmLoop`
is unchanged at 34/35: the vtable slot is correct
(`setFrame(float)` at target slot `0x84`, confirmed via the `dPyMdlBase_c`
+2-slot rule), but the source's exact operand-evaluation/register-holding
shape (target holds the `mFrameMax`-derived value in the non-volatile
`f31` across the `getFrame()` call, needing a `stfd f31`/`psq_st f31`
prologue our current phrasing does not produce) has not been matched.

## Final result this round: 13/19, unchanged from round 2

| target | size | draft | note |
|---|---|---|---|
| classInit, ctor, dtor, create, execute, draw, doDelete, createModel, calcModel, resetStep, unusedStub, checkSpawnGate | — | **MATCH** (11 functions) | |
| `fn_2_16D270` `.ctors` callback | 0x1C | **MATCH** | auto-generated, contingent on keeping the `d_wm_lib.hpp` include |
| `fn_2_16C530` resetPosition | 0x90 | 3 differing | rodata pooling-order conflict with `.text` ordering, see above |
| `fn_2_16D050` checkAnmLoop | 0xB0 | 34 differing | vtable slot correct; `f31`-holding evaluation shape not yet matched |
| `fn_2_16D1E0` `.ctors` init | 0x84 | 32 differing | carries `sc_ForceList`'s extra work; three no-include variants tried and all worse (lose the registration mechanism entirely) |
| `fn_2_16C5E0` processCutsceneCommand, `fn_2_16D100` startJump | — | not attempted |
| `fn_2_16C810` stepCutscene70 (0x834) | — | left alone, per instruction |

**13/19 byte-identical.**

## Round 4: both coordinator-suggested mechanisms retried, both negative

**Score unchanged at 13/19.** All three suggested levers were retried
carefully rather than assumed; none reproduced this round's residuals.

### `.ctors` mirror retried with the constructor-call theory -- ruled out for this unit

The coordinator's hypothesis: `sc_ForceList`'s `mVec3_c` field is
initialised via a constructor call (`mVec3_c(2160.0f, -30.0f, -478.0f)`),
not brace-init, and that dynamic initialisation is what forces
`__register_global_object`. Checked: **this unit's `sForceList` mirror
already used the constructor-call form in every earlier attempt** (visible
in the round-3 diff transcripts). Retried once more explicitly to be sure
-- identical result: no `__register_global_object`, no `fn_2_16D270`, byte
-for-byte the same broken `__sinit` as every prior no-include variant. So
brace-vs-constructor-init is not the discriminator for *this* unit's
include-vs-no-include gap specifically, even though it may be the right
explanation for the castle case that prompted the theory. Reverted to
including `d_wm_lib.hpp` (13/19's actual state) per the coordinator's own
fallback instruction.

### `resetPosition`'s pooling order: named-constant positioning retried, twice, no change

Two variants of "pool the constants as named objects instead of anonymous
literals" were tried:
1. All four constants as separate named `static const float`s (this was
   already the round-2/3 state).
2. Only the one truly-shared constant (`0x40`/`0.5f`, used by both
   `calcModel()` and `resetPosition()`) named; the other three (each used
   by only one function) left as plain literals.

**Both produce byte-identical output** to each other and to the
already-measured 3-differing state: `lfs f1` lands at `0x44` (want
`0x3c`), `lfs f0` at `0x40` (want `0x38`), `lfs f3` at `0x38` (want
`0x40`) -- the same three-way rotation every time, regardless of which
constants are named or how many. This means declaration-point pooling
either does not apply the way it did for `course`/`antlion_mng`, or the
right positioning was not found -- isolating the shared constant alone
did not surface it. Kept the single-shared-constant version (marginally
cleaner, functionally identical) rather than reverting.

### `checkAnmLoop`'s `f31`-across-a-call shape: one attempt, no change

Bound `mpMdlMng->getLastFrame()`'s result to an explicit local `float
lastFrame` before the `==` comparison, to test whether forcing early
evaluation would persuade the compiler to hold it in a non-volatile
register across the `getFrame()` call the way the target does (`stfd
f31`/`psq_st f31` in the prologue). **No change whatsoever** -- identical
instruction sequence, same missing prologue shape. Reverted to the
simpler expression form (no local) since it made no measured difference
either way.

## Final state, this session: 13/19

No change in score from round 3. Every negative this round is a genuine,
specific, single-variable retry of a named coordinator hypothesis, not a
repeat of an already-ruled-out shape -- and each produced a clean,
reproducible non-result rather than an ambiguous one. Remaining open
functions: `fn_2_16D1E0` (32 differing, `.ctors` double-init), `fn_2_16D050`
checkAnmLoop (34 differing, register-preservation shape), `fn_2_16C530`
resetPosition (6 raw diff lines / 3 counted, rodata rotation),
`fn_2_16C5E0` processCutsceneCommand and `fn_2_16D100` startJump (not
attempted), `fn_2_16C810` stepCutscene70 (0x834, left alone throughout).

## Round 5: the two unattempted functions

**Result: 14/19, up from 13/19.** `startJump` (`fn_2_16D100`) reached
**MATCH** on the first attempt. `processCutsceneCommand` (`fn_2_16C5E0`)
is now real (structurally authored, dispatch shape confirmed correct)
but not byte-exact -- 136 differing, arithmetic/vectorization residual
only, no logic errors identified.

### `startJump` -- MATCH, first attempt

Its true signature (`const char *nodeName, const JumpParam_t *param`) was
settled from the raw field offsets alone (`+0x4` float, `+0x8` s16,
`+0xc`/`+0x10` floats), without visibility into its caller
(`fn_2_16C810`, still unauthored) -- a local `JumpParam_t` struct
declared in the header captures the shape. `daWmMap_c::GetNodePos(const
char*, mVec3_c&)` and `dWmDemoActor_c::_initDemoJumpBase(...)` are both
already declared in real headers; nothing needed to be guessed there.

### `processCutsceneCommand` -- dispatch shape confirmed, arithmetic open

Read the raw bytes fresh rather than trusting the round-1 characterization,
per the coordinator's repeated "re-read, don't reuse" instruction. Control
flow, fully confirmed against the disassembly:

```cpp
void daWmKinopio_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) return;
    if (isFirstFrame && cutsceneCommandId == 0x70) {
        mSpeedF = 0.8f;
        setDirection(mVec3_c(-500.0f, 0.5f, 0.5f));
        if (!checkSpawnGate()) { /* m_19c = GetPos("W103")*0 + GetPos("W102")*5 */ }
        else                   { /* m_19c = GetPos("W103")*0 + GetPos("W102")*8 */ }
        m_1a8 = 0; m_198 = 0xf;
    }
    if (cutsceneCommandId == 0x70) stepCutscene70();
    else                           mIsCutEnd = true;
}
```
This is a single if-chain (not a switch): one `cmpwi -1`+`beq`, folded
`isFirstFrame && cmd==0x70` (two more compare-and-skip pairs sharing one
body), an inner if/else on `checkSpawnGate()`, then a final if/else on
`cmd==0x70` at the tail -- matches the target's branch count and skip
targets exactly, confirmed instruction-by-instruction against the raw
bytes (not assumed from the round-1 read).

`daWmMap_c::GetPos(const char*)` is not in the real header (only
`GetPos(int)` is) -- declared as `extern "C" mVec3_c
GetPos__9daWmMap_cFPCc(daWmMap_c*, const char*)`, the project's usual
raw-extern technique, using the mangled name read directly off the
target.

**What's open (136 differing, size 130 vs target 140)**: the target
computes both branches' weighted-position math using paired-single
(`ps_muls0`/`ps_add`, `psq_l`/`psq_st`) instructions and holds `f30`/`f31`
across both `GetPos()` calls (needing the `_savegpr_27`/`stfd
f31`/`psq_st f31` prologue shape); the current draft's `mVec3_c operator*`
/`operator+` expression compiles to plain scalar `fmuls`/`fadds` instead.
This is a vectorization-level difference, not a logic error -- no branch
target, constant value, or call argument is wrong, only the FP instruction
selection. Not chased further this round; likely needs a source shape
that visibly hints paired-single treatment (unclear what that would look
like in plain C++ without intrinsics), a genuinely open question for next
round rather than a quick fix.

### `fn_2_16C810` groundwork: jump table entry count confirmed by counting relocations

Per the coordinator's instruction, counted (not guessed) the case count
before touching the function: relocations in `.data 0x45d14-0x45d68`
(`jumptable_2_data_45D14`) give exactly **20 entries** (`0x45d14`
through `0x45d60`, 4 bytes apart, matching the table's own dtk-reported
size `0x50` exactly and the function's own `cmplwi r0,0x13` bound check).
Case targets: `0x16c8a0, 0x16c974, 0x16ca0c, 0x16d024, 0x16ca48, 0x16ca74,
0x16cb28, 0x16cb90, 0x16cbc4, 0x16cc28, 0x16cc3c, 0x16cd08, 0x16cd80,
0x16ce24, 0x16ce5c, 0x16cf08, 0x16cf2c, 0x16cf4c, 0x16cf60, 0x16d010` --
this gives the exact per-case byte ranges for ascending-size authoring
next round, without needing to re-derive them. Not authored this round --
ran out of time after landing the two previously-unattempted functions,
which the coordinator correctly identified as higher-value than a fourth
retry on the three known walls.

## Final result this session: 14/19

| target | size | draft | note |
|---|---|---|---|
| classInit, ctor, dtor, create, execute, draw, doDelete, createModel, calcModel, resetStep, unusedStub, checkSpawnGate | — | **MATCH** (11) | |
| `fn_2_16D270` `.ctors` callback | 0x1C | **MATCH** | |
| `fn_2_16D100` **startJump** | 0x84 | **MATCH** | new this round |
| `fn_2_16C530` resetPosition | 0x90 | 3 differing | walled, multiple negatives, left alone |
| `fn_2_16D050` checkAnmLoop | 0xB0 | 34 differing | walled, multiple negatives, left alone |
| `fn_2_16D1E0` `.ctors` init | 0x84 | 32 differing | walled, multiple negatives, left alone |
| `fn_2_16C5E0` processCutsceneCommand | 0x230 | 136 differing | new this round -- dispatch shape confirmed correct, vectorization residual open |
| `fn_2_16C810` stepCutscene70 | 0x834 | not attempted | jump table counted (20 entries), case addresses catalogued, not authored |

**14/19 byte-identical.**

## Round 6: fn_2_16C810, authored in pieces (9 of 20 cases fully decoded)

**X/19 unchanged at 14/19** (the giant function needs all 20 cases right
to reach MATCH, and 11 remain placeholders), but `fn_2_16C810` is now a
real, compiling, correctly-dispatching `switch` rather than an unattempted
0x834-byte block. Preamble (`calcSpeed()`, `posMove()`, the footstep-sound
check, the `cmplwi 0x13`/`bgt`-guarded jump-table dispatch) matches the
target's shape closely, with only a minor register-allocation difference
in the footstep-sound call's argument evaluation order left unresolved.

### 9 of 20 cases fully decoded and authored

| case | target addr | size | body |
|---|---|---|---|
| 3 | 0x16d024 | 0 (shares the function's own epilogue) | `break;` -- the jump table entry points straight at the shared exit, no unique code exists |
| 9 | 0x16cc28 | 0x14 | `m_198 = 0x1e; m_1a8 = 0xa;` |
| 19 | 0x16d010 | 0x14 | `setCutEnd();` -- dispatched through the class's own vtable at slot `0x68`, 2 slots after `processCutsceneCommand`'s confirmed slot `0x60`; a plain call reproduced it exactly the same way `execute()`'s `processCutsceneCommand(...)` call already had (no special vtable-pointer code needed) |
| 17 | 0x16cf4c | 0x14 | `fn_2_192920(m_1b8); m_1a8 = 0x12;` |
| 16 | 0x16cf2c | 0x20 | `if (*(int*)((u8*)dWCamera_c::m_instance + 0x5f4) == 0) m_1a8 = 0x13;` |
| 15 | 0x16cf08 | 0x24 | countdown-timer pattern: `if (m_198>0) m_198--; else m_1a8=0x13;` |
| 4 | 0x16ca48 | 0x2c | same countdown pattern, transitions to state 5 |
| 7 | 0x16cb90 | 0x34 | same countdown pattern, transitions to state 8, also resets `m_1ac` |
| 13 | 0x16ce24 | 0x38 | camera check wrapping the countdown pattern, transitions to state 0xe |

The countdown-timer shape (`if (m_198 > 0) m_198--; else { ...transition... }`)
recurs across at least 4 of the 9 decoded cases -- clearly the dominant
per-state idiom in this machine, which should make the remaining 11 cases
faster to read next round now that the pattern is established.

### 11 of 20 cases NOT decoded -- placeholder `break;` bodies (one exception)

Cases 0, 1, 2, 5, 6, 8, 10, 11, 12, 18 are `break;` stubs (case 2 got a
partial read -- see below); case 14 got a one-line partial (its first
call only). None of these were fully transcribed, so none should be read
as verified. Two negatives worth recording precisely:

- **Case 2** was partially read: `if (m_1ac <= 1) break;` then writes
  `true`/`true`/`0xd` into fields `+0x544`/`+0x546`/`+0x55c` of a pointer
  loaded from `lbl_2_bss_11B70` (a shared `.bss` slot, not one of this
  unit's own fields), then sets `m_198 = 0xb4`, `m_1a8 = 4`. The pointed-to
  object's real type was not identified in the time available -- accessed
  via raw byte-offset casts (`*(bool*)(mgr+0x544)` etc.) rather than a
  named type, which is a real gap: those fields are almost certainly a
  `daWmMap_c`-family or effects-manager object with real accessors this
  unit should be using by name.
- **Case 14** begins with `bl InitKinopioCourse__6dWmLibFv` --
  `dWmLib::InitKinopioCourse()`, declared via a bare forward declaration
  (not in the real header) -- but the rest of the case body (its actual
  size, 0x38 bytes, suggests more happens after that call) was not read.

### New reusable technique confirmed: a plain virtual call reproduces a "this+0x60"-shaped dispatch automatically

Case 19's `setCutEnd()` call compiled to the exact same
`lwz r12,0x60(this); lwz r12,0x68(r12); mtctr; bctrl`-shaped dispatch the
target uses, from an ordinary (non-special) virtual method call -- no
manual vtable-pointer code needed, confirming (a second time, after
`execute()`'s `processCutsceneCommand(...)` call) that what looked like a
"secondary vtable" convention is simply this class hierarchy's own
regular vtable layout at a non-zero base offset, handled entirely by the
compiler for any ordinary virtual call written normally in C++.

## Final state, this session: 14/19

`fn_2_16C810` moved from "not attempted" to "9/20 cases decoded and
authored, structurally correct switch/dispatch, 11 cases stubbed" -- real,
measured progress on the unit's last function, even though it does not
close this round. `resetPosition` (3), `checkAnmLoop` (34), the `.ctors`
init (32), and `processCutsceneCommand` (136) were left untouched this
round per instruction.

## Round 7: 3 more cases closed (12/20 total), lbl_2_bss_11B70 investigated and unresolved

**X/19 unchanged at 14/19** (the giant function still needs every case
right). Cases decoded and authored this round: **6, 8, 11** -- bringing
the total to **12 of 20 cases fully authored** (3, 4, 6, 7, 8, 9, 11, 13,
15, 16, 17, 19). Cases 0, 1, 5, 10, 12, 18 remain untouched stubs; case 2
is a complete byte-cast implementation (not a stub, but not resolved to a
named type); case 14 is a genuine partial (one confirmed call, explicitly
labeled, not claiming completeness).

### `lbl_2_bss_11B70`: identification attempted, not resolved -- genuine negative

Ran the ownership check per instruction: **over 130 relocation references**
to this `.bss` slot, spanning almost the entire module (`0x18d000` through
`0x1c1000` in `.text`) -- confirming it's a widely-shared singleton
pointer, not something local to this unit. Checked three specific
candidates by grepping `include/` and comparing field layouts against the
confirmed offsets (`+0x544`, `+0x545`, `+0x546`, `+0x54d` -- four
booleans -- and `+0x55c`, an int/enum):
- `dCsSeqMng_c` (already used elsewhere in this same unit) -- its
  documented fields end at `0x1b4`, far short of `0x544`.
- `dWmEffectManager_c` -- no data members at all beyond the static
  instance pointer.
- `dGameKey_c` -- wrong shape (a 4-element pointer array, not a flags
  struct).

Also grepped all of `include/` for literal `0x544`/`0x545`/`0x546`/`0x55c`
in comments -- the five hits are all unrelated enemy-actor base classes.
**No match found.** Per the established handling (confirmed by the
coordinator as used by two already-landed units), kept the raw
`u8*`/offset-cast form, documented with every offset found across cases 2,
6, and 8 (`+0x544`, `+0x545`, `+0x546`, `+0x54d` all booleans; `+0x55c` an
int, values `4`/`7`/`0xd`/`0xe` observed). This is a real, not a lazy,
negative -- the type search was run before falling back to casts, not
instead of it.

### 3 newly decoded cases

- **Case 6** (`0x16cb28`, 0x68): guarded by `lbl_2_bss_11B70+0x54d == 0`;
  sets `m_198 = 0xb4`, calls `mpMdlMng->mpMdl->setAnm(0, -500.0f, 0.25f,
  0.5f)` (vtable slot `0x5c`, confirmed the same slot as `resetPosition`'s
  `setAnm` call from round 2's `dPyMdlBase_c` +2-slot rule), sets three
  more `lbl_2_bss_11B70` fields, transitions to state 7.
- **Case 8** (`0x16cbc4`, 0x64): another countdown-to-zero, then on
  expiry checks **live controller input** --
  `dGameKey_c::m_instance->mRemocon[mPad::g_currentCoreID]->mDownButtons
  & 0x900` -- both `dGameKey_c` and `mPad::g_currentCoreID` were already
  declared in `include/`, including a `checkButtonsDown()` helper whose
  body proved `mRemocon[i]->mDownButtons` was the right access shape (the
  raw offset `+0x1c` in the disassembly matches `Remocon::mDownButtons`'s
  documented offset exactly). Sets one `lbl_2_bss_11B70` flag,
  transitions to state 9.
- **Case 11** (`0x16cd08`, 0x78): guarded by `mPos.x < -500.0f`; calls
  `dWmDemoActor_c::clearSpeedAll()` (already declared) and
  `dWmEffectManager_c::m_pInstance->endEffect(m_1b0)` -- which
  **identifies `m_1b0`** (previously "unobserved") as an effect-ID `int`,
  updated in the header. Then the same `setAnm(0, -500.0f, 0.5f, 0.5f)`
  shape, then branches on `checkSpawnGate()` to state `0xe` or `0x13`.

### Progress this round

12/20 cases now authored (up from 9/20). `m_1b0`'s role resolved (effect
ID for `dWmEffectManager_c::endEffect`). `lbl_2_bss_11B70`'s type search
was genuine and exhaustive within the time available but did not
converge -- recorded as a real negative per instruction, not silently
left as an unexamined assumption.

## Final state, this session: 14/19

12 of 20 `stepCutscene70` cases closed; `resetPosition` (3), `checkAnmLoop`
(34), the `.ctors` init (32), and `processCutsceneCommand` (136) untouched
this round per instruction. Remaining `stepCutscene70` work: cases 0, 1,
5, 10, 12, 18 (untouched), case 14 (partial, one confirmed call out of an
estimated 4-5 needed), case 2 (complete but using unidentified-type byte
casts).

## Round 8: critical constant-addressing bug caught and fixed, 2 more cases closed (14/20 total)

**X/19 unchanged at 14/19.** Cases decoded and authored this round: **5,
18**. Total now **14 of 20 cases authored** (3, 4, 5, 6, 7, 8, 9, 11, 13,
15, 16, 17, 18, 19). Cases 0, 1, 10, 12 remain untouched; case 14 is still
the same one-call partial from round 7; case 2 unchanged.

### A real regression caught before it shipped: displacement-vs-absolute-address bug in cases 6 and 11

Per the coordinator's caution ("a variant that improves the count while
shifting data offsets is a regression"), re-derived every `.rodata`
constant used this round from first principles instead of reusing a
mental shortcut, and caught a genuine bug in the round-7 code: `r31`
(and `r29`/`r30` in other functions) holds `lbl_2_rodata_8B10`'s address
(`0x8b10`) for the whole function, so `lfs fN, 0xNN(r31)` reads absolute
address `0x8b10 + 0xNN` -- **not** the literal bytes `0x8bNN`. Round 7's
`setAnm(...)` calls in cases 6 and 11 had used the raw displacement digits
as if they were the absolute address directly (e.g. displacement `0x48`
read as address `0x8b48` instead of the correct `0x8b10+0x48=0x8b58`),
giving completely wrong constants in both calls. Caught by rebuilding a
full displacement-indexed table (`0x0` through `0x8c`) and cross-checking
every earlier use against it -- found the two `setAnm` calls did not match
any consistent pattern with the guard-check constants in the same cases
(which *were* right, by coincidence of an earlier correctly-computed
value being reused). Fixed:
- Case 6: `setAnm(0, -500.0f, 0.25f, 0.5f)` (wrong) -> `setAnm(0, 1.0f,
  20.0f, 0.0f)` (correct: displacements `0x48`/`0x60`/`0x40` ->
  addresses `0x8b58`/`0x8b70`/`0x8b50`).
- Case 11: `setAnm(0, -500.0f, 0.5f, 0.5f)` (wrong) -> `setAnm(0, 1.0f,
  0.0f, 0.0f)` (correct: displacements `0x48`/`0x40`/`0x40`).

Neither case was reported as matching before or after the fix (both were
already counted as "authored, not verified" negatives), so this was not
a case of a false positive shipping -- but it would have become one had
these cases' surrounding code ever gotten close enough to expose the
wrong data via a differing-count comparison instead of an outright logic
read. Recorded as a real, caught negative per the coordinator's standing
instruction to read the diff rather than trust an improving count.

### 2 more cases decoded and authored

- **Case 5** (`0x16ca74`, 0xb4): countdown-to-zero, then on expiry checks
  live controller input (same `dGameKey_c`/`mPad::g_currentCoreID` shape
  as case 8), then -- win or lose an apparent redundant `if
  (dWmLib::IsSingleEntry())` check whose two branches compile to
  **identical** code (both read `lbl_2_bss_11B70+0x538` as another
  pointer, and if non-null set fields `+0x251`/`+0x254` on it) -- written
  as literal duplicate `if`/`else` bodies to match the target's own
  un-merged duplication, then sets one more `lbl_2_bss_11B70` flag and
  transitions to state 6.
- **Case 18** (`0x16cf60`, 0xb0): a "wait for the child actor to catch up,
  then play a sound and follow it" case -- `if (m_1b8->mPos.x <=
  mPos.x)`, `dWmSeManager_c::m_pInstance->playSound(0x67, mPos, 1)`
  (already declared, its 3-arg overload matched the mangled name
  exactly), a `setAnm` call and a position update once triggered, then
  a `fn_2_192930(m_1b8)` check that hides/transitions on success --
  confirming `mVisible` (established inherited field) is written here
  too.

### Untouched / partial cases -- genuine stops, not silent gaps

- **Case 12** (`0x16cd80`, 0xa4): read in full. Involves indexing
  `daWmMap_c`'s internal `dWmMapModel_c` array via a computed stride
  (`idx * 0xbf8 + 0x1a0`), `dWmMapModel_c::GetEndNodePos`, and writes to
  **six** `dWCamera_c` fields at offsets `0x5f0`-`0x71c` -- offsets far
  beyond the header's documented `char pad[0x4f8]` placeholder, confirming
  (again) that `dWCamera_c`'s real layout is much larger than currently
  captured in `include/`. Not authored: too many simultaneously-uncertain
  pieces (the map-model stride, `GetEndNodePos`'s exact signature, and six
  undocumented camera fields) to write correctly under the time available.
- **Case 10** (`0x16cc3c`, 0xcc): read in full up to one blocking unknown
  -- a virtual dispatch through `mpMdlMng->mpMdl` at vtable slot `0x28`
  whose return value is used as an argument to a further call
  (`fn_80103520`), meaning it returns something despite the header listing
  the slot's likely name (`getBodyMdl()`) as `void`. Rather than guess a
  vtable slot number by hand (the exact failure mode the `+2`-offset
  lesson exists to prevent), left unauthored.
- **Case 1, Case 0**: not read this round -- time ran out after the
  bug-fix pass and the two closed cases; both are the two largest
  remaining (0x98 and 0xd4 bytes).
- **Case 14**: unchanged from round 7 (one confirmed call,
  `dWmLib::InitKinopioCourse()`, explicitly labeled incomplete).
- **Case 2**: unchanged, complete but still using the unidentified-type
  `lbl_2_bss_11B70` byte casts (see round 7 for the exhausted type
  search).

## Final state, this session: 14/19

**14 of 20 `stepCutscene70` cases now closed** (up from 12). Remaining:
cases 0, 1, 10, 12 untouched; case 14 partial; case 2 complete-but-typed-
as-bytes. `resetPosition` (3), `checkAnmLoop` (34), the `.ctors` init (32),
and `processCutsceneCommand` (136) untouched this round per instruction.

## Round 9: case 1 closed, case 0 confirmed blocked on the same defect as case 10 -- stocktake

**X/19 unchanged at 14/19.** Case 1 fully decoded and authored (no
blockers). Case 0 read in full and found to hit the identical blocker as
case 10 (see below) -- left as a documented stub rather than guessed.

### Case 1 (`0x16c974`, 0x98) -- clean, no blockers

```cpp
mSpeedF = mSpeedF + m_194;
if (m_198 > 0) {
    m_198 = m_198 - 1;
} else {
    mSpeedF = 0.0f;
    mpMdlMng->mpMdl->setAnm(0xab, 1.0f, 5.0f, 0.0f);
    m_198 = 0xa; m_1a8 = 2; m_1ac = 0;
    dWmEffectManager_c::m_pInstance->endEffect(m_1b0);
    dWmSeManager_c::m_pInstance->playSound(0x35, mPos, 1);
}
```

### Case 0 (`0x16c8a0`, 0xd4) -- read in full, blocked, not authored

Read completely, including the classic PowerPC int-to-float bit-trick
(`0x43300000`/`XOR 0x80000000` double construction, subtracted against a
magic bias loaded from `.rodata`) used twice to convert the integer `15`
to a float divisor. That resolved one real, previously-"unobserved"
field: **`m_194 = ((m_19c.x - mPos.x) / 15.0f) / 15.0f`** -- consumed by
case 1 (`mSpeedF += m_194`), confirming the two cases are a matched pair
(case 0 computes a per-frame speed delta from the remaining distance to
the cutscene target, case 1 applies it). Recorded in the header despite
the case itself not being authorable.

The case is otherwise blocked by the **exact same defect as case 10**:
after a `setAnm(2, 3.0f, 5.0f, 0.0f)` call, it dispatches
`mpMdlMng->mpMdl`'s vtable at slot `0x28` and uses the **return value** as
an argument to `fn_80103520(dWmEffectManager_c::m_pInstance, 2, <that
return value>, lbl_2_data_45D00, 0, 0)`, storing the overall result into
`m_1b0` (the effect ID). `include/game/bases/d_player_model_base.hpp`
declares the method at that slot `void`. Two independent occurrences
(cases 0 and 10) of the identical shape confirm this is a genuine header
defect, not a one-off misread -- left unauthored rather than hand-guess a
return type or bypass the vtable with a raw function-pointer cast.

## Final stocktake: `stepCutscene70`'s 20 cases

| status | cases | count |
|---|---|---|
| authored, no open questions | 1, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 16, 17, 18, 19 | 15 |
| authored, but reads/writes an unidentified `.bss` singleton via byte casts | 2 | 1 |
| partial (one confirmed call of an estimated four-plus) | 14 | 1 |
| blocked -- `dPyMdlBase_c` vtable slot `0x28` is `void` in `include/` but the target uses its return value | 0, 10 | 2 |
| blocked -- `dWCamera_c`'s real layout extends past the header's documented `pad[0x4f8]` (fields at `0x5f0`-`0x71c` observed) | 12 | 1 |

**16/20 cases are authored** (15 clean + case 2's caveated one); the
remaining 4 are a partial and three genuinely blocked cases, none of them
fixable without editing files outside this unit's own directory. This
function cannot reach MATCH within this unit's own scope: cases 0/10/12
need either a header correction (out of scope -- `include/` is
off-limits) or a resolved singleton identity that a from-scratch header
grep did not surface. That is the honest stopping point for
`stepCutscene70` this session.

## Final result, this session: 14/19

| target | size | draft | note |
|---|---|---|---|
| classInit, ctor, dtor, create, execute, draw, doDelete, createModel, calcModel, resetStep, unusedStub, checkSpawnGate | — | **MATCH** (11) | |
| `fn_2_16D270` `.ctors` callback | 0x1C | **MATCH** | |
| `fn_2_16D100` startJump | 0x84 | **MATCH** | |
| `fn_2_16C530` resetPosition | 0x90 | 3 differing | walled, untouched this round |
| `fn_2_16D050` checkAnmLoop | 0xB0 | 34 differing | walled, untouched this round |
| `fn_2_16D1E0` `.ctors` init | 0x84 | 32 differing | walled, untouched this round |
| `fn_2_16C5E0` processCutsceneCommand | 0x230 | 136 differing | walled, untouched this round |
| `fn_2_16C810` stepCutscene70 | 0x834 | 514 differing | **16/20 cases authored**, 1 partial, 3 blocked on real, documented defects outside this unit's editable scope |

**14/19 byte-identical.**

## Round 10: the `dPyMdlBase_c` vtable slot 0x28 defect, proven in a shadow header

Per the coordinator's instruction: copied `d_player_model_base.hpp` into
this unit's own `shadow_include/`, corrected exactly one declaration, and
authored cases 0 and 10 against it, rather than asking for the real
header to be trusted on a slot-offset argument alone.

### The change

**Method**: `dPyMdlBase_c::getBodyMdl()`.
**Vtable byte offset**: `0x28` (compiled slot 10, 0-indexed).
**Hand-declaration-order index**: 8 -- `10 - 2` for the two destructor
slots (scalar + vector-deleting), per this unit's own earlier-established
rule; index 8 in `d_player_model_base.hpp`'s declaration order lands
exactly on `getBodyMdl()`, immediately after `draw()` and before
`getAnmResFile()`.
**Real declaration** (`include/`): `virtual void getBodyMdl();`
**Shadow-header correction** (this unit's `shadow_include/game/bases/
d_player_model_base.hpp`): `virtual m3d::mdl_c *getBodyMdl();`

**What both call sites do with the value**: in `fn_2_16C810`'s cases 0 and
10, the return value is passed straight through as the 3rd argument to an
unnamed cross-TU call, `fn_80103520(dWmEffectManager_c *mgr, int
effectId, m3d::mdl_c *model, const char *kind, int, int)` -- the exact
same argument shape as the already-landed `fn_80103420` in
`d_a_wm_kinoko_base.cpp`, whose corresponding parameter is typed
`m3d::mdl_c *model`. That precedent is what fixed the return type, not a
guess from the name alone. Unlike `fn_80103420`, this one's own result is
also used (stored into `m_1b0`, the effect-ID field established in round
6), so it returns `int`, not `void`.

### Result: both cases now compile with the exact target dispatch and argument-passing shape

With the shadow-header fix, `mpMdlMng->mpMdl->getBodyMdl()` compiles to
**exactly** the target's `lwz r12,0x0(r3); lwz r12,0x28(r12); mtctr r12;
bctrl` dispatch (previously unreachable at all, since the method didn't
exist with a usable return type), and the immediately-following `mr
r5,r3` (passing the return value as `fn_80103520`'s 3rd argument) matches
target byte-for-byte in both cases. This is not a guess confirmed by
elimination -- it's a structural MATCH on the exact instructions the
defect was blocking.

**One unrelated residual remains in both cases**: the `"kinopio_all_root"`
string constant (`lbl_2_data_45D00`) pools differently -- target
establishes its own dedicated base register (`lis r6,
lbl_2_data_45D00@ha; addi r6,r6,...@l`, 2 instructions), while the draft's
copy gets folded into an existing register already live for `.rodata`
access (`addi r6, r31, <offset>`, 1 instruction) regardless of where the
string is declared (tried both inside the existing anonymous namespace
and as its own separate one -- same result both times). This is the same
class of constant/string-pooling residual already characterized
repeatedly this session (WM_ITEM's `cycleAnm`/`__sinit`, this unit's
`resetPosition`) -- a real, separate, already-understood gap, not a new
unknown and not related to the vtable fix.

### Recommendation for the real header

Change `include/game/bases/d_player_model_base.hpp`'s `virtual void
getBodyMdl();` to `virtual m3d::mdl_c *getBodyMdl();` (the file already
includes `<game/mLib/m_3d.hpp>`, so no new include is needed). Proven
against two independent call sites in this unit; recommend the standard
full five-binary verify before landing, per the coordinator's own
process.

## Case count after this round

**18 of 20 `stepCutscene70` cases now authored** (15 clean + case 2's
`.bss`-cast caveat + cases 0 and 10, both structurally proven against the
corrected vtable slot). Remaining: case 12 (blocked on `dWCamera_c`'s
real layout exceeding its header's documented padding -- a second,
unrelated header gap, not this unit's to fix) and case 14 (partial, one
confirmed call of an estimated four-plus, not blocked, just not finished
in the time available).

## Final result, this session: 14/19

| target | size | draft | note |
|---|---|---|---|
| classInit, ctor, dtor, create, execute, draw, doDelete, createModel, calcModel, resetStep, unusedStub, checkSpawnGate | — | **MATCH** (11) | |
| `fn_2_16D270` `.ctors` callback | 0x1C | **MATCH** | |
| `fn_2_16D100` startJump | 0x84 | **MATCH** | |
| `fn_2_16C530` resetPosition | 0x90 | 3 differing | walled, untouched |
| `fn_2_16D050` checkAnmLoop | 0xB0 | 34 differing | walled, untouched |
| `fn_2_16D1E0` `.ctors` init | 0x84 | 32 differing | walled, untouched |
| `fn_2_16C5E0` processCutsceneCommand | 0x230 | 136 differing | walled, untouched |
| `fn_2_16C810` stepCutscene70 | 0x834 | — | **18/20 cases authored**; case 12 blocked on a second header gap (`dWCamera_c`), case 14 partial |

**14/19 byte-identical.** The `getBodyMdl()` return-type fix is proven and
ready for the coordinator to apply to the real header; once landed, this
unit's own draft would need no further change to benefit from it (the
shadow-header override already produces the correct dispatch).

## Round 11: clean rebuild against the landed getBodyMdl fix; cases 14 and 12 both closed out (20/20 cases now have authored code)

**X/19 unchanged at 14/19** (stepCutscene70 still needs every last byte to
flip to MATCH), but this is real, verified progress: both of the two
remaining open `stepCutscene70` cases from round 10 (case 14's partial,
case 12's "blocked") are now fully authored. `stepCutscene70` moved from
493 differing... (see below for the actual before/after: 517 -> 493).

### Step 0: shadow header dropped, rebuild confirmed clean

`dPyMdlBase_c::getBodyMdl()`'s return-type fix landed for real in
`include/game/bases/d_player_model_base.hpp` (commit `c02cc69`). Diffed
this unit's shadow copy against the real header first -- only a comment
block differed, no code. Deleted
`wip/wm_units/agent_kinopio/shadow_include/game/bases/d_player_model_base.hpp`
and rebuilt. **Verified**: compile succeeds, score is unchanged at 14/19,
and the two `getBodyMdl()` call sites (cases 0 and 10) still produce the
exact same `lwz r12,0x0(r3); lwz r12,0x28(r12); mtctr r12; bctrl; ...; mr
r5,r3` dispatch byte-for-byte against target at both `0x0016C928-0x0016C950`
and `0x0016CCBC-0x0016CCE4` (checked instruction-by-instruction, not just
"it compiled").

### Case 14 fully authored -- and a wrong profile-ID guess caught by the compiler itself

Finished reading the case from `fn_16C810_full.txt` (target
`0x16ce5c`-`0x16cf08`, 0xac bytes) before writing anything, per
instruction. Full body:

```cpp
case 14: {
    dWmLib::InitKinopioCourse();
    fBase_c *balloon = fManager_c::searchBaseByProfName(fProfile::WM_KINOBALLOON, nullptr);
    if (balloon) {
        void *node = fn_80100640(daWmMap_c::m_instance, sW101, 0);
        *(u32 *) ((u8 *) balloon + 0x184) = node ? *(u32 *) ((u8 *) node + 0xc) : 0;
        mVec3_c pos = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, sW101);
        *(mVec3_c *) ((u8 *) balloon + 0xac) = pos;
    }
    fn_2_16AE70();
    fn_2_1998E0(daWmPlayer_c::ms_instance);
    m_198 = 0x3c;
    m_1a8 = 0xf;
    break;
}
```

**A genuine bug caught before it shipped**: first attempt guessed the
searched profile was `fProfile::WM_BUBBLE` from the raw immediate
(`li r3, 0x2a7`) using a manual enum count. Compiling against the REAL
enum showed `fProfile::WM_BUBBLE` actually compiles to `0x2a6`, one off --
my manual count was wrong by one somewhere in a ~680-entry list, exactly
the kind of error the "let the compiler check your literal" technique
exists to catch. `fProfile::WM_KINOBALLOON` compiles to the wanted `0x2a7`
and, better, makes narrative sense: `fn_2_16AE70` (called right after) is
independently confirmed to be `daWmKinoballoon_c::triggerFirstStartMove()`
(identical address `0x0016AE70` in `wip/wm_units/agent_kinoballoon`'s own
draft, a static method of the sibling class in this same REL, called with
no arguments in either code path here) -- so case 14 is "find a kinoballoon
actor, arm its node/position fields, then kick off its start-move state
machine." This ties the whole case together and is the kind of
cross-check that should have been done before the first guess, not after.

`fManager_c::searchBaseByProfName(ProfileName, const fBase_c*)` (the
static overload, matching the target's exact mangled name, not
`dBase_c`'s convenience wrapper) needed `<game/framework/f_manager.hpp>`
and `<game/framework/f_base.hpp>` added to the includes; `daWmPlayer_c`
already existed in `include/game/bases/d_a_wm_player.hpp` with
`ms_instance` declared. `fn_80100640` reuses the exact signature already
established in `wip/wm_units/agent_anchor/d_a_wm_anchor.cpp`
(`daWmMap_c*, const char*, int`). `fn_2_1998E0` has no name or precedent
anywhere in the tree; declared as a raw `extern "C"` taking
`daWmPlayer_c*`, same technique as `fn_2_192920`/`fn_2_192930` already in
this file.

**Two residuals remain in case 14, both the SAME already-characterized
class as elsewhere in this unit (constant/string-pool positioning)**: the
shared `"W101"` node name (now hoisted to a file-scope `sW101[]`, used by
both cases 12 and 14) compiles to `addi rN, r31, 0x64` (folded into the
existing rodata-base register) instead of the target's own dedicated
`lis/lwz` pair for `lbl_2_data_45CBC`; and the balloon's target-position
struct-copy shape differs (target keeps a genuine stack round-trip through
a second temp at `r1+0x8` that the draft's single-use local optimizes
away -- 3 fewer store instructions, not a logic difference). Not chased
further, consistent with this unit's repeated, reproducible finding that
MWCC's constant/temp placement in this specific pooling class does not
respond to declaration reordering.

### Case 12 -- reframed from "blocked" to "solved," no header change needed

Read the case fully (target `0x16cd80`-`0x16ce24`, 0xa4 bytes) before
writing anything. The previous round's "blocked" verdict assumed fixing
`dWCamera_c`'s layout would need the same kind of header correction as
`getBodyMdl()`. Checking for a landed precedent first (per the
coordinator's own standard) found something different and better:
**`source/d_basesNP/bases/d_a_wm_note.cpp`** (already landed, not `wip/`)
already writes the exact same six `dWCamera_c` offsets
(`0x5f0/0x5f4/0x604/0x608/0x624/0x71c`) in its own `processCutsceneCommand`,
using a **local raw `u8*` cast confined to that one `.cpp`** -- not a
header fix. `wip/wm_units/agent_start/d_a_wm_start.cpp` independently hit
the identical six offsets with matching types
(`u32/mVec3_c*/u32/bool/u32/const float*`). Three independent units,
including one already-landed, agreeing on the same six fields is a
materially stronger cross-check than this unit's own reading alone could
give -- and it points at a DIFFERENT accepted fix than a header edit:
**this project's standing policy for `dWCamera_c` past its documented
padding is local raw casts, not extending the shared header.** Adopted
that technique directly rather than writing a new header proposal.

The map/model indexing half of the case needed **no new header work at
all**: `daWmMap_c::mModels[4]` and `daWmMap_c::currIdx` are ALREADY
correctly declared in `include/game/bases/d_a_wm_map.hpp`, and
`dWmMapModel_c` is already `u8 mPad[0xbf8]` (matching the observed
`0xbf8` stride exactly). Verified independently:
`offsetof(daWmMap_c, mModels) == 0x1a0`, and `offsetof(daWmMap_c, currIdx)
== 0x1a0 + 4*sizeof(dWmMapModel_c) == 0x1a0 + 4*0xbf8 == 0x338c`, which is
exactly the raw offset (`lwz r0, 0x338c(r5)`) the target reads -- so
`map->mModels[map->currIdx]` needed zero raw-offset arithmetic on the
`daWmMap_c` side. The only missing piece was one method,
`dWmMapModel_c::GetEndNodePos(mVec3_c&)`, declared as a raw `extern "C"`
free function from its confirmed mangled name
(`GetEndNodePos__13dWmMapModel_cFR7mVec3_c`), same technique as
`GetPos__9daWmMap_cFPCc` already in this file.

Authored body:

```cpp
case 12: {
    daWmMap_c *map = daWmMap_c::m_instance;
    dWCamera_c *camera = dWCamera_c::m_instance;
    mVec3_c endPos;
    GetEndNodePos__13dWmMapModel_cFR7mVec3_c(&map->mModels[map->currIdx], endPos);
    m_19c = GetPos__9daWmMap_cFPCc(daWmMap_c::m_instance, sW101);
    m_19c.y = endPos.y;
    m_19c.z = endPos.z;
    u8 *cam = (u8 *) camera;
    *(u32 *) (cam + 0x604) = 2;
    *(mVec3_c **) (cam + 0x5f4) = &m_19c;
    *(u32 *) (cam + 0x5f0) = 0;
    *(bool *) (cam + 0x624) = false;
    *(u32 *) (cam + 0x608) = 0;
    *(const float **) (cam + 0x71c) = sCamParams;
    m_198 = 0x3c;
    m_1a8 = 0xd;
    break;
}
```

The `m_19c = pos; m_19c.y = endPos.y; m_19c.z = endPos.z;` shape (full
3-float assign, then two of the three components immediately overwritten)
is read directly off the instruction order, not inferred: target calls
`GetEndNodePos` FIRST (into a stack temp), then `GetPos` SECOND, then
stores all three of `GetPos`'s components into `m_19c`, then re-stores
`GetEndNodePos`'s already-computed `.y`/`.z` (never its `.x`) over the top
-- matching exactly.

`sCamParams[4] = {0.1f, 12.0f, 1.0f, 100.0f}` -- the first three floats
match `wip/wm_units/agent_start`'s own `sc_CamParams` table
(`{0.1f, 12.0f, 1.0f, 0.0f}`) exactly; only the last entry differs (a
per-call-site parameter), reinforcing this is the same "camera ease-curve
parameter block" family, not a coincidence.

**Result: every instruction in case 12 matches the target's shape,
register allocation, and operand order exactly**, checked line-by-line
against the disassembly, with the SAME two already-characterized residual
classes as case 14: the shared `sW101` string and the new `sCamParams`
table both fold into the existing rodata-base register (`addi rN, r31,
0x64` / `addi rN, r31, 0x70`) instead of getting the target's own
dedicated `lis`/`addi` pair. No logic, register-allocation, or
control-flow difference found anywhere else in the case.

### Net result: `stepCutscene70` now has all 20 cases authored (up from 18)

| status | cases | count |
|---|---|---|
| authored, no open questions | 1, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19 | 17 |
| authored, but reads/writes an unidentified `.bss` singleton via byte casts | 2 | 1 |
| authored, structurally exact except the known constant-pool-folding residual | 0, 10 | (subset of the 17 above) |

**0 cases remain unauthored or blocked.** Whole-function diff count moved
from 517 (start of this round, after just dropping the shadow header) to
493 differing after both cases -- real, measured improvement, though the
function overall needs a 0-diff match on every remaining case's residual
class (constant pooling + one function-wide register-allocation gap,
likely stemming from register pressure that will only resolve once the
last stack-temp/pooling residuals are solved, which none of this round's
attempts moved) before it can flip to MATCH.

### Task-queue item 3: no genuinely unwritten function remains

Checked the full 19-function table before picking a "smallest unwritten
function" per instruction: every one of the 5 non-MATCH functions
(`resetPosition` 3 differing, `checkAnmLoop` 34 differing, `fn_2_16D1E0`
`.ctors` init 32 differing, `processCutsceneCommand` 136 differing,
`stepCutscene70` now 493 differing) already has real authored code from
earlier rounds -- none is blank. Re-ran `resetPosition` and `checkAnmLoop`
after this round's other changes (new file-scope constants added
elsewhere in the TU) in case of a ripple effect on constant-pool ordering,
per the "re-test after a genuine change" rule -- **both unchanged** (3 and
34 respectively), confirming no ripple. Did not re-attempt any of the
three previously-exhausted variants for these without a new hypothesis,
per instruction not to repeat already-ruled-out shapes.

## Final state, this round: 14/19 byte-identical (unchanged count, but stepCutscene70 now 100% case-covered)

| target | size | draft | note |
|---|---|---|---|
| classInit, ctor, dtor, create, execute, draw, doDelete, createModel, calcModel, resetStep, unusedStub, checkSpawnGate | — | **MATCH** (11) | |
| `fn_2_16D270` `.ctors` callback | 0x1C | **MATCH** | |
| `fn_2_16D100` startJump | 0x84 | **MATCH** | |
| `fn_2_16C530` resetPosition | 0x90 | 3 differing | walled, re-verified unchanged this round |
| `fn_2_16D050` checkAnmLoop | 0xB0 | 34 differing | walled, re-verified unchanged this round |
| `fn_2_16D1E0` `.ctors` init | 0x84 | 32 differing | walled, untouched this round |
| `fn_2_16C5E0` processCutsceneCommand | 0x230 | 136 differing | untouched this round |
| `fn_2_16C810` stepCutscene70 | 0x834 | 493 differing (down from 517) | **20/20 cases now authored** (up from 18/20); remaining gap is the constant-pooling residual class plus one function-wide register-allocation difference, not a logic/case gap |

**14/19 byte-identical.** Next round's highest-value target is almost
certainly the constant-pooling/register-allocation residual class itself
(now shared across cases 0, 10, 12, 14 AND `resetPosition`) -- a genuine
fix there would likely move several functions at once rather than one at
a time.
