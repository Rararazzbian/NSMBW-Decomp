# AC_WATER_MOVE / AC_WATER_MOVE_REGULAR -- daWaterMove_c

`.text 0x152010-0x1530E0` (0x10D0 bytes), ONE `.ctors` entry (`0x394 -> __sinit` at
`0x152CE0`). One class, two classInit entry points -- confirmed the coordinator's own
prediction two independent ways: (1) direct target bytes -- `fn_2_152010` and `fn_2_1520B0`
are identical apart from address, same `li r3, 0x4c0`, same vtable patch to
`lbl_2_data_421C0`; (2) a compile probe (`probe_oneclass.cpp`, this directory) confirms two
classInit stubs for one placeholder class emit ONE shared local vtable, not two.

## Current tally: 20/27 byte-identical modulo symbol names, order GREEN, .ctors correct

```
python wip/wm_units/agent_water_move/build.py
```
No "FUNCTION ORDER IS WRONG". The order fix was real, not cosmetic: an earlier round had
all three `initializeState_*` grouped together, then all three `finalizeState_*`, then both
non-trivial executes -- ground-truthing against `bin/dtk/d_basesNP_symbols.txt`'s own
addresses shows each state's own `finalizeState_X` precedes its `initializeState_X`/
`executeState_X` pair, and the whole class's own definition order is
`create, execute, draw, doDelete, createMdl, calcModel, checkPlayers, approach, calcWave`,
then the nine state functions, `~daWaterMove_c()` LAST (immediately before `__sinit`).

### Matched (19)
Both classInits, `draw()`, `doDelete()`, all nine state functions (three
`initializeState`/`executeState` blr stubs, three identical `finalizeState` bodies
resetting `mSpeed` to zero, `executeState_Udmove`/`executeState_Lrmove` via `approach()`),
the destructor, and all five compiler-template-generated `sFStateID_c<daWaterMove_c>`
members (its own destructor, `isSameName`, and the three `__ptmf_scall` state trampolines)
-- none of the last five were hand-authored; `STATE_FUNC_DECLARE`/`STATE_DEFINE` (the same
idiom `source/d_basesNP/bases/d_a_wm_sandpillar.cpp` uses) generates them by template
instantiation.

### Real defects found and fixed this round, worth recording
- **A phantom member.** An earlier draft declared a separate `mAllocator_c mAnimAllocator`
  member, believing `mAnimTexSrt.create()`/`mModel.create()` needed their own allocator.
  They don't -- both target calls pass `&mAllocator` (the `dHeapAllocator_c` at +0x3d4,
  which IS-A `mAllocator_c`). Removing the invented member made `sizeof(daWaterMove_c)`
  match `0x4c0` exactly and collapsed a spurious second `bl __ct__12mAllocator_cFv` down to
  the one the target actually has. A single-instruction-at-a-time diff caught this; nothing
  about the class's own vtable or profile data hinted at it.
- **`finalizeState_*`'s real target was `mSpeed`** (a real, landed `dBaseActor_c::mSpeed`,
  `include/game/bases/d_base_actor.hpp:140`), not an invented `daWaterMove_c`-own field.
  An early transcription misread the target's own `0xe8/0xec/0xf0` offsets as something in
  this class's own added-member region; they are inside the INHERITED region (`mScale`
  ends at `0xe8`, matching `mSpeed`'s own declared position right after it).
