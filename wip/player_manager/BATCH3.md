# Batch 3 — `0x8005F5C0`-`0x8005FA5F`, 7 functions

All 7 bodies were compiled and diffed against `wip/player_manager/target_text.txt`
via `tools/auto_decomp/harness.py` (`compile_draft`/`disasm`/`extract`/`diff_fn`),
using `mwcceppc.exe` with the exact flags from `SHARED-BRIEF.md`. Draft and
shadow headers live under `wip/player_manager/scratch/b3/`.

**Five of seven print an EMPTY diff (MATCHING, stated explicitly). Two
(`update`, `decideCtrlPlrNo`) do not print empty and are NOT claimed as
matching** — see their rows below for exactly how close they get and why.

## Per-function status

| Address | Function | Status | Notes |
|---|---|---|---|
| `0x8005F5C0` | `update()` | **NEAR MATCH, not claimed** | 173/174 instructions when canonicalised; every remaining diff line is a register-allocation/materialization choice (which physical register holds an already-known address, and whether MWCC pre-materializes a base+offset pointer once for reuse or folds it into each access), never a different instruction, operand kind, field, or branch target. Full logic verified correct against named fields (see below). See "update() in detail". |
| `0x8005F880` | `isPlayerPauseEnable(s8)` | **MATCHING** (15 instructions) | |
| `0x8005F8C0` | `setPlayer(int, dAcPy_c*)` | **MATCHING** (14 instructions) | |
| `0x8005F900` | `getPlayer(int)` | **MATCHING** (5 instructions) | Tail call, as predicted. |
| `0x8005F920` | `decideCtrlPlrNo()` | **NEAR MATCH, not claimed** | 26 vs target's 25 instructions; identical logic and instruction *kinds* in the same order, but MWCC's register allocator assigns the loop-invariant "mActPlayerInfo" and the constant "1" to different physical registers than our compile, and applies a one-instruction CSE for the `i==1` unrolled block (reusing the register that already holds constant `1` as the shift-amount operand too) that our compile does not reproduce. Tried ~8 source-level rewrites (hand-unrolled with literal/reassigned shift amounts, `for`-loop, `do`-while, extracting `mActPlayerInfo`/the mask into locals, calling the existing `checkPlayer()` inline, swapping operand order) — every variant either reproduces this exact register pattern or (when the shift amount is a compile-time-provable constant) MWCC optimizes the whole test into an `rlwinm` single-bit-test and drops the `slw`/`and.` shape entirely, which is further from target. The `for`-loop version below is the closest found. |
| `0x8005F990` | `setYoshi(daPlBase_c*)` | **MATCHING** (25 instructions) | **Return type contradicts the header** — see flag below. |
| `0x8005FA00` | `releaseYoshi(daPlBase_c*)` | **MATCHING** (22 instructions) | |

## Header contradiction: `setYoshi` returns `bool`, not `void`

The header currently declares `static void setYoshi(daPlBase_c *);`. The
disassembly loads `r3` with a clean `0` or `1` on **every** exit path — the
`player == nullptr` early-out (`li r3,0`), the "found a free slot" success
path (`li r3,1`), and the "no free slot" fallthrough (`li r3,0`) — matching
exactly the "every exit path explicitly loads r3" signature the header's own
comment on `fn_8005f4d0` uses to justify that function's `void`→`bool`
correction. **Correct signature: `static bool setYoshi(daPlBase_c *);`.**

I did not edit the real header (per the brief, and because it is marked
COMPLETE / do-not-edit in my brief). I shadow-copied it into
`wip/player_manager/scratch/b3/shadow_include/game/bases/d_a_player_manager.hpp`
with the signature fixed, compiled against that shadow copy, and got a clean
`MATCHING` result. `releaseYoshi`, by contrast, does **not** show this
pattern — its "found" path clobbers `r3` with the array base address
(meaningless as a return value) and its "not found" path leaves `r3`
unchanged from the incoming argument — consistent with genuine `void`,
matching the header as written.

## `update()` in detail — corroborated field/method identifications

Every symbol `update()` touches was cross-checked against already-landed
code or the existing header before being written, not guessed:

