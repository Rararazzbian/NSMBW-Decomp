# AC_WATER_MOVE / AC_WATER_MOVE_REGULAR -- daWaterMove_c

`.text 0x152010-0x1530E0` (0x10D0 bytes), ONE `.ctors` entry (`0x394 -> __sinit` at
`0x152CE0`). One class, two classInit entry points -- confirmed the coordinator's own
prediction two independent ways: (1) direct target bytes -- `fn_2_152010` and `fn_2_1520B0`
are identical apart from address, same `li r3, 0x4c0`, same vtable patch to
`lbl_2_data_421C0`; (2) a compile probe (`probe_oneclass.cpp`, this directory) confirms two
classInit stubs for one placeholder class emit ONE shared local vtable, not two.

## Current tally: 19/27 byte-identical modulo symbol names, order GREEN, .ctors correct

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

### Not yet matched (8) -- honest state, not claimed as landed

Per-function diff counts this round (raw `difftool.py` line counts, not the normalised
`verify_anon.py` score -- so these overstate the true gap slightly, since naming-only
lines still show as "different" here):

| function | diff (start of round -> now) | attempts this round | parked because |
|---|---|---|---|
| `create()` | untouched, 141/145 | 0 | Largest remaining function; bitfield-heavy, multi-branch, not started this round in favour of the smaller ones. |
| `execute()` | untouched, 27/54 | 0 | Same -- order/structure already right, likely register-allocation only, not attempted this round. |
| `createMdl()` | 70 -> 50 | 3 (see below) | Anchor-pooling gap: target reads its 3 model-variant name strings AND the two `getRes()` strings all anchor-relative to `g_profile_AC_WATER_MOVE` itself; consolidating into one shared `sWaterFloatNames[]` array got the CALL SHAPE right (attempt 2, 76->50), but forcing MWCC to co-locate the `getRes()` strings into the SAME pool regressed it (attempt 3, 50->68, reverted). Parked at the attempt-2 state. |
| `calcModel()` | untouched, 63/65 | 0 | Not attempted this round. |
| `checkPlayers()` | untouched | 0 | Not attempted this round. |
| `approach()` | 49 -> 23 | 3 | (1) hoisted the rodata table address into one local `const f32 *table` shared across both branches instead of two separate loads (49->46); (2) rewrote the entry comparison from `mUnk484 >= value` to `!(mUnk484 < value)`, which changed the compiled branch from a `cror`+`bne` pair to the target's own single `bge`-shaped form (46->23); (3) restructured the tail to index `mUnk47C`/`mUnk480` as a `clamp[which]` 2-element array, matching the target's own `slwi r0,r5,2; add r4,r3,r0` indexing -- this REGRESSED it (23->46) and was reverted. Parked at attempt-2's state; the array-indexing shape is very likely still correct semantically (the target bytes clearly show it), just not reproducible from this specific C++ phrasing in one more try. |
| `calcWave()` | 37 -> 24 | 2 | (1) switched from manual `mUnk4BA * (1/256)` multiplication to `nw4r::math::CosF(U16ToF32(mUnk4BA))`, matching the real inline chain in `include/lib/nw4r/math/math_triangular.h` (37->27 raw, fixed the wrong int-to-float bit-trick codegen); (2) switched to calling `CosIdx`/`SinIdx` directly (same header) instead of manually chaining `U16ToF32`+`CosF` (cosmetic, same score). Remaining gap is one `lha` vs `lhz` half-word load (signed vs unsigned read of `mUnk4BA` feeding the call) plus its downstream scheduling; not resolved in the tries spent. |
| `__sinit` | 21/159, untouched | 0 | Deliberately last, per your own guidance -- already close purely as a side effect of the class/state fixes, nothing in this unit is blocked on it. |

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
