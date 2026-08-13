# Batch 3 report — goal-demo tail, fireworks, castle

STOPPED EARLY on a session-limit warning while mid-edit on `executeGoalCastle`.
The file `wip/demo_manager/dm-b3.cpp` has just been edited with a new theory
for the `save->isCourseDataFlag(...)` logic (see below) and **has NOT been
recompiled/reverified since that edit** — do not trust its current byte
match status for `executeGoalCastle` without recompiling first.

## Per-function status

| Function | Status |
|---|---|
| `executeGoalDemo` | **BYTE-EXACT**, verified via harness `diff_fn`. |
| `setGoalDemoKimeAll` | **BYTE-EXACT**, verified. |
| `setGoalDemoRunCastle` | **BYTE-EXACT**, verified. |
| `isAllPlayerGoalIn` | **BYTE-EXACT**, verified (needed `dAcPy_c *player;` hoisted out of the `for` loop, per coordinator relay — fixed an r29/r30 register swap). |
| `setHanabiEffect` | **VERY CLOSE, not byte-exact.** 84-86/87 instructions match depending on variant; every instruction's *logic, order and operands* verified correct against the target trace. The only remaining diffs are (a) which local-data symbol the compiler picks as an address "anchor" (see below) and (b) one instruction (`blt X; b Y` vs a single `bge Y`) in the leading guard clause that resisted several structural rewrites. See "setHanabiEffect" section below for exact detail and what was tried. |
| `executeGoalCastle` | **NOT YET BYTE-EXACT, mid-fix when stopped.** Was last measured at 173 (target) vs 179 (draft) instructions, with a real semantic bug identified and just-edited (not yet recompiled). See "executeGoalCastle" section — it has the most useful in-progress detail of anything in this report. |

## Verification method used throughout

`python -c` scripts calling `tools/auto_decomp/harness.py`'s `compile_draft`,
`extract`, and `diff_fn`, exactly as the brief specifies, with one addition:
**a scratch include-override directory** (outside the repo, in this agent's
scratchpad) was passed as `extra_inc` to `compile_draft` so two blocking
header gaps (see "Header contradictions" below) could be worked around
*for verification only*, without ever touching the tracked `include/` files.
Scratch path used:
```
C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\bbb6de6c-60a6-4643-b213-9bed3be8fe41\scratchpad\include_override\
  game\bases\d_s_stage.hpp   (copy of the real header + 2 added statics)
  game\bases\d_info.hpp      (copy of the real header + 2 added statics)
```
Every function was extracted **by address-confirmed name** (I read
`target.txt` at the literal 6 addresses in the brief's table before writing
anything, and re-confirmed function names/sizes matched exactly), and
compared via `diff_fn`, which itself uses `extract()` (canonicalised,
symbol-name-aware, not raw-word-only) per the brief's verification standard.
I did **not** get to run the mandated negative-control test (corrupt one
table entry, confirm the checker notices) — flagging this as incomplete per
the "never claim a match you did not observe" rule.

## HEADER CONTRADICTIONS — the most important thing in this report

`executeGoalCastle` references THREE static fields that do not exist in the
currently-committed, frozen headers. This blocked me from compiling it
against the real `include/` at all; I only ever compiled it against my
scratch-patched copies. **The lead must add these to the real headers**
before `executeGoalCastle` can be assembled for real:

1. `dScStage_c::m_OtehonClear_p` — `.sbss:0x8042A4D0`, size 4. A **pointer**
   type (dereferenced with `stb`/`lbz` at offsets `+0xb5`, `+0xb6`, `+0xb7`,
   `+0xb8`, `+0xb9` in `executeGoalCastle`/case 3). I do not know its real
   pointee type/name; I used `u8 *` in my scratch header purely so the byte
   offsets could be indexed. NOT in `include/game/bases/d_s_stage.hpp`.
2. `dScStage_c::m_goalType` — `.sbss:0x8042A4DC`, size 4, `int`. Confirmed
   `int` not `bool` because the target emits the full `neg/or/srwi`
   canonicalization tail when writing to it (per batch-4/1's relayed lever:
   an `int` destination keeps that tail, a `bool` destination would let MWCC
   skip it — and the target has the tail). NOT in the tracked header.
