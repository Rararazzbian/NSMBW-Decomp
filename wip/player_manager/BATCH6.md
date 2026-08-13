# Batch 6 — `0x80060250`–`0x800608DF`, 11 functions

All 11 functions were compiled with `mwcceppc.exe` (the exact `SHARED-BRIEF.md`
command line, via `tools/auto_decomp/harness.compile_draft`) and disassembled
with `dtk-windows-x86_64.exe`, then diffed instruction-by-instruction against
`wip/player_manager/target_text.txt` using `harness.diff_fn`. Nothing here is
claimed MATCHING unless the diff printed nothing, stated explicitly below.

## Status table

| # | Function | Result | Notes |
|---|---|---|---|
| 1 | `incCoin__9daPyMng_cFi` | **NEAR-MATCH** (not byte-exact) | Same control flow, same callees, same statement order as the target; 134 draft instructions vs 130 target. Two unresolved codegen gaps, both isolated below. |
| 2 | `addRest__9daPyMng_cFiib` | **NEAR-MATCH** (not byte-exact) | Instruction count now exact (74 = 74). Every remaining diff line is a uniform +1 register-number shift in the clamp section (mine: r4/r5/r6/r7, target: r3/r4/r5/r6) — logic, branch polarity, and instruction shape are all identical. |
| 3 | `incRestAll__9daPyMng_cFb` | **MATCHING** | Diff printed nothing (18 instructions). |
| 4 | `decRest__9daPyMng_cFi` | **PROVEN HEADER DEFECT — not fixable under the current header** | See "Header contradiction" below. Body is otherwise correct; the target's return value doesn't fit the header's declared `bool`. |
| 5 | `addScore__9daPyMng_cFii` | **MATCHING** (modulo the anon-namespace name, see below) | Diff printed nothing except the anonymous-namespace symbol suffix, which differs only because my scratch file is named `draft.cpp` — the mangled name embeds the *TU filename* (`@unnamed@draft_cpp@` vs `@unnamed@d_a_player_manager_cpp@`). Resolves automatically once the code lands in the real `.cpp`. |
| 6 | `setCourseInStarBGM__9daPyMng_cFv` | **MATCHING** | Diff printed nothing. |
| 7 | `startStarBGM__9daPyMng_cFv` | **MATCHING** | Diff printed nothing. |
| 8 | `stopStarBGM__9daPyMng_cFv` | **MATCHING** | Diff printed nothing. |
| 9 | `startMissBGM__9daPyMng_cFi` | **NEAR-MATCH** (not byte-exact) | 24 = 24 instructions; only 2 of 24 differ, and only in *which register* holds the intermediate pointer (target keeps everything in r12, mine spills through r4 first). Same shape, same offsets, same call. |
| 10 | `startYoshiBGM__9daPyMng_cFv` | **MATCHING** | Diff printed nothing. |
| 11 | `stopYoshiBGM__9daPyMng_cFv` | **MATCHING** | Diff printed nothing. |

6 of 11 are proven byte-exact matches. The other 5 are documented below with
exactly what's still open, rather than claimed as matches.

## Header contradiction — `decRest` return type

`d_a_player_manager.hpp` declares `static bool decRest(int);`. The target's
body sets **no return value at all on one path** (falls through with `r3`
left holding an address, not a canonicalised `0`/`1`) and returns a **raw
`rest - 1`** (an arbitrary small int, not a boolean) on the other. A true
`bool`-returning function would need to canonicalise that int to `0`/`1`
before returning (an extra `neg`/`or`/`srwi` sequence), and the target does
not have those instructions.

I proved this empirically, not just by reading the disassembly: I shadow-copied
`d_a_player_manager.hpp` into scratch with `decRest`'s return type changed to
`int`, compiled the identical body against that shadow copy, and it is a
byte-exact match — 36/36 instructions, confirmed via `harness.diff_fn`
(`MATCHING (36 instructions)`). Against the real (unedited) header, the same
body must declare `bool`, which is a lie the compiler enforces with a real
canonicalisation sequence the target doesn't have — that's what shows up as
the diff.

**I did not edit the real header** (per the brief). The source below defines
`bool daPyMng_c::decRest(int)` to compile against the header as it stands
today, and will not byte-match until the header's declared return type is
fixed to `int`. Recommend the lead change:
```cpp
static bool decRest(int);
```
to
```cpp
static int decRest(int);
```
in `include/game/bases/d_a_player_manager.hpp`.

