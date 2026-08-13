# `daPyMng_c` static-member inventory and header audit

Deliverable for the shared `daPyMng_c` unit (see `SHARED-BRIEF.md`). Scope:
every static member (§1), the two arithmetic findings (§2), and the three odd
functions (§3). No function bodies authored, no header edited, no shared
files touched. All addresses below were re-derived independently from
`bin/dtk/wiimj2d_symbols.txt`, `wip/player_manager/target_*.txt`, and (for
`__sinit`) a fresh disassembly of `bin/dtkspl/obj/auto_sinit__d_a_player_ma_text.o`.

## Headline findings (read this first)

1. **The "16-byte hole" at `0x803551D0` is not a data member.** `__sinit`
   proves it is the `__register_global_object` destructor-chain node for
   `mDemoManager` (0xC bytes) plus a 4-byte alignment pad. This **refutes**
   the brief's leading hypothesis of an unnamed `static X m_something[4]`.
   See §2(a).
2. **`__sinit` fully resolves finding (b)**: the four embedded objects are
   constructed `daPyDemoMng_c → dMultiMng_c → dAttention_c → dPyEffectMng_c`,
   each via `__ct__...` followed by `bl __register_global_object(this, dtor,
   chainNode)`. The chain node is always the object that comes *right
   before* the constructed object in `.bss`. The one 4-byte gap the brief
   flagged (before `mAttention`) is simple 8-byte alignment padding — the
   exact same pad already present before `mDemoManager`, for the same
   reason. See §2(b).
3. **`fn_8005f4d0`'s header signature has the wrong return type.** Every
   return path explicitly loads `r3` with `0x0` or `0x1`; it cannot be
   `void`. Its parameter types are locked by the existing `syms.txt` pin
   (`P7mVec3_cii` — both `mode` and `flag` are plain `int`, not
   `PLAYER_POWERUP_e`). See §3.
4. **`fn_80060DB0` is almost certainly a file-scope `static` helper, not a
   `daPyMng_c` member** — it has no `syms.txt` pin and (unlike `fn_8005f4d0`,
   which a banked caller in `d_a_en_blockmain.cpp` reaches as
   `daPyMng_c::fn_8005f4d0`) no cross-TU caller exists anywhere in
   `source/`. It is only ever called once, from within `setHipAttackQuake`
   in the same TU. See §3.
5. Eighteen `.bss`/`.sbss` symbols carry the `9daPyMng_c` mangling and are
   **not declared in the header at all** (`m_playerID`, `m_yoshiID`,
   `m_yoshiFruit`, `mCoin`, `m_quakeTimer`, `m_quakeEffectFlag`,
   `m_yoshiColor`, `mScore`, `mStopTimerInfoOld`, `mQuakeTrigger`,
   `mBgmState`, `mBonusNoCap`, plus the four embedded managers). Every
   header-declared member that *does* have a symbol agrees in size and
   apparent element type with the disassembly — no contradictions found
   there. See §1.

---

## 1. Full static-member inventory

Legend: **H** = in header, **U** = usage-inferred from disassembly.

### 1a. Members already in `include/game/bases/d_a_player_manager.hpp`