3. `dInfo_c` has **two** unmodeled fields inside its documented `pad4[0x8]`
   in `include/game/bases/d_info.hpp`: an `int` at offset `0x64` and an
   `int` at offset `0x68` (both plain `stw`'s of small int/bool-like
   values, e.g. `0`/`1`/`m_41`). The header's own doc-comment already
   admits `pad4` may hide real fields; this confirms it does. I split
   `pad4[0x8]` into `int m_64; int m_68;` in my scratch copy, removing the
   pad entirely (0x64-0x6c is now fully accounted for, no leftover pad).

None of these three were edited in the tracked `include/` — only in my
private scratch copies, per the hard rule. **Report only; the lead or
whoever owns `d_s_stage.hpp`/`d_info.hpp` needs to add real fields with
real names before this function can land.**

## `setHanabiEffect` — your nine (really eleven) `@LOCAL@` tables

The brief's headline "NINE `@LOCAL@` tables" is the `scHanabiOffset_1`
through `_9` family specifically (confirmed exact count: 9). Read literally,
the total distinct `@LOCAL@setHanabiEffect__13daPyDemoMng_cFv@...` symbols
in `bin/dtk/wiimj2d_symbols.txt` is **eleven** — the same paragraph in the
brief names the other two (`scHanabiOffsetDt`, `scHanabiEffectID`)
separately. **Flagging this explicitly as the brief's "nine" undercounting
its own definition by two** — report all eleven, not nine, to whoever
assembles the final file.

All eleven, in symbol-table order (== `@LOCAL@` numbering order == the order
I declared them in source, first-use order inside the function body):

| # | Symbol | Section:Addr | Size | Contents (verified against `original/wiimj2d.dol` raw bytes, NOT text-comparator-inferred) |
|---|---|---|---|---|
| 1 | `scHanabiOffset_1` | `.rodata:0x802EEEC0` | 0xC | 1 element: `{x=0.0, y=16.0, u16pair=(26,0)}` |
| 2 | `scHanabiOffset_2` | `.rodata:0x802EEED0` | 0x18 | 2 elements: `(64,16,26,0)`, `(-64,16,26,0)` |
| 3 | `scHanabiOffset_3` | `.rodata:0x802EEEE8` | 0x24 | 3 elements: `(0,16,20,0)`, `(64,16,10,0)`, `(-64,16,20,0)` |
| 4 | `scHanabiOffset_4` | `.rodata:0x802EEF10` | 0x30 | 4 elements — see `dm-b3.cpp` for full list |
| 5 | `scHanabiOffset_5` | `.rodata:0x802EEF40` | 0x3C | 5 elements |
| 6 | `scHanabiOffset_6` | `.rodata:0x802EEF80` | 0x48 | 6 elements |
| 7 | `scHanabiOffset_7` | `.rodata:0x802EEFC8` | 0x54 | 7 elements |
| 8 | `scHanabiOffset_8` | `.rodata:0x802EF020` | 0x60 | 8 elements |
| 9 | `scHanabiOffset_9` | `.rodata:0x802EF080` | 0x6C | 9 elements |
| Dt | `scHanabiOffsetDt` | `.data:0x8030992C` | 0x24 | 9 pointers, one per table above, in order 1..9. Verified by reading the raw 9 x 4-byte values out of the DOL and confirming they equal the 9 addresses above exactly, byte for byte. |
| ID | `scHanabiEffectID` | `.data:0x803099A0` | 0x10 | 4 pointers to the 4 firework colour strings `"Wm_ob_fireworks_y"`, `"_b"`, `"_g"`, `"_p"` (verified: read the raw pointers, followed each to its string, matched literally). |