- **The `p->isWaitFrameCountMax()` call.** The disassembly's mystery
  double-indirect dispatch (`lwz r12,0x60(rX); lwz r12,0x370(r12); mtctr;
  bctrl`) is **not** a call through some undocumented sub-object — it is an
  ordinary virtual call through `daPlBase_c`'s own vtable, which this
  project's compiled objects place at object offset `0x60` (confirmed
  empirically: `dAcPy_c::walkAction_Wait()`, already landed at
  `source/dol/bases/d_a_player.cpp:2147`, opens with `if
  (!isWaitFrameCountMax())` and its disassembly in
  `tools/dis/corpus_dol_bases_d_a_player.txt:7608` uses the **exact same**
  `0x60`/`0x370` two-step). `isWaitFrameCountMax()` is declared virtual on
  `daPlBase_c` (`include/game/bases/d_a_player_base.hpp:669`), so no header
  change is needed for this one.
- **`STATUS_64` (0x64).** Confirmed by direct compile test
  (`player->mUniqueID` → `lwz r3,0x0(r3)`, an unrelated finding below) plus
  reading the `daPlBase_c::STATUS_e` enum's sequential numbering
  (`d_a_player_base.hpp:448-450`, unbroken from the last explicit `=`
  anchor) — the naming convention names each unnamed enumerator after its
  own hex value, and `STATUS_64` sits exactly at ordinal `0x64`.