| Address | Section | Symbol | Size | Declared (H) | Inferred (U) | Agree? |
|---|---|---|---|---|---|---|
| `0x80355130` | `.bss` | `mCourseInList` | `0x10` | `int[4]` | `stw`/`lwz`, word array | Yes (header even cites this address in its own comment — confirmed exact) |
| `0x80355150` | `.bss` | `mPlayerEntry` | `0x10` | `int[4]` | word array (`data:4byte`) | Yes |
| `0x80355160` | `.bss` | `mPlayerType` | `0x10` | `PLAYER_TYPE_e[4]` | word array; read via `lwz 0/4/8/0xc` in `getCoinAll` | Yes (enum-as-int, `-enum int`) |
| `0x80355170` | `.bss` | `mPlayerMode` | `0x10` | `PLAYER_POWERUP_e[4]` | word array | Yes |
| `0x80355180` | `.bss` | `mCreateItem` | `0x10` | `u32[4]` | word array | Yes |
| `0x80355190` | `.bss` | `mRest` | `0x10` | `int[4]` | word array | Yes |
| `0x80429F80` | `.sbss` | `mNum` | `0x4` | `int` | `lwz`/`stw` | Yes |
| `0x80429F84` | `.sbss` | `mCtrlPlrNo` | `0x4` | `u32` | `lwz`/`stw` (decideCtrlPlrNo) | Yes |
| `0x80429F88` | `.sbss` | `mActPlayerInfo` | `0x1` | `u8` | `lbz`/`stb` throughout | Yes |
| `0x80429F90` | `.sbss` | `m_star_time` | `0x8` | `s16[4]` | `data:2byte`, 4×2=8 | Yes |
| `0x80429F98` | `.sbss` | `m_star_count` | `0x8` | `s16[4]` | `data:2byte`, 4×2=8 | Yes |
| `0x80429FA4` | `.sbss` | `mKinopioMode` | `0x4` | `PLAYER_POWERUP_e` | word | Yes |
| `0x80429FA8` | `.sbss` | `mTimeUpPlayerNum` | `0x4` | `int` | word | Yes |
| `0x80429FAC` | `.sbss` | `mAllBalloon` | `0x4` | `int` | word | Yes |
| `0x80429FB0` | `.sbss` | `mPauseEnableInfo` | `0x4` | `int` | word (`isPlayerPauseEnable`) | Yes |
| `0x80429FB4` | `.sbss` | `mPauseDisable` | `0x4` | `u32` | word | Yes |
| `0x80429FB8` | `.sbss` | `mStopTimerInfo` | `0x4` | `u32` | word | Yes |
| `0x80429FCC` | `.sbss` | `mKinopioCarryCount` | `0x4` | `int` | word (`sda21` store) | Yes |

Every header-declared static member has a symbol, and every one agrees in
size with its declared type. **No "declared but no symbol" cases.** No type
contradictions found in this half of the table.

### 1b. Symbols present but **absent from the header**

