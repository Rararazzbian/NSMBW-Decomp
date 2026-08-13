# Batch B4 — `0x8005FA60`-`0x8005FDAF`, 10 functions

All 10 functions compiled and diffed against `target_text.txt` with
`tools/auto_decomp/harness.py` (`compile_draft` / `disasm` / `diff_fn`), using
the exact compile flags from `SHARED-BRIEF.md`. Working files are in
`wip/player_manager/scratch/b4/` (`draft.cpp` is the final source; the
`try_*.py` scripts are the empirical searches that found each match, kept for
audit).

## Status table

| Function | Address | Size | Status |
|---|---|---|---|
| `getYoshi(int)` | `0x8005FA60` | `0x9C` | **NOT MATCHING** — 37/39 instructions match; 2 differ only in register choice (`r4` vs `r12`). See "getYoshi" below. |
| `getYoshiNum()` | `0x8005FB00` | `0x6C` | **MATCHING** (27 instructions) |
| `getYoshiDirectP(int)` | `0x8005FB70` | `0x14` | **MATCHING** (5 instructions) |
| `getCtrlPlayer(int)` | `0x8005FB90` | `0x50` | **MATCHING** (20 instructions) |
| `getCourseInPlayerModelType(u8)` | `0x8005FBE0` | `0x40` | **MATCHING** (16 instructions) |
| `setCarryOverYoshiInfo(u8,u8,int)` | `0x8005FC20` | `0x1C` | **MATCHING** (7 instructions) |
| `getYoshiColor(u8)` | `0x8005FC40` | `0xC` | **MATCHING** (3 instructions) |
| `getYoshiFruit(u8)` | `0x8005FC50` | `0x14` | **MATCHING** (5 instructions) |
| `getActScrollInfo()` | `0x8005FC70` | `0xA8` | **MATCHING** (42 instructions) |
| `getScrollNum()` | `0x8005FD20` | `0x8C` | **MATCHING** (35 instructions) |

9 of 10 diffed clean (printed nothing). `getYoshi` is reported as NOT matching
per the brief's rule — see below for why, and for the alternative that *does*
print a clean diff but carries a different, larger risk.

## Source

```cpp
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/framework/f_manager.hpp>

// ---------------------------------------------------------------------
// Unnamed daPlBase_c field this batch touches. Offset proven by this TU's
// own disassembly (see the per-field comment at each use site); left as a
// raw cast per house style, since d_a_player_base.hpp is a shared header
// this batch may not edit. Signed: the target compares it with `cmpwi`,
// not `cmplwi`, which only happens for a signed comparand.
// ---------------------------------------------------------------------
static inline s8 &scroll_flag_ref(dAcPy_c *p) {
    // 0x153c: read in getActScrollInfo and getScrollNum.
    return *reinterpret_cast<s8 *>(reinterpret_cast<u8 *>(p) + 0x153c);
}

// ---------------------------------------------------------------------
// getYoshi's inner dispatch: `lwz r12,0x60(this); lwz r12,0x6c(r12); mtctr
// r12; bctrl`, then `lbz`+`extsb` the returned pointer's byte 0. This is
// fBase_c's own vtable, whose pointer sits at object offset 0x60 (not 0)
// per HANDOFF.md ("fBase_c's vtable pointer is at object offset 0x60, not
// 0"); the call at raw slot 0x6c returns a small-int reference dereferenced
// right after -- the exact shape of `dActor_c::getPlrNo()`
// (`virtual s8 &getPlrNo() { return mPlayerNo; }`, d_actor.hpp:105), and
// "does the yoshi's rider match plrNo" is exactly what getYoshi needs.
//
// Calling it BY NAME (`((dActor_c*)base)->getPlrNo()`) reproduces getYoshi's
// 39 instructions exactly, confirming the identification -- but it also
// makes MWCC instantiate a local weak `getPlrNo__8dActor_cFv` (0x8 bytes) in
// THIS object, because getPlrNo is defined inline in the class body. The
// target has no such symbol: the gap between getYoshi and getYoshiNum is
// exactly 4 padding bytes (`gap_03_8005FAFC_text`, size 0x4), too small to
// hold it, and `grep -rn getPlrNo` over target_text.txt's whole 0x8005E9A0-
// 0x80061310 range is empty. So the real TU reached this vtable slot
// WITHOUT ODR-using the named method -- reproduced here as a raw, untyped
// vtable-slot fetch instead. See the writeup below for the two candidate
// sources and the exact tradeoff.
// ---------------------------------------------------------------------
typedef s8 &(*GetPlrNoFn)(dActor_c *);
static inline GetPlrNoFn get_vfunc_6c(fBase_c *base) {
    return (*(GetPlrNoFn **)((u8 *)base + 0x60))[0x6c / 4];
}

daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (get_vfunc_6c(base)((dActor_c *)base) == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}

int daPyMng_c::getYoshiNum() {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]) != nullptr) {
            count++;
        }
    }
    return count;
}

daYoshi_c *daPyMng_c::getYoshiDirectP(int idx) {
    return (daYoshi_c *)fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[idx]);
}

dAcPy_c *daPyMng_c::getCtrlPlayer(int plrNo) {
    dAcPy_c *player = getPlayer(plrNo);
    if (player == nullptr) {
        return nullptr;
    }
    daYoshi_c *yoshi = player->getRideYoshi();
    if (yoshi != nullptr) {
        return (dAcPy_c *)yoshi;
    }
    return player;
}

dPyMdlMng_c::ModelType_e daPyMng_c::getCourseInPlayerModelType(u8 idx) {
    static const dPyMdlMng_c::ModelType_e scModelTypeDt[4] = {
        (dPyMdlMng_c::ModelType_e)0,
        (dPyMdlMng_c::ModelType_e)1,
        (dPyMdlMng_c::ModelType_e)2,
        (dPyMdlMng_c::ModelType_e)3,
    };
    PLAYER_TYPE_e type = mPlayerType[idx];
    if (mCreateItem[type] & 0x8) {
        return (dPyMdlMng_c::ModelType_e)4;
    }
    return scModelTypeDt[type];
}

void daPyMng_c::setCarryOverYoshiInfo(u8 plrNo, u8 yoshiColor, int fruitCount) {
    m_yoshiColor[plrNo] = yoshiColor;
    m_yoshiFruit[plrNo] = fruitCount;
}

int daPyMng_c::getYoshiColor(u8 plrNo) {
    return m_yoshiColor[plrNo];
}

int daPyMng_c::getYoshiFruit(u8 plrNo) {
    return m_yoshiFruit[plrNo];
}

int daPyMng_c::getActScrollInfo() {
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scroll_flag_ref(player) != 1) {
                    u8 bit = 1 << i;
                    mask |= bit;
                }
            } else {
                u8 bit = 1 << i;
                mask |= bit;
            }
        }
    }
    return mask;
}

int daPyMng_c::getScrollNum() {
    u8 count = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scroll_flag_ref(player) != 1) {
                    count++;
                }
            } else {
                count++;
            }
        }
    }
    return count;
}
```

## Findings and contradictions (report, not reconciled)

### 1. `getCtrlPlayer` contradicts MAP.md row 25 — the disassembly is the tiebreaker

MAP.md row 25 describes `getCtrlPlayer` as: "returns null when riding a yoshi
(not 'prefer the yoshi')". The disassembly says the opposite. Tracing
`0x8005FBB8`-`0x8005FBCC`:

```
bl getRideYoshi__7dAcPy_cFv
cmpwi r3, 0x0
beq   .L_8005FBC8      ; rideYoshi == null -> go return the player
b     .L_8005FBCC      ; rideYoshi != null -> fall straight through,
                        ;   returning r3 AS-IS = getRideYoshi()'s own value