- **A genuine unidentified 4-byte field between `mModel` and `mAnimTexSrt`** (`+0x434`),
  explicitly zeroed by an inline constructor MWCC needs anyway (`mResFile` has no default
  constructor -- `ResCommon<T>`'s only ctors take a `void*`). Declared `u32 mUnk434;`
  initialised via the class's own explicit-but-inline `daWaterMove_c() : mResFile(nullptr),
  mUnk434(0) {}`, which the compiler inlines into `new` exactly as classInit shows (no
  separate `bl __ct__13daWaterMove_cFv`).
- **A single shared, anchor-relative rodata pool.** `approach()`/`calcWave()`/`create()`/
  `calcModel()` all read addresses relative to `lbl_2_rodata_81C8` well past that object's
  own 0x48-byte size -- MWCC pools five adjacent `.rodata` objects (81C8/8210/8228/8240/
  824C/8250) into one contiguous blob addressed off the first symbol, the exact anchor
  pattern already established for `g_profile_AC_WATER_MOVE`'s own `STATE_DEFINE` arguments.
  Declared as ONE file-scope `static const u32 sWaterMoveConsts[]` (raw bits -- two of the
  35 slots are not floats: a `0x00030000` sentinel and a `{1,2,4,8}` byte lookup table) with
  a `wmConstF(i)` reinterpret helper, used consistently everywhere instead of per-function
  local arrays (which produced per-function duplicate local objects, provably wrong since
  the target has one pool, not N).

### Round 4: execute() reached N/N; two levers recorded as doctrine paid off directly

**`execute()` is now MATCHED (was 27 differing).** Two real fixes, not scheduling noise:
- `mUnk4A8 > 0` should have been `mUnk4A8 != 0` -- the target's own `beq`/no-decrement-at-zero
  shape only comes from an explicit `!= 0` test, not a signed `> 0` one (comparison-direction
  lever, same family as `approach()`'s `>=` -> `!(...<...)` fix last round).
- The `mUnk470/474/478` delta store is NOT three independent assignments in declaration
  order -- the target computes it as one `mVec3_c` temporary, `mVec3_c(x-delta, y-delta,
  z-delta)`, and MWCC evaluates constructor arguments RIGHT TO LEFT, so the z-component is
  computed FIRST despite being written last in the source. Writing the constructor call in
  natural `(x, y, z)` order (relying on that evaluation order, not fighting it) reproduced
  the target's own z,y,x compute sequence exactly. Also applied `dGameCom::rndF`-style
  lesson in reverse: this is a case where TRUSTING an unintuitive but real compiler behaviour
  (argument evaluation order), rather than hand-ordering the source to match the visible
  instruction order, was the fix.

**`calcWave()` narrowed further, 24 -> 22.** The `lha`/`lhz` residual WAS a type question,
exactly as flagged: `mUnk4BA` is `u16`, not `s16` -- changing the declared field type fixed
both `CosIdx`/`SinIdx` call-site loads to `lha`, matching target. One load (the
`mUnk4BA += 0x400` increment's own re-read) still shows `lhz` where target wants `lha`;
tried forcing it via an explicit `(s16)` cast on the increment, no change, reverted. Left at
22 -- a genuine, if partial, win from the type-question lever.

**`checkPlayers()` gained one real bug fix, no net tally movement.** `approach()`'s own
`mUnk47C`/`mUnk480` (clamp bounds) were being passed to `fn_800EBBC0` where the target
actually passes `mPos.x`/`mPos.y` -- fixed. A `kind == 1 || kind == 2` -> nested-if rewrite
(testing whether the fused `(kind-1) <= 1` range check was avoidable) made the diff WORSE
(65 -> 79) and was reverted; the `||` form apparently is what the original wrote, and MWCC's
own optimizer fuses it regardless of source phrasing -- a genuinely different function must
be doing something else to avoid the fusion, not this one. Two structural items remain
unexplained: (1) `_savegpr_27`/`_restgpr_27` (a multi-register save helper this draft's
smaller local-variable count doesn't trigger), and (2) a virtual call through a pointer
stored at the player object's own `+0x60` (`lwz r12,0x60(r28); lwz r12,0x6c(r12); mtctr;
bctrl` -- reads a POINTER at that offset and treats it directly as a vtable, not
double-indirected through an object+vtable-pointer pair the way a normal member call would
be). Neither is a scheduling issue; both need more investigation than this round had time
for. Not parked after 3 attempts -- only 2 spent -- but time-boxed this round in favour of
`execute()` and `calcWave()`, which paid off.

**`calcModel()` -- looked at but not attempted.** Target is 157 instructions; the current
draft is a 52-instruction placeholder missing the whole three-pass `ZrotM`/`PSMTXScale`/
`PSMTXConcat` wobble sequence entirely. This is a full rewrite, not a lever-application --
flagged for next round rather than attempted partially this one.

**`create()` -- untouched again this round**, still 141/145; same reason as `calcModel`,
a large rewrite rather than a lever fix.

### Not yet matched (7) -- honest state, not claimed as landed

| function | status |
|---|---|
| `create()` | Untouched. Large (145 instructions), bitfield-heavy, multi-branch. Needs a dedicated round. |
| `createMdl()` | Parked at 3 attempts last round (70->50). Anchor-pooling gap on the `getRes()` call strings specifically; do not re-attempt without a new lever. |
| `calcModel()` | Looked at, not attempted -- current draft only covers ~1/3 of the target's real work (missing the 3-pass rotation/wobble sequence). Needs a dedicated round, not a quick fix. |
| `checkPlayers()` | One real bug fixed (`mPos.x`/`mPos.y` instead of the clamp-bound fields), net diff count unchanged. Two unexplained structural gaps: `_savegpr_27` register-save pattern, and a virtual dispatch through a pointer-as-vtable at `+0x60` that needs independent investigation, not another lever guess. |
| `approach()` | Parked at 3 attempts (49->23) last round. |
| `calcWave()` | `mUnk4BA` type fixed to `u16` this round (24->22); one `lhz`-vs-`lha` load (the increment's own re-read) still open, one attempt spent on it this round (cast, no effect). |
| `__sinit` | Still last, still untouched, still 21/159 as a side effect. |

## Verified vs inferred, explicitly
VERIFIED (byte-identical against a freshly-dumped target, this session): both classInits,
draw, doDelete, all nine state functions, the destructor, all five template-generated
`sFStateID_c` members. Order and `.ctors` count independently confirmed via
`verify_anon.py`'s own gate.

INFERRED, not yet round-tripped to a clean match: create, execute, createMdl, calcModel,
checkPlayers, approach (partially), calcWave (partially), __sinit. Every field/type/call
target used in these is either a real landed symbol (cross-checked against its own header)
or an explicitly-flagged `mUnk<offset>`/`field_<offset>_ref` placeholder for something in
the INHERITED (already-landed-elsewhere) `dActor_c`/`dBaseActor_c` region that this unit
does not own and cannot rename.

## Files
- `d_a_wm_water_move.cpp` -- the draft, 19/27.
- `probe_oneclass.cpp` -- empirical one-class confirmation.
- `shadow_include/game/bases/d_game_com.hpp` -- adds ONE declaration
  (`dGameCom::rndF(float)`) the real header is missing; proof in the file's own header
  comment (dtk's full symbol map already names `rndF__8dGameComFf` at a real DOL address,
  and it is not the same function as the already-landed `cM::rndF`).
- `target_auto_00_00151E90_text.txt` / `target_auto_fn_2_152CE0_text.txt` /
  `target_auto_00_00152F5C_text.txt` -- fresh target `.text` dumps (all three objects
  `check_target_objs.py` requires, `__sinit`'s own split object included).
- `target_auto_04_0003A960_data.txt` -- fresh target `.data` dump (vtable + profile structs
  + the STATE_DEFINE argument pool).

Not landable yet -- 8 functions short of N/N, all real content gaps, not structural ones.