## Other findings

- **`addRest`'s declared `bool` return is also never set** (matches the
  target, which likewise sets no return value on any path — confirmed by the
  single compiler warning I got, `(10184) return value expected`, pointing at
  `addRest`'s closing brace). Unlike `decRest`, this one doesn't block a
  match, because "return nothing" is exactly what the target does too — no
  canonicalisation is needed either way. Flagging alongside `decRest` since
  it's the same underlying phenomenon (header return types are CFront
  guesses); it just happens not to matter here.
- **`incCoin`'s and `addRest`'s shared `dBgParameter_c` midpoint block reads
  as `xStart() + xSize() / 2` / `yStart() - ySize() / 2`**, not `* 0.5f` —
  even though the pooled `.sdata2` constant is a plain `0.5f` (MWCC's
  strength-reduction of `/2` produces that same pooled reciprocal, and the
  *operand order* in the resulting `fmuls` only comes out right when the
  source is written as a division). Confirmed by comparing against
  `d_multi_manager.cpp`'s `setClapSE()`, which uses the identical
  `xStart() + xSize() / 2` shape against the same `dBgParameter_c` fields —
  a strong precedent, not a guess.
- **The clamp comparisons in `incCoin`/`addRest`/`addScore` are all
  `>=`, not `>`.** i.e. `if (candidate >= max) candidate = max;`. Written as
  `>` the branch polarity comes out backwards against the target on all
  three.
- **`addScore`'s player-index bound check is an unsigned compare** —
  `(u32)plrNo <= 3`, confirmed by the target's `cmplwi`/`cmplwi`-style
  branch. Written as a plain signed `plrNo <= 3` (matching the header's
  declared `int` parameter) it compiles to `cmpwi`, which doesn't match.
  Cast at the comparison site rather than changing the header's declared
  parameter type.

## Unresolved codegen gaps (reported, not papered over)