| Address | Section | Symbol | Size | Inferred type | Evidence |
|---|---|---|---|---|---|
| `0x80355110` | `.bss` | `m_playerID` | `0x10` | `int[4]` (an ID, likely `fBaseID_e`) | `getPlayer(int)`: `lwzx r3,base,idx*4` then **tail-calls** `searchBaseByID__10fManager_cF9fBaseID_e(r3)` — so the array holds an `fBaseID_e`-typed handle, one per player slot, word-sized |
| `0x80355120` | `.bss` | `m_yoshiID` | `0x10` | `int[4]` (`fBaseID_e`) | Same `lis/addi` + word-indexed pattern as `m_playerID`, used throughout `setYoshi`/`releaseYoshi`/`getYoshi`/`getYoshiDirectP` |
| `0x80355140` | `.bss` | `m_yoshiFruit` | `0x10` | `int[4]` (or `u32[4]`) | `setCarryOverYoshiInfo`/`getYoshiFruit`: `clrlslwi r0,r3,24,2` (byte index → ×4) then `lwzx`/`stwx` — word-sized, 4 elements. **Note:** map entry has no `data:4byte` annotation unlike its 4-byte-array siblings, despite disassembly proving 4-byte elements — a minor map-generator inconsistency, not a type problem |
| `0x803551A0` | `.bss` | `mCoin` | `0x10` | `int[4]`, **indexed by `PLAYER_TYPE_e` value**, not raw player slot | `getCoinAll()` reads `mPlayerType[0..3]`, multiplies each by 4, and uses that as the index into `mCoin` (`lwzx`) — same map-annotation quirk as `m_yoshiFruit` (no `data:4byte` tag) |
| `0x803551B0` | `.bss` | `m_quakeTimer` | `0x10` | `int[4]` | `setHipAttackQuake`: `stwx r7(=5), (m_playerID+0xA0=this), idx*4` — countdown set to 5; `fn_80060DB0` reads it word-wise per player |
| `0x803551C0` | `.bss` | `m_quakeEffectFlag` | `0x10` | `int[4]` (bool-ish) | `setHipAttackQuake` clears it to 0 (`m_playerID+0xB0`); `fn_80060DB0` tests `==0`/sets `=1` |
| `0x80429F8C` | `.sbss` | `m_yoshiColor` | `0x4` | `u8[4]` | `setCarryOverYoshiInfo`: `stbx r4, base, r3` (raw, unscaled index — 1-byte elements); `getYoshiColor`: `lbzx r3, base, r3`. Size `0x4` = 4×1 byte, exactly matches |
| `0x80429FA0` | `.sbss` | `mScore` | `0x4` | `int` (scalar, not per-player) | Always addressed via plain `...@sda21(r0)`, never indexed — an aggregate total, read/written in `incCoin`/`addScore`-area code |
| `0x80429FBC` | `.sbss` | `mStopTimerInfoOld` | `0x4` | `u32`/`int` | `lwz`/`stw`, paired with `mStopTimerInfo` in `initStage`-area code |
| `0x80429FC0` | `.sbss` | `mQuakeTrigger` | `0x4` | `int`/`bool` | `lwz`/`stw`, near `mStopTimerInfoOld` |
| `0x80429FC4` | `.sbss` | `mBgmState` | `0x4` | `int` (small state enum, 0/1/2-ish) | `lwz`/`stw` across `startStarBGM`/`stopStarBGM`/`startMissBGM`/`startYoshiBGM`/`stopYoshiBGM` |
| `0x80429FC8` | `.sbss` | `mBonusNoCap` | `0x4` | `int`/`bool` | `stw`/`lwz` in `checkBonusNoCap()` (also undeclared as a *method* in the header) |
| `0x803551E0` | `.bss` | `mDemoManager` | `0x98` | `daPyDemoMng_c` | Constructed at `__sinit`+`0xD0` — matches brief and the class's own header docstring exactly |
| `0x80355284` | `.bss` | `mMultiManager` | `0x5C` | `dMultiMng_c` | Constructed at `__sinit`+`0x174` |
| `0x803552F0` | `.bss` | `mAttention` | `0x58` | `dAttention_c` | Constructed at `__sinit`+`0x1E0` |
| `0x80355354` | `.bss` | `mEffectMng` | `0xC5C` | `dPyEffectMng_c` | Constructed at `__sinit`+`0x244` |

All four embedded-object sizes (`0x98`/`0x5C`/`0x58`/`0xC5C`) match the brief
exactly and are independently corroborated by `__sinit`'s displacement
arithmetic (see §2b) — no contradiction there.

### 1c. Non-member data objects in our neighbourhood (report only, not `daPyMng_c` statics)

| Address | Section | Symbol | Size | What it is |
|---|---|---|---|---|
| `0x80427C00` | `.sdata` | `scRestMax` | `0x4` (`= 0x63` = 99) | File-scope **anonymous-namespace** `const int`, mangled `@unnamed@d_a_player_manager_cpp@::scRestMax` — not a class member |
| `0x80427C04` | `.sdata` | `scCoinMax` | `0x4` (`= 0x63` = 99) | Same anonymous-namespace pattern |
| `0x80427C08` | `.sdata` | `scScoreMax` | `0x4` (`= 0x3B9AC9FF` = 999,999,999) | Same anonymous-namespace pattern |
| `0x802EF608` | `.rodata` | `@LOCAL@getCourseInPlayerModelType__9daPyMng_cFUc@scModelTypeDt` | `0x10` | Function-**local** `static const int[4] = {0,1,2,3}` inside `getCourseInPlayerModelType(u8)`. This is the **entire answer to the brief's "derive it" `.rodata` line** — everything else in `target_rodata.txt`'s neighbourhood (Hanabi offsets, `dAcPy_HIO_Speed_c` tables, `scStoopOffset`, `scYoshiOffset`, `scCloudOffset`, `l_start_pos_ofs`, …) belongs to `daPyDemoMng_c`/`dAcPy_HIO_Speed_c`, not `daPyMng_c` |

---

## 2. The two arithmetic findings

### 2(a). The `0x803551D0`–`0x803551E0` "hole" — resolved, contradicts the brief's hypothesis

Disassembling `__sinit_\d_a_player_manager_cpp` (see §3 for the full listing)
shows:

```
lis  r31, m_playerID__9daPyMng_c@ha
addi r31, r31, m_playerID__9daPyMng_c@l   ; r31 = 0x80355110
addi r3,  r31, 0xd0                        ; r3 = 0x803551E0 (= mDemoManager)
bl   __ct__13daPyDemoMng_cFv
lis  r4,  __dt__13daPyDemoMng_cFv@ha
addi r3,  r31, 0xd0                        ; this        = mDemoManager (0x803551E0)
addi r4,  r4, __dt__13daPyDemoMng_cFv@l    ; dtor fn ptr
addi r5,  r31, 0xc0                        ; chain node  = 0x803551D0  <-- the "hole"
bl   __register_global_object
```

`r31 + 0xC0 = 0x803551D0` is passed as the **third argument** to
`__register_global_object(void *object, void (*dtor)(void*), void *node)` —
it is the 0xC-byte destructor-chain-registration node for `mDemoManager`,
not a data member. `target_bss.txt` independently shows this exact object as
an **unnamed `local`** symbol (`"@77033"`, size `0xC`), immediately followed
by a 4-byte compiler-inserted gap (`gap_08_803551DC_bss`) that brings
`mDemoManager` up to 8-byte alignment (`0x803551D0` is only 4-aligned;
`0x803551E0` is 8-aligned).

**This directly refutes the brief's leading hypothesis** ("an unnamed
`static X m_something[4]`"). It is not a per-player array of anything; it is
exactly the same kind of node that appears before every other embedded
manager (see 2b), and nothing in `daPyMng_c`'s own `.text` ever touches
`0x803551D0` directly (confirmed by grep over `target_text.txt` — zero
hits) — consistent with it being written only by the compiler-generated
`__sinit` glue, never by user code.

### 2(b). The `0xC` destructor-chain-record pattern — fully explained by `__sinit`

`__sinit` constructs the four managers in this order, and after each
construction immediately registers it:

| Constructed object | `this` | Chain-node arg (`r5`) | Node's raw `.bss` symbol |
|---|---|---|---|
| `daPyDemoMng_c` (`mDemoManager`) | `+0xD0` = `0x803551E0` | `+0xC0` = `0x803551D0` | `"@77033"` (0xC) + 4-byte pad |
| `dMultiMng_c` (`mMultiManager`) | `+0x174` = `0x80355284` | `+0x168` = `0x80355278` | `"@77034"` (0xC), no pad |
| `dAttention_c` (`mAttention`) | `+0x1E0` = `0x803552F0` | `+0x1D0` = `0x803552E0` | `"@77035"` (0xC) + 4-byte pad |
| `dPyEffectMng_c` (`mEffectMng`) | `+0x244` = `0x80355354` | `+0x238` = `0x80355348` | `"@77036"` (0xC), no pad |

So **every** embedded object is preceded in `.bss` by its own 0xC
`__register_global_object` chain node (all four classes therefore have
non-trivial/virtual destructors — consistent with `daPyDemoMng_c`'s already-
proven one-slot vtable). The chain-node size (0xC) is constant; the only
variable is whether a 4-byte alignment pad follows it, and that is
explained purely by **arithmetic**, from our side, with no need to touch
`dAttention_c`/`dMultiMng_c`:

- `mDemoManager` (`0x803551E0`) and `mAttention` (`0x803552F0`) both land on
  an 8-byte boundary → each needed a 4-byte pad after its 0xC node to get
  there (`0x803551D0+0xC=0x803551DC`, not 8-aligned, so a pad is inserted;
  same at `0x803552E0+0xC=0x803552EC`).
- `mMultiManager` (`0x80355284`) and `mEffectMng` (`0x80355354`) only land on
  a 4-byte boundary → their 0xC node is immediately followed by the object,
  no pad needed.

**Evidence bearing on Codex's `sizeof` question (report only, not resolved
here):** this shows `dAttention_c`, like `daPyDemoMng_c`, requires 8-byte
alignment (mod-8-zero start address), whereas `dMultiMng_c` and
`dPyEffectMng_c` only require 4-byte alignment. That is consistent with
`dAttention_c` (like `daPyDemoMng_c`) having a vtable pointer plus some
8-byte-aligned member (e.g. a `double`), while `dMultiMng_c` does not. This
is offered as evidence only — Codex's sizes are assumed correct per the
brief and not re-derived here.