- **`dNext_c::m_instance->mNextDataSet` (byte @0x18) and
  `->mMultiplayerDelay` (u16 @0x1c).** Computed from `d_next.hpp`'s
  already-declared layout: `mNextGotoData` (`sNextGotoData`, confirmed
  `0x14` bytes via `d_cd_data.hpp:113`'s `u8 mData[20]`) + `mSceneChangeType`
  (`int`, 4 bytes) lands `mNextDataSet` at `0x18` and, after
  `mStartSceneChange`/`mSceneChangeDone` and 2-byte alignment,
  `mMultiplayerDelay` at `0x1c` — both already-declared header members, no
  header change needed.
- **`dQuake_c::m_instance->mFlags` (@0x30).** Computed from `d_quake.hpp`'s
  already-declared float members (10 floats = `0x28`) + `mScreenOffset`
  (`mVec2_c`, 8 bytes) = `0x30`. **But the bits tested (`0x38` overall,
  `0x20` and `0x08` individually) do not correspond to any of the three
  declared `FLAGS_e` enumerators** (`FLAG_0=0x1, FLAG_1=0x2, FLAG_2=0x4`) —
  see the header gap flagged below.
- **`STATUS_QUAKE_BIG` (0x8b) / `STATUS_QUAKE_SMALL` (0x8c).** Already
  declared and named in `d_a_player_base.hpp:487-488` — not a guess.
- **`player->mUniqueID` compiles to offset `0x0`.** Verified directly (not
  inferred) with a throwaway compile: a function returning `player->mUniqueID`
  for a `dAcPy_c*` argument emits exactly `lwz r3, 0x0(r3); blr`. This settled
  `setPlayer`/`getPlayer`/`setYoshi`/`releaseYoshi`'s dereferences.

## Header/data gaps found while authoring `update()` — report, not resolved

`update()` cannot compile against the real, current headers. Everything
below is needed. I shadow-copied and patched the affected headers under
`wip/player_manager/scratch/b3/shadow_include/` (never the real files) to
prove the function's shape compiles; the real fixes are for the lead.

1. **`PauseManager_c` does not exist anywhere in the tree.** `grep -rn
   "PauseManager_c" include/ source/` finds zero hits outside this unit's own
   target disassembly. `update()` calls `PauseManager_c::m_instance
   ->setPauseEnable(bool)` (mangled `setPauseEnable__14PauseManager_cFb`,
   confirmed from the disassembly's own symbol name). A new header
   (`d_pause_manager.hpp`, by the project's naming convention) needs at
   least:
   ```cpp
   class PauseManager_c {
   public:
       void setPauseEnable(bool);
       static PauseManager_c *m_instance;
   };
   ```
2. **`dScStage_c` is missing `getGameDisplay()`.** Mangled
   `getGameDisplay__10dScStage_cFv`, called with `bl` (out-of-line, not
   inlined, unlike `getCourseIn`), returning what `update()` treats as a
   `dGameDisplay_c*`.
3. **`dGameDisplay_c` (`include/game/bases/d_game_display.hpp`) is missing
   four methods**, all confirmed by mangled name from the disassembly:
   `void setPlayNum(int*)`, `void setCoinNum(int)`, `void setScore(int)`,
   `void setCollect()`.
4. **`dPyEffectMng_c` is missing `update()`.** Only `fn_800d2de0` is
   currently declared; `update__14dPyEffectMng_cFv` is called from
   `update()` via `mspInstance`.
5. **`dStageTimer_c` is missing a field at offset `0xc`.** The class
   currently has only a vtable pointer (`0x0`) and `mTimeValue` (`0x4`),
   ending at `0x8` — four bytes short of `0xc`, and the byte at `0xc` itself
   is unaccounted for. `update()` does `stb` a `0`/`1` there gated on
   `mStopTimerInfo != 0`, strongly suggesting a `bool` (I named it
   `mStopped` in the shadow copy — name not verified, just a placeholder to
   compile against; content/meaning fits "is the stage timer currently
   stopped").
6. **`dQuake_c::FLAGS_e` is missing enumerators for bits 3-5** (`0x08`,
   `0x10`, `0x20`). The three declared (`FLAG_0=0x1, FLAG_1=0x2, FLAG_2=0x4`,
   `d_quake.hpp:20-24`) don't cover the mask `update()` actually tests
   (`mFlags & 0x38` gates the whole quake-dispatch block; `mFlags & 0x20`
   selects `STATUS_QUAKE_BIG`; `mFlags & 0x08` selects `STATUS_QUAKE_SMALL`).
   I wrote the literal hex masks rather than inventing enumerator names,
   since I have no behavioural evidence for what bits 3/4 (only 5 and 3 are
   actually read; bit 4, `0x10`, is included in the `0x38` gate mask but
   never tested individually) are *for*.

None of 1-6 are files this batch owns or may edit (`d_quake.hpp` and
`d_stage_timer.hpp` are foreign to this whole unit; `PauseManager_c` doesn't
have a home yet; `d_s_stage.hpp`/`d_game_display.hpp` are also foreign).
Reporting per the brief's "report contradictions, do not reconcile" rule.

## Data objects with their sections

**None owned by this batch.** `update()` references only pre-existing named
statics (`mQuakeTrigger`, `mPauseDisable`, `mStopTimerInfo`,
`mStopTimerInfoOld`, `mScore`, all already `.sbss`-declared in the header)
plus the foreign singletons/fields listed above. No new `.rodata`,
`.sdata`, `.sdata2`, or string data is emitted by any of my 7 functions —
confirmed by reading the compiled draft objects; none reference a pool
symbol at all except through calls into already-named externals.

## The `m_playerID`-relative addressing pattern in `update()`

`update()`'s HUD block (`mRest`) and both quake-array blocks (`m_quakeTimer`,
`m_quakeEffectFlag`) are **not** addressed by their own symbols in the
target — every access is a `base+literal` offset from a single
`m_playerID` relocation computed once at function entry (`r31`), even
though `update()` never itself reads or writes `m_playerID`. This is the
**same phenomenon** `STATICS.md` already documented independently for
`setHipAttackQuake` (owned by B7: `m_quakeTimer` written via
"`m_playerID+0xA0`"), so it is not an isolated fluke of this one function —
it looks like a systematic trait of how this TU's statics get addressed
under `-ipa file` once several of them are used together in one function.

I reproduced it by writing genuine pointer arithmetic from `m_playerID`
(`char *base = (char*)m_playerID; int *rest = (int*)(base+0x80);` etc.)
rather than referencing `mRest`/`m_quakeTimer`/`m_quakeEffectFlag` by name,
and it compiles to the same base+offset shape the target uses. **I do not
know if this reflects the literal original source** (hand-written pointer
casts feel unlikely) **or is purely a back-end address-folding optimization
that requires some other, unknown source shape to trigger identically** —
flagging per "report contradictions rather than reconcile them" rather than
presenting the pointer-cast version as confirmed-original. Whichever it is,
referencing `mRest`/`m_quakeTimer`/`m_quakeEffectFlag` by their own names
compiles to visibly wrong code (fresh `lis`/`addi` per symbol, no base
reuse) — so **whoever eventually re-attempts `update()` should not "clean
up" this pointer-cast style without re-verifying against the target first.**

## Files touched

- `wip/player_manager/BATCH3.md` (this file).
- `wip/player_manager/scratch/b3/` — draft source, shadow headers (never the
  real ones), and compile/diff scripts. Nothing outside `wip/player_manager/`
  was edited.
