# BATCH B5 — `0x8005FDB0`–`0x8006024F`, 12 functions

Verified with the harness's own compile/disasm/diff (`tools/auto_decomp/harness.py`
`compile_draft` / `disasm` / `diff_fn`), never `ninja`/`configure.py`/`progress.py`.
Compiled against a **shadow copy** of `d_a_player_manager.hpp` in scratch (per
SHARED-BRIEF's "shadow-copy it into your scratch, prove the change there, and
report it" instruction) — the real header was not edited. Two functions needed a
shadow correction to reach a byte-exact diff; see "Header contradictions" below.
`diff_fn` reports `MATCH` only when the canonicalised instruction stream is
identical, including registers — no false pass.

## Status table

| # | Address | Size | Function | Params | Status |
|---|---|---|---|---|---|
| 1 | `0x8005FDB0` | `0x74` | `addNum__9daPyMng_cFi` | **1** (`int`) | **MATCHING** — diff printed nothing |
| 2 | `0x8005FE30` | `0x7C` | `decNum__9daPyMng_cFi` | **1** (`int`) | **MATCHING** — diff printed nothing |
| 3 | `0x8005FEB0` | `0x18` | `addNum__9daPyMng_cFv` | **0** | **MATCHING** — diff printed nothing (as `void`, see below) |
| 4 | `0x8005FED0` | `0x18` | `decNum__9daPyMng_cFv` | **0** | **MATCHING** — diff printed nothing (as `void`, see below) |
| 5 | `0x8005FEF0` | `0xB8` | `getNumInGame__9daPyMng_cFv` | 0 | **NOT MATCHING** — logic verified, register/anchor mismatch only (see below) |
| 6 | `0x8005FFB0` | `0x58` | `getEntryNum__9daPyMng_cFv` | 0 | **MATCHING** — diff printed nothing |
| 7 | `0x80060010` | `0x74` | `getItemKinopioNum__9daPyMng_cFv` | 0 | **MATCHING** — diff printed nothing |
| 8 | `0x80060090` | `0x74` | `getItemKinopio__9daPyMng_cFv` | 0 | **MATCHING** — diff printed nothing |
| 9 | `0x80060110` | `0x5C` | `getPlayerIndex__9daPyMng_cF13PLAYER_TYPE_e` | 1 | **MATCHING** — diff printed nothing |
| 10 | `0x80060170` | `0x64` | `changeItemKinopioPlrNo__9daPyMng_cFRi` | 1 (`int&`) | **MATCHING** — diff printed nothing (as `bool`, see below) |
| 11 | `0x800601E0` | `0x14` | `getCourseInListPlrNo__9daPyMng_cFi` | 1 | **MATCHING** — diff printed nothing |
| 12 | `0x80060200` | `0x50` | `getCoinAll__9daPyMng_cFv` | 0 | **NOT MATCHING** — logic verified, register-permutation only (see below) |

**10 / 12 byte-exact.** The overload quartet (rows 1–4) is fully confirmed
distinct — four different bodies, four different mangled names, matched
independently, in target address order, exactly as the brief asked:
`addNum(int)` → `decNum(int)` → `addNum()` → `decNum()`.

## The overload trap — resolved

All four bodies are genuinely distinct (confirmed by compiling and diffing each
independently against its own address range in `target_text.txt`), matching
MAP.md's own independent finding. No shared key, no duplicate-body bug on this
batch.

- `addNum__9daPyMng_cFi` — **1 param** (`int`), `0x74` bytes, sets a per-player
  bit in `mActPlayerInfo`, falls back to `addNum()` if the player isn't already
  carrying a Kinopio.
- `decNum__9daPyMng_cFi` — **1 param** (`int`), `0x7C` bytes, mirror of the
  above (clears the bit), always calls `decideCtrlPlrNo()` afterward.
- `addNum__9daPyMng_cFv` — **0 params**, `0x18` bytes, clamped `++mNum` (cap 4).
- `decNum__9daPyMng_cFv` — **0 params**, `0x18` bytes, clamped `--mNum` (floor 0).

## Header contradictions — reporting, not editing

Per SHARED-BRIEF and my own batch brief, `d_a_player_manager.hpp` was **not**
edited. Both findings below were proven against a shadow copy in scratch
(`…/scratchpad/b5/include/game/bases/d_a_player_manager.hpp`), by compiling both
ways and diffing against the target bytes.

1. **`addNum()` / `decNum()` are `void`, not `bool`.** The header (line 100–101)
   declares `static bool addNum(); static bool decNum();`. Neither target body
   ever sets `r3` explicitly — both just fall off the end after the
   conditional increment/decrement, reusing whatever register held the `mNum`
   temp. Compiled as declared (`bool`, no return statement), MWCC still
   compiles it (`(10184) return value expected` warning, not an error) but
   allocates the `mNum` temp into **r4** instead of **r3**, because a
   non-void function keeps r3 provisionally reserved for its eventual return
   value even on a fall-through path. Compiled as `void`, the temp lands in
   r3 and the diff is byte-exact. Confirmed both ways, both functions.
   **The header needs `bool` → `void` on both declarations for a true
   byte-exact landing of this pair.**
2. **`changeItemKinopioPlrNo(int&)` is `bool`, not `void`.** The header
   (line 13) declares `static void changeItemKinopioPlrNo(int &);`. The
   target explicitly sets `r3 = 0x1` on the success path and `r3 = 0x0` on
   the failure path, both immediately before `blr` — the same
   explicit-both-arms pattern the header's own comment (on `fn_8005f4d0`)
   already documents as proof of a non-void return. Compiled as `bool`, the
   diff is byte-exact. **The header needs `void` → `bool` here.**

These are the two return-type corrections this batch needed. Both are
consistent with the header's own guidance ("A wrong return type is invisible
to every symbol comparison and shows up only in the body — correct them from
the bodies") but SHARED-BRIEF and this batch's own brief say the header is
complete and not to be edited, so this is reported for the lead's header pass,
not applied.

## Not-matching functions — reported, not forced

Both remaining functions have **verified-correct logic** (every load, compare,
shift and add is present, in the same order, computing the same value) but
differ from the target in **register allocation only** — the SHARED-BRIEF /
harness system prompt is explicit that a register difference is a real failure,
so these are reported as NOT MATCHING rather than papered over.

- **`getNumInGame__9daPyMng_cFv`** (`0xB8`, 46 canonical instructions both
  sides). Target computes ONE base address — `lis r4, m_playerID@ha; addi r4,
  r4, m_playerID@l` — and reaches `mPlayerEntry` (`+0x40`), `mPlayerType`
  (`+0x50`) and `mRest` (`+0x80`) from it via plain `addi` offsets, even though
  `m_playerID` itself is never read in this function. Every source shape tried
  (plain `for`, `while`, `u8`-typed index, calling the header's
  `getRest`/`getPlayerType` inline accessors instead of raw array access)
  still materialises **three separate** `lis`/`addi` base pairs (one per
  array) when compiled in isolation. Given `m_playerID` is the earliest
  `.bss` field of the class and is otherwise untouched here, this reads as a
  whole-TU codegen artifact — most plausibly MWCC picking whichever `.bss`
  symbol serves as anchor based on state accumulated from OTHER functions in
  the same file that DO touch `m_playerID` directly (B4 territory:
  `getYoshiDirectP`, etc.) — not reproducible from a single-function isolated
  compile. Flagging for the lead to re-check once the whole TU is assembled;
  if it still doesn't resolve at that point, that is new information.
- **`getCoinAll__9daPyMng_cFv`** (`0x50`, 20 canonical instructions both
  sides). Target avoids `r3` for every intermediate value and only writes it
  once, on the final `add r3, r5, r0` before `blr`; every source shape tried
  (compound `total +=`, a straight-line sum expression, a real `for` loop,
  reversed-associativity parenthesisation, pre-loading `mPlayerType[0..3]`
  into named locals) puts `r3` into service immediately for one of the two
  address bases. Same loads (`mPlayerType[0..3]`, `mCoin[...]` four times),
  same `slwi`/`lwzx`/`add` sequence, same order — a pure register permutation,
  not a logic difference. Confirms the brief's documented indexing scheme
  (`mCoin` indexed by `mPlayerType[i]`, not by slot) is correct; the shape
  is:
  ```cpp
  int daPyMng_c::getCoinAll() {
      return mCoin[mPlayerType[0]] + mCoin[mPlayerType[1]] + mCoin[mPlayerType[2]] +
             mCoin[mPlayerType[3]];
  }
  ```

## Data objects

**None.** This batch introduces no new `.data`, `.rodata`, `.sdata`, `.sdata2`,
`.bss` or `.sbss` objects — every symbol touched (`mActPlayerInfo`, `mNum`,
`m_playerID`, `mPlayerEntry`, `mPlayerType`, `mRest`, `mCoin`,
`mCourseInList`) is a pre-existing static member already declared in the
header and already covered by an earlier batch's `.bss` accounting (B1/shared
recon). No pooled literals (no floats, no strings) are referenced by any of
these twelve functions.