---

## 3. `__sinit`, and the three odd functions

### `__sinit_\d_a_player_manager_cpp` (`0x80061310`, `0x9C`)

Contrary to the brief's suggested path, this symbol is **not** in its own
`auto_03_80061310_text.o` (that file doesn't exist — `auto_03_8005E9A0_text.o`
covers only up to `0x80061310` exactly, i.e. our own `.text` region ends
exactly where `__sinit` begins). The disassembler locates it instead in
**`bin/dtkspl/obj/auto_sinit__d_a_player_ma_text.o`** (found by pattern
`auto_sinit__*`, not `auto_03_<addr>*`). Disassembling that object (absolute
`dtk-windows-x86_64.exe` path, per house rules) gives the complete function,
reproduced above in §2(b)'s summary. In short, it:

1. Computes `r31 = &m_playerID` once.
2. Calls `__ct__13daPyDemoMng_cFv(r31+0xD0)`, then
   `__register_global_object(r31+0xD0, &__dt__13daPyDemoMng_cFv, r31+0xC0)`.
3. Calls `__ct__11dMultiMng_cFv(r31+0x174)`, then registers it with node
   `r31+0x168`.
4. Calls `__ct__12dAttention_cFv(r31+0x1E0)`, then registers it with node
   `r31+0x1D0`.
5. Calls `__ct__14dPyEffectMng_cFv(r31+0x244)`, then registers it with node
   `r31+0x238`.
6. Restores and returns.

