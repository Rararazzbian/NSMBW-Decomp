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