## Contradictions found (reporting per the "report, don't reconcile" rule)

1. Header return types wrong for three functions — see "Header contradictions"
   above (`addNum()`, `decNum()`, `changeItemKinopioPlrNo(int&)`). Not
   reconciled; not edited.
2. `getItemKinopio()`'s header-guessed return type `dAcPy_c *` is **confirmed
   correct** by the body — no contradiction, but confirming it since the
   header explicitly flagged it as an unverified guess. The function returns
   the exact pointer `getPlayer(i)` produced, not a bool/count, and `getPlayer`
   is declared to return `dAcPy_c *`.
3. No contradiction found in `getCourseInListPlrNo(int)` or `getPlayerIndex
   (PLAYER_TYPE_e)` — both matched on the first structurally-reasonable
   attempt with no signature surprises.

## Verification method note

All 12 functions were compiled individually (and the whole batch together, as
delivered below) via `harness.compile_draft` → `harness.disasm` →
`harness.diff_fn`, using the real MWCC (`compilers/Wii/1.1/mwcceppc.exe`) and
`bin/dtk-windows-x86_64.exe`, against `wip/player_manager/target_text.txt`.
Nothing was run against the shared build; no `slices/wiimj2d.json` or
`syms.txt` edit was made. Scratch files (shadow header, draft, object,
disassembly) live entirely under
`…/scratchpad/b5/` in this session's temp directory, outside the repo.