**`incCoin` — shared base-pointer address folding.** The target materialises
`m_playerID__9daPyMng_c@ha`/`@l` into `r31` **unconditionally in the
prologue**, even though `incCoin` never actually reads `m_playerID`, and then
reaches `mPlayerEntry` (`r31+0x40`), `mPlayerType` (`r31+0x50`), and `mCoin`
(`r31+0x90`) all as offsets from that one register — all three are in the
same `.bss` region as `m_playerID`, in address order (see
`d_a_player_manager.hpp`'s own address-order comment block). My draft emits a
separate `lis`/`addi` pair per array instead, 4 extra instructions worth. I
could not find a source-level way to make MWCC choose `m_playerID` as the
shared anchor — every phrasing I tried (named locals, `getPlayerType()`
helper calls, if/else polarity swaps) reproduced the *logic* exactly but left
this specific address-CSE decision to the compiler's own cost model. This
looks like a whole-function `-ipa file` heuristic, not something the source
shape controls directly. Flagging for whoever has bandwidth to run the
deterministic sweep in `tools/auto_decomp/harness.py` against it, or to spot
the missing lever.

**`incCoin` — the `getEntryNum() > 1` comparison.** The target compiles this
specific comparison to a 6-instruction branchless sequence
(`xori`/`srawi`/`and`/`subf`/`srwi.`/`beq`) that I verified, instruction by
instruction, is mathematically equivalent to `getEntryNum() > 1` for every
non-negative input — but writing `> 1`, `>= 2`, or the logically-equivalent
`!isEntryNum1()` (the header's own inline helper, `getEntryNum() == 1`
negated — **not actually equivalent to `>1` for `getEntryNum()==0`, so I did
not substitute it**) all compile to a plain `cmplwi`/branch instead. I don't
have a source form that reproduces this shape and didn't want to guess one
that might be semantically different just to chase bytes. Reporting rather
than reaching for something I can't justify.

**`addRest` — uniform register-number shift in the clamp section.**
Instruction count and everything else in the function (the two `dAudio`
sound cues, the `changeItemKinopioPlrNo` call, branch polarity) is
byte-identical. Only the final clamp-and-store sequence differs, and only by
register number (my r4/r5/r6/r7 where target has r3/r4/r5/r6) — every operand
role, addressing mode, and instruction opcode matches. I tried: a named
`PLAYER_TYPE_e type` local, calling the header's `getPlayerType()` accessor
twice instead of indexing `mPlayerType[]` directly (this is what got the
instruction *count* to match, up from 71), and separating `scRestMax` into
its own named `max` local. None shifted the register numbers closer. Likely
downstream of the same whole-function allocation pressure as the `incCoin`
base-pointer issue above.

**`startMissBGM` — one register differs in a 2-instruction vtable-slot
chase.** Target: `lwz r12, 0x60(r3)` then `lwz r12, 0xe0(r12)` (chained
entirely through r12). Mine: `lwz r4, 0x60(r3)` then `lwz r12, 0xe0(r4)`
(first load lands in r4, not r12). I tried removing every intermediate named
local (a `typedef`'d function-pointer-array struct pointer, then a raw
double-`reinterpret_cast` with no local at all) and got the identical result
both times — this looks like a fixed MWCC scheduling choice for indirect-call
argument chains, not something the C++ shape controls.

## Data objects emitted, by section

**`.sdata` — the three anonymous-namespace constants (owned by this batch
per `BATCHES.md`).** Confirmed landing at `0x80427C00`–`0x0C`, in the required
order `scRestMax`, `scCoinMax`, `scScoreMax`, each `0x4` bytes:

```cpp
namespace {
    int scRestMax = 99;
    int scCoinMax = 99;
    int scScoreMax = 999999999;
}
```

**This is a real, confirmed finding, not a formality: they must NOT be
declared `const`.** I first wrote them exactly as the brief and `MAP.md`
suggested — `static const int scRestMax = 99;` etc. — and at `-O4` MWCC
constant-folded every use site into bare immediates (`cmpwi r3, 0x63`,
`lis r5, 0x3b9b; subi r0, r5, 0x3601` for `999999999`, and so on): the three
symbols were **not emitted at all**, exactly the failure mode the brief
warned about. Dropping `const` (keeping them as plain, non-const,
file-scope-anonymous-namespace `int`s with an initialiser) fixed it — the
compiler now genuinely loads them via `@sda21`, matching the target's `lwz
r0, "scCoinMax__32@unnamed@d_a_player_manager_cpp@"@sda21(r0)` and friends in
every call site across `incCoin`, `addRest`, and `addScore`. Confirmed
values match the target's `target_sdata.txt`: `scRestMax`/`scCoinMax` =
`0x63` (99), `scScoreMax` = `0x3B9AC9FF` (999,999,999).

(In my scratch compiles, the mangled suffix reads `@unnamed@draft_cpp@`
because the scratch file is literally named `draft.cpp`; landed under the
real filename it will read `@unnamed@d_a_player_manager_cpp@` as the target
does — already confirmed for the other two `.sdata`-referencing functions
above, where that's the *only* line that differs.)

**No other new data objects.** `incCoin`'s two-cue block and `startMissBGM`'s
vtable-slot chase reference only already-attributed pool literals (the
shared `0.5f` at `@80832_8042BD78`, owned per the brief's own resolution) and
raw immediate offsets (`0x1070`, `0x1554`, `0x60`, `0xe0`) into
still-undecompiled `dAcPy_c` — no new `.rodata`/`.data`/`.sdata2` symbols are
introduced by this batch.

## `dAcPy_c` raw-offset casts (out of scope, flagged not invented)

`stopStarBGM` reads an undeclared 4-byte field at `dAcPy_c` offset `0x1070`
(compared against `0x3c`, a timer of some kind). `startMissBGM` chases a
function-pointer slot at `*(p+0x60)+0xE0` (called with `p` itself as the sole
argument — not a this-adjusting virtual thunk, since `r3` at the call site is
still the original player pointer, unmodified) and then reads a 4-byte field
at offset `0x1554`. `dAcPy_c`/`daPlBase_c` are not part of this unit and
their headers carry no annotations anywhere near these offsets, so per
`SHARED-BRIEF.md`'s own guidance ("reach for a cast only when the field
belongs to a class that has genuinely not been decompiled, and then say so"),
these are raw offset casts with an explanatory comment, not invented member
names.

## Source

See the reply for the full `wip/player_manager` batch 6 source (all 11
functions plus the three `.sdata` constants and the local `Unk60Vtbl_c`
helper type used only by `startMissBGM`).