The `.ctors` slot at `0x802EDD68` (`.ctors:0x88` per the brief's table)
holds exactly one 4-byte pointer to this `__sinit` function — the brief's
"one free slot" claim is consistent with this being a single, self-contained
`.ctors` entry.

This **pins the .bss order** used throughout §1–2: `m_playerID` base, then
+0xD0 `mDemoManager`, +0x174 `mMultiManager`, +0x1E0 `mAttention`, +0x244
`mEffectMng` — matching `target_bss.txt` exactly, address for address.

### `fn_8005F4D0` (`0x8005F4D0`, `0x9C`)

Header currently declares:
```cpp
static void fn_8005f4d0(mVec3_c *pos, int mode, int flag);
```
`syms.txt:49` already pins `fn_8005f4d0__9daPyMng_cFP7mVec3_cii = 0x8005F4D0`
— confirming it **is** a real `daPyMng_c` static member (also proven by the
already-banked `source/dol/bases/d_a_en_blockmain.cpp:722`, which calls it
as `daPyMng_c::fn_8005f4d0(&pos, l_player_mode[mode], flag)`), and confirming
the parameter types exactly as declared: `mVec3_c*`, `int`, `int` (**not**
`PLAYER_POWERUP_e` for `mode`, even though `mode` is passed straight through
as the first argument to `fn_8005f570(PLAYER_POWERUP_e, int)` inside the
function body — the mangled name overrides that inference).

**The return type is wrong.** Disassembly:
- Loops `plrNo = 0..3`; for the first slot where `getPlayer(plrNo) == 0`
  (empty), calls `fn_8005f570(mode, plrNo)`; if that returns true, indexes a
  2-entry table at `.sdata2:0x8042BD70` (`{0x19, 0x1A}`) with `flag` to get a
  3rd argument, calls `create(plrNo, pos, table[flag], 0)`, and **returns 1**
  (`li r3,0x1`).
- If the loop exhausts all 4 slots without success, **returns 0**
  (`li r3,0x0`).

Every exit path explicitly sets `r3`; this cannot be `void`. Correct
signature:
```cpp
static bool fn_8005f4d0(mVec3_c *pos, int mode, int flag);  // or plain int
```

### `fn_80060DB0` (`0x80060DB0`, `0x138`)

No header declaration, no `syms.txt` pin, and **zero cross-TU callers**
(`grep -rn "80060DB0" source/ include/` finds nothing outside this
inventory). Its only caller anywhere in the extracted text is
`setHipAttackQuake__9daPyMng_cFiUc`, one basic block before its own
epilogue, in the *same* translation unit.

Signature: **no incoming arguments** (`r3` is never read before being
clobbered by the first `bl`) and **no return value** (no `r3` write before
any return; the function falls straight into the restore/`blr` epilogue).
So: `static void <name>();`.

Behaviour: calls `SndSceneMgr::onPowerImpact()` once, then for `i = 0..3`:
if `m_quakeTimer[i] != 0 && m_quakeEffectFlag[i] == 0`, sets
`m_quakeEffectFlag[i] = 1`, fetches `getPlayer(i)` (substituting the ridden
Yoshi via `getRideYoshi()` if the player `isStatus(0x4b)`), and — if a
player/Yoshi was found — spawns two `mEf::createEffect` calls at its
position (`0xac/0xb0/0xb4` floats) using the two string constants at
`.data:0x80309A28`/`0x80309A3C` (**both are the two strings the brief marks
as Codex's — evidence, not a resolution: both are used exclusively by this,
our own, function**), then calls `dQuake_c::startShock(...)` on the
`dQuake_c` singleton.

**Verdict:** this is best modelled as a **file-scope `static` helper
function defined directly in `d_a_player_manager.cpp`**, not a
`daPyMng_c` member — it needs neither a header declaration nor a
`syms.txt` name pin, unlike `fn_8005f4d0` which needed both (precisely
because it has a real cross-TU caller). This mirrors the brief's own
framing of the key test almost exactly.

**Additional `.sdata2` evidence for Codex (not resolved here):** the two
floats at `0x8042BD78` (`0.5f`) and `0x8042BD7C` (`≈15200.0f`, the brief's
"`~0xa18-0xa20`" range) are referenced **only** by `daPyMng_c`'s own code —
`0x8042BD78` from `incCoin`, the `addRest`-area code, and
`deleteCullingYoshi`; `0x8042BD7C` from `fn_80060DB0` itself.

---

## 4. Section-boundary sanity checks against the brief

- `.bss`: brief states `0x3790–0x4640` off base `0x80351980` →
  `0x80355110–0x80355FC0`. `target_bss.txt`'s own neighbourhood file
  (`0x80354F20..0x80355FC0`) ends in a `0x10`-byte trailing gap object
  (`gap_08_80355FB0_bss`) at exactly `0x80355FC0` — end boundary confirmed.
- `.sdata`: brief states `0x280–0x290` off base `0x80427980` →
  `0x80427C00–0x80427C10`. `target_sdata.txt` confirms exactly three 4-byte
  objects there (`scRestMax`, `scCoinMax`, `scScoreMax`) followed by a
  4-byte gap to `0x80427C10` — matches exactly, but see the anonymous-
  namespace note in §1c: **these are not class members.**
- `.sbss`: brief states offset `0xE0–0x110` (no absolute address given).
  The actual `9daPyMng_c`-tagged `.sbss` symbols span
  `0x80429F80–0x80429FD0` (18 symbols, `0x50` bytes total — see §1a/§1b).
  If the section base is back-derived as `(0x80429F80 − 0xE0) =
  0x80429EA0`, the true extent is offset `0xE0–0x130`, not `0xE0–0x110` as
  stated — a `0x20`-byte discrepancy. **Flagging per the brief's own "report
  contradictions" rule; not reconciled here** since the brief never states
  the `.sbss` base and I only back-derived it from one assumed anchor point.
- `.data` (`0x80309A28-0x80309A58`, Codex's): confirmed present, exactly two
  strings (`"Wm_mvrs_vpichattack\0"`, `"Wm_mvrs_vpichattack_ind\0"`), both
  consumed only by `fn_80060DB0` in our own disassembly — evidence only, not
  resolved, per the brief.

## 5. Files touched

Only `wip/player_manager/STATICS.md` (this file) was written. No edits to
`include/`, `slices/`, `syms.txt`, or any other shared file. Scratch
disassembly output used for this audit lives in the session scratchpad
(`auto_8005E9A0.txt`, `sinit_player_manager.txt`) and is not part of the
deliverable.