.L_8005FBC8:
mr r3, r31              ; r31 = the player
.L_8005FBCC:
... epilogue, returns r3
```

When `getRideYoshi()` returns non-null (the player IS riding a yoshi), the
`beq` is **not** taken, and execution falls through to an unconditional
branch that returns r3 **unchanged** — i.e. the yoshi pointer itself, not
`0`. So the real behaviour is: *return the ridden yoshi if riding one,
otherwise return the player, otherwise (no player) return null* — this is
literally "prefer the yoshi", the exact case MAP.md says was rejected.

This is corroborated (not just my own reading): `d_a_player_demo_manager.cpp`
calls `getCtrlPlayer()` dozens of times and, per SHARED-BRIEF, "treats a null
return as 'no controllable player'" — that part is undisturbed, since null is
still only returned when there is no player at all. Only the
riding-a-yoshi case differs from MAP's description. The body above compiles
byte-for-byte against the target (20/20 instructions, confirmed via
`diff_fn`) using exactly this "prefer the yoshi" logic, so I'm treating the
disassembly as authoritative and flagging MAP.md's row as needing a
correction, per the brief's "report contradictions" rule.

### 2. `getCourseInPlayerModelType`'s bit test is `& 0x8`, not "bit 2"

MAP.md row 26 says "bit 2 set → returns 4". The actual instruction is
`rlwinm. r0, r0, 0, 28, 28` — PowerPC MSB0 bit 28 is value `0x8`
(`0x80000000 >> 28`), not `0x4`. Implemented as `mCreateItem[type] & 0x8`;
compiles byte-exact (16/16 instructions). Flagging the discrepancy since the
row's English description undercounts by one bit position — possibly just
loose phrasing in the original recon rather than a real error, but noting it
per the brief's rule.

### 3. `getYoshi`'s vtable-@0x6c call: identified, but naming it costs a stray symbol

Full reasoning is in the source comment above; summary:

- **Identification.** `HANDOFF.md` documents that `fBase_c`'s vtable pointer
  sits at object offset `0x60` (not `0`), with slot index `(offset-8)/4`.
  `dActor_c::getPlrNo()` (`d_actor.hpp:105`, `virtual s8 &getPlrNo() { return
  mPlayerNo; }`) returns a reference to a signed byte — exactly the shape
  `getYoshi` dereferences (`lbz`+`extsb` on the vtable call's return value,
  compared against the `int plrNo` argument). Calling it by name
  (`((dActor_c*)base)->getPlrNo() == plrNo`) reproduces **all 39
  instructions of `getYoshi` exactly**, including the pooled-symbol-free
  vtable/slot immediates — about as strong a confirmation as a body can give
  without a name in the symbol map.
- **The cost.** Because `getPlrNo` is defined inline in the class body,
  MWCC's `-ipa file` mode instantiates a local weak copy
  (`getPlrNo__8dActor_cFv`, `0x8` bytes, `.text`, vis `0xD`) in *any* TU that
  calls it by name — confirmed by compiling this exact source and reading
  `dtk elf info` on the result. The target has no such symbol anywhere in
  its `0x8005E9A0`-`0x80061310` range (checked by grep), and the physical gap
  between `getYoshi` and `getYoshiNum` in the target is only `0x4` bytes
  (`gap_03_8005FAFC_text`) — not enough room for it. If this extra symbol
  ends up in the assembled TU, it would shift every function after it in the
  whole unit, which is a much bigger failure than a two-instruction diff
  inside one function.
- **What's in the deliverable instead.** `get_vfunc_6c()` above performs the
  identical two-`lwz`-then-`bctrl` dispatch via an untyped function-pointer
  cast rather than a named call. Compiling it in isolation produces **only**
  the `getYoshi` symbol (verified with `dtk elf info` — no stray weak
  copy), matches the target's total instruction count (39) and function size
  (`0x9C`), and differs from the target in exactly 2 of 39 instructions:

  ```
  want: lwz r12, 0x60(r3)      got: lwz r4, 0x60(r3)
  want: lwz r12, 0x6c(r12)     got: lwz r12, 0x6c(r4)
  ```

  Every register-shuffling and expression-restructuring lever I tried
  (moving the load into its own statement vs. a single expression, a
  `static inline` helper, reusing one variable across both dereferences,
  `u32`/`u8*`/`void**` casts, a `register ... asm("r12")` hint, a genuine
  pointer-to-member-function call) still picked `r4` for the first load —
  `r12` for *both* loads appears to be a fixed convention baked into MWCC's
  dedicated virtual-call lowering, not reachable through hand-written
  pointer-chasing C++. I did not find a source form that gets both properties
  (clean diff *and* no stray symbol) at once.
- **My call for the deliverable:** use the untyped-cast version. Its defect
  is local and non-propagating (same instruction count, same `0x9C` size, so
  nothing downstream shifts); the named-call version's defect is a real
  symbol that would corrupt the rest of the unit's layout if landed as-is.
  If the lead wants to try the named-call version in the context of the full
  65-function TU (rather than my isolated 10-function compile) to see if the
  weak-copy behaviour differs, the source is:

  ```cpp
  daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
      fBase_c *base;
      for (int i = 0; i < 4; i++) {
          base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
          if (base != nullptr) {
              if (((dActor_c *)base)->getPlrNo() == plrNo) {
                  return (daYoshi_c *)base;
              }
          }
      }
      return nullptr;
  }
  ```

  (Everything else in this batch is independent of this choice.)

### 4. Two register-allocation levers worth recording for other batches

- **`getYoshi`:** declaring the search-result pointer (`fBase_c *base`)
  *outside* the loop (reused each iteration) rather than as a fresh
  loop-scoped local was the difference between register swap (`r29`/`r30`
  reversed from target) and an exact match. Loop counter stays loop-scoped;
  only the "found this iteration" pointer needed hoisting.
- **`getActScrollInfo` / `getScrollNum`:** the target does **not** fold the
  `player == nullptr || field != 1` condition into shared code reached from
  two branches — it duplicates the "set bit" / "increment" statement once
  per source-level `if`/`else` arm. Writing it as a single `||` condition
  compiles to *shorter*, folded code (confirmably wrong instruction count);
  writing it as `if (player != nullptr) { if (field != 1) {...} } else
  {...}` — cond-true arm containing the nested check, `else` handling the
  null case — reproduces the target's un-folded, arm-duplicated layout
  exactly. Getting the `if`/`else` arm order backwards (null-check as the
  primary `if`) puts the null-case code first instead of last, which is also
  wrong.
- **`getScrollNum`'s return** truncates through a `u8` local
  (`clrlwi r3, r30, 24` in the target) even though the function returns
  `int` — the counter had to be declared `u8 count` to reproduce that
  instruction; a plain `int count` compiles one instruction short.

## Data objects emitted, with section

| Object | Section | Size | Notes |
|---|---|---|---|
| `@LOCAL@getCourseInPlayerModelType__9daPyMng_cFUc@scModelTypeDt` | `.rodata` | `0x10` | Confirmed via `dtk elf info` on the compiled draft: address `0x0` within `.rodata`, size `0x10`, the section's *only* object (matches SHARED-BRIEF's "one object only" claim). Contents `{0x00000000, 0x00000001, 0x00000002, 0x00000003}` — **4-byte elements** (`lwzx`, not `lbzx`), confirmed against `target_rodata.txt:608-615` byte-for-byte. Declared as a function-local `static const dPyMdlMng_c::ModelType_e[4]` inside `getCourseInPlayerModelType`, per the brief. |

No other `.data`, `.sdata`, `.sdata2`, `.bss`, or `.sbss` objects are touched
by this batch — every access in these 10 functions is either a plain member
array indexed by argument, or the one `.rodata` array above. `mPlayerType`,
`mCreateItem`, `mPlayerEntry`, `m_yoshiID`, `m_yoshiColor`, `m_yoshiFruit` are
all class members declared in the (frozen) header; none of them are defined
here.

## Not touched, not authored

- `scroll_flag_ref`'s target field (`daPlBase_c`'s undeclared byte at
  `0x153c`) is read-only in this batch and not claimed as a named member —
  consistent with MAP.md row 30's "undeclared daPlBase_c field" note and the
  brief's rule against editing shared headers.
- `get_vfunc_6c`'s helper and the `GetPlrNoFn` typedef are file-scope,
  `d_a_player_manager.cpp`-local plumbing, not class members — nothing here
  needs a `syms.txt` entry.