**Important correction to the brief's Finding 7:** each `scHanabiOffset_N`
element is **NOT** a plain 3-float `mVec3_c`, despite the brief's/sibling
map's "obvious 1..9-element `mVec3_c[]` progression" framing. I read the
raw bytes directly and the 3rd 4-byte field of every element is a tiny
denormalized "float" (e.g. `0x001a0000`) that decodes cleanly as **two
`u16` values `(26, 0)` / `(20, 0)` / `(10, 0)`** — never as a sane float.
`setHanabiEffect` itself never reads this 3rd field (confirmed: only
offsets +0 and +4 are ever `lfs`'d), so I modeled it as a local struct
`{ float x, y; u16 m_08, m_0a; }` purely to reproduce the exact bytes; I do
**not** know its real name or semantic purpose (worth flagging to whoever
next touches fireworks/effect code — the value pattern `26`/`20`/`10`
correlates with which `scHanabiOffset_N` table it's in: table 1-2 use 26,
tables 3-9 use 20 for "primary" positions and 10 for "secondary" — looks
like some kind of effect-variant or delay ID, unconfirmed).

**A twelfth, UNNAMED table exists that is NOT one of "my" `@LOCAL@` objects**
and must NOT be given a name: `.data:0x803099F0-0x80309A18` (28 bytes = 10
pointers), holding 3 distinct strings — `"Wm_ob_fireworks_1up"` (indices
0-2), `"Wm_ob_fireworks_k"` (indices 3-8), `"Wm_ob_fireworks_star"` (index
9) — used to pick a special effect name keyed off `m_41` (capped `<10`).
It has **no `@LOCAL@` symbol at all** in `wiimj2d_symbols.txt` (the bytes
are absorbed into a generic, unnamed `lbl_803099ED` blob that also includes
3 bytes of alignment padding before it). I tried three ways to get MWCC to
reproduce this as an anonymous table: (1) a `switch` with 10 explicit cases
— MWCC compiled it as a real compare-chain, not a table, no matter how the
cases were grouped; (2) a plain **non-static** local `const char *const[10]`
— MWCC built it on the STACK at runtime (individual loads/stores, `_savegpr_27`
spill, totally wrong shape); (3) a `static const char *const[10]` **named**
local — this DOES reproduce the single-`lwzx`-indexed-load shape the target
has, but it necessarily gets its own `@LOCAL@setHanabiEffect...@names`
symbol that the real target does not have. **I landed on (3) as the least-wrong
option** (correct instruction shape and correct data bytes, wrong/extra
symbol name) and documented this discrepancy rather than silently
resolving it — the lead should decide whether an extra local symbol here is
acceptable or whether a different source shape is worth hunting for.

### `setHanabiEffect` — remaining known diffs (not yet closed)

1. **Address anchor choice.** The target's prologue computes
   `r31 = &dWmLib::sc_ForceList` (a `.data` object from `d_wm_lib.hpp`,
   **never actually read** by `setHanabiEffect` — confirmed by full
   instruction trace) purely as a cheap base register, then reaches
   `scHanabiOffsetDt` (`r31+0x24`), `scHanabiEffectID` (`r31+0x98`), and the
   unnamed 10-entry table (`r31+0xE8`) via 16-bit offsets from it instead of
   separate relocations. This is a real, reproducible MWCC `-O4` behavior —
   I verified it by watching MY OWN compile do the exact same trick, just
   anchored on one of MY OWN local tables instead of `sc_ForceList` (because
   `sc_ForceList` is dead-code-eliminated in my isolated 6-function
   compile: nothing in my batch reads it, so it's never emitted at all).
   **This is very likely a whole-TU-context artifact of compiling 6 of 51
   functions in isolation** — whichever OTHER batch's function genuinely
   uses `dWmLib::sc_ForceList`'s contents will cause it to be emitted in the
   final assembled file, and `setHanabiEffect`, sitting in the same real
   TU, should then anchor off it exactly like the target does. I could not
   verify this without the other 45 functions. **Flagging for the lead**:
   if the final assembled object still doesn't pick `sc_ForceList` as
   anchor even with the full file present, that's a real bug to chase;
   if it does, this diff should vanish for free.
2. One residual instruction-count diff (`blt LABEL; b LABEL2` in the target
   vs a single `bge LABEL2` in mine) for the leading `if (m_40 != 0 &&
   m_44 < m_41)` guard. Tried: separate vs combined guard clauses
   (`if(m_40==0)return; if(m_44>=m_41)return;` vs
   `if(m_40!=0 && m_44<m_41){...}` vs wrapping the whole rest of the
   function in the positive-form `if`) — all three produced byte-identical
   results to each other, all differing from target the same way. Did not
   find the lever. Likely another single-instruction, low-value item to
   revisit with fresh budget.

Everything else in `setHanabiEffect` — the `if(m_44==0){named}else{random}`
branch shape (needed a **redundant, always-taken-inside-that-arm** re-test
`if (m_40 != 0)` nested inside the `m_44==0` arm to reproduce the target's
reuse of `cr1` — confirmed this exactly reproduces the target's `cmpwi
cr1,r0,0` / `beq cr1,...` appearing TWICE), the `mVec3_c pos = mFireworkPos;
pos.x += offset.x; pos.y += offset.y;` copy-then-patch construction (NOT a
3-arg direct-sum constructor — that was my first, wrong guess; the target
literally stores the raw copy to the stack THEN overwrites x/y with the
sums, confirmed instruction-for-instruction), the `dGameCom::rndInt(6)`
random-color path, and both `dAudio::g_pSndObjMap->startSound(...)` calls —
all verified instruction-for-instruction identical to target, just shifted
by whatever the anchor-symbol diff shifts.

## `executeGoalCastle` — state when stopped

**Do not trust the file's current byte-match status without recompiling.**
Last full compile+diff (BEFORE the most recent edit) measured target=173,
draft=179 instructions — 6 instructions too many, from **duplicated**
`isCourseDataFlag` OTEHON-write code that shouldn't have been duplicated.

### The fix just made (unverified — recompile first)

Root cause found: the `world==2 && level==3` inner branch's OTEHON-vs-ELSE
decision is **not** a simple `if/else` picking between two
`isCourseDataFlag` calls (my first two attempts). Tracing the target's raw
control flow instruction-by-instruction (both branch targets, both
fallthrough paths) gives this truth table:
- `mGoalType == 0`: OTEHON iff `!save->isCourseDataFlag(world,level,0x90)`
  (0x120 check never runs).
- `mGoalType != 0`: OTEHON iff `!save->isCourseDataFlag(world,level,0x120)`
  (0x90 check never runs).

A clean `if(mGoalType==0){...} else {...}` (attempt 2) reproduces this
truth table but does NOT reproduce the target's register allocation: target
keeps `save` (the `dMj2dGame_c*` from `getSaveGame`) alive in **r28** (a 4th
callee-saved register, `stw r28,0x10(r1)` in the prologue) because its two
`isCourseDataFlag` call sites are reached via a **shared merge point** that
re-tests `mGoalType != 0` a SECOND time (a second `lwz`/`cmpwi` on the same
field) — meaning after the first `isCourseDataFlag` call clobbers r3, the
merge point can't statically know whether r3 still holds `save` (one
incoming path never called anything and still has it in r3; the other
path's call clobbered r3), so it always reads from the r28 backup instead.
A plain `if/else` has no such merge point (each branch is a dead end via
`break`), so MWCC correctly proved it never needs r28 — 3 saved regs, not 4,
size mismatch.

The just-made (unverified) fix reshapes this as a two-step accumulator that
DOES have that shared merge point, matching the observed "re-test
`mGoalType!=0` a second time" behavior:
```cpp
dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
bool ok = true;
if (mGoalType == 0) {
    ok = save->isCourseDataFlag(world, level, 0x90);
}
if (ok && mGoalType != 0) {
    ok = save->isCourseDataFlag(world, level, 0x120);
}
if (!ok) {
    dScStage_c::m_OtehonClear_p[0xb9] = 0;
    dScStage_c::m_OtehonClear_p[0xb8] = 1;
    dScStage_c::m_OtehonClear_p[0xb5] = 1;
    m_08 = 3;
    break;
}
```
Verify truth table equivalence: `mGoalType==0` -> `ok=check1`; second `if`'s
`mGoalType!=0` is false so `ok` stays `check1` -> OTEHON iff `!check1`.
Matches. `mGoalType!=0` -> `ok` stays `true` from init; second `if` fires
(`true && true`) -> `ok=check2` -> OTEHON iff `!check2`. Matches. **NEXT
STEP: recompile and re-diff `executeGoalCastle` against target — this was
the very next action when the session-limit warning arrived.** If it closes
the r28/size gap, the remaining diffs (as of the last real diff before this
edit) were: field-write order for `m_OtehonClear_p[0xb9/0xb8/0xb5]`
(**already fixed and confirmed correct** — write 0xb9 then 0xb8 then 0xb5,
not b5/b8/b9), and case-3's `dFader_c` argument
(**already fixed** — both `setNextScene` calls in case 3 use
`fader_type_e` value `1` = `FADER_CIRCLE_MIDDLE`, NOT `FADER_DRIP_DOWN`(3)
as I first guessed; `Exit_e` is `EXIT_0` for the true branch, `EXIT_2` for
the false branch).

### Other executeGoalCastle findings, believed solid (verified against target
### before the stop, should still hold)

- `case 0`: `if (m_08==0)` dispatch matches target's switch exactly
  (`cmpwi 0/1/2/3` chain, not a jump table) — was already byte-exact in an
  earlier diff pass.
- `case 1`: restructured from a branchless ternary
  (`m_08 = m_44!=0?1:2; m_0c = m_44!=0?0x3c:0x1c;` — WRONG, target branches)
  to the target's actual shape:
  ```cpp
  case 1:
      if (m_0c == 0) {
          m_44--;
          setHanabiEffect();
          m_0c = 0x1c;              // unconditional
          if (m_44 == 0) {
              m_08 = 2;
              m_0c = 0x3c;           // overwrites the 0x1c
          }
      }
      break;
  ```
  This was verified correct against the target trace (target sets `m_0c=0x1c`
  UNCONDITIONALLY first, then conditionally overwrites both `m_08` and
  `m_0c` only when `m_44==0`; when `m_44!=0` nothing else happens because
  `m_08` is already `1` from being in this case).
- `case 2` header sequence (`info->m_68=0; if(m_42) info->m_68=1;
  info->m_64=m_41; dScStage_c::m_goalType=(mGoalType!=0);` then
  `if (mGameMode==GAME_MODE_SUPER_GUIDE) {...} ` — this whole block was
  reverse-engineered from a **wrong initial guess**
  (I originally assumed `dInfo_c::getInstance()->mGameFlag=...` and
  `if(!dSaveMng_c::m_instance)` — both wrong; the real target dereferences
  `dInfo_c::m_instance` as an **instance pointer** at offsets `+0x64`/`+0x68`,
  not the static `mGameFlag`, and the `world<=9 && level<=0x29` gate is
  guarded by `mGameMode==GAME_MODE_SUPER_GUIDE`, not by a saveMng null
  check) — this part (through the `GAME_MODE_SUPER_GUIDE`/`world`/`level`
  bounds gate) WAS confirmed matching in the last real diff pass.
- `case 3`: confirmed matching (modulo cascading label-offset diffs caused
  by the case-2 size mismatch above) once the fader/exit values were fixed.

### Verified-correct symbol/type facts used throughout (from already-matched
### headers, re-confirm if anything above turns out wrong)

- `dInfo_c::m_startGameInfo.mGameMode` at offset `0x8` of `StartGameInfo_s`
  (confirmed by field layout: `mReplayDuration(4)+mMovieType(1)+mEntrance(1)
  +mArea(1)+mIsReplay(1)=8`, then `mGameMode` as an `int`-sized enum).
  `mWorld1`/`mLevel1` at `0xC`/`0xD` (also confirmed, matches the disasm's
  `lbz r30,0xc(r4)` / `lbz r29,0xd(r4)`).
- `dWmLib::IsCourseClear(int,int)` — free function in namespace `dWmLib`,
  NOT a class member (confirmed: no implicit `this` register load before
  the call).
- `dMj2dGame_c::isCourseDataFlag(int,int,ulong)` — member function; flag
  values `0x90` = `GOAL_NORMAL|SUPER_GUIDE_GOAL_NORMAL`, `0x120` =
  `GOAL_SECRET|SUPER_GUIDE_GOAL_SECRET` (both confirmed by adding the named
  enum constants from `d_mj2d_data.hpp` and matching exactly).
- `world`/`level` compare-instruction signedness is INCONSISTENT in the
  target: the two bounds checks (`world<=9`, `level<=0x29`) compile as
  `cmplwi` (unsigned) but the two equality checks (`level==3`, `world==2`)
  compile as `cmpwi` (signed) — **on the same two registers**. I had NOT
  resolved what source-level typing produces this exact mix before
  stopping (my `u8 world/level` locals currently produce `cmplwi` for
  everything, which is wrong for the equality checks) — this is an
  open, unverified item for whoever continues.

## Everything else / process notes

- Compiled against the real `include/` for the 5 functions that don't touch
  the missing fields (all of them except `executeGoalCastle`) — those 5 are
  genuinely, fully verified against the tracked headers, no scratch
  override needed.
- `daPyMng_c::checkPlayer(u8)` (inline accessor, not hand-written bitmask)
  was required for `isAllPlayerGoalIn`'s byte match, per the coordinator's
  relay — confirmed directly from the `clrlwi` truncation-before-shift
  instruction order in the target.
- Did not touch `slices/wiimj2d.json`, `syms.txt`, `include/`, `source/`,
  `HANDOFF.md`, `CODEX_HANDOFF.md`, `CODEX_PROMPT.md`, or any other batch's
  `dm-b*.cpp`. Only wrote to `wip/demo_manager/dm-b3.cpp`, this report, and
  my own scratchpad (`include_override/`).
- Did not run `ninja`, `configure.py`, `progress.py`, or `land.py`.
