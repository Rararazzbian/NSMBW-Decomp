# Batch 8 — `0x80060F20`–`0x80061304` (6 functions)

Deliverable per `SHARED-BRIEF.md` / `BATCHES.md`. Source is in the reply;
this file carries per-function status and the data-object report.

## Verification method

Every function was compiled standalone with `tools/auto_decomp/harness.py`
(`compile_draft`/`disasm`/`diff_fn`, the same functions the shared loop
uses) against `wip/player_manager/target_text.txt`, not eyeballed. Draft and
scratch scripts live in the session scratchpad, not under `wip/`.

One structural fact had to be discovered before any multi-array function
would diff cleanly: **`m_playerID`, `mPlayerType`, `mPlayerMode`,
`mCreateItem`, `mRest`, `mCoin` etc. are addressed in the target as a single
base (`m_playerID`) plus small constant offsets** (`+0x50`, `+0x60`, `+0x70`,
`+0x80`, `+0x90`), not via each array's own symbol. This only happens when
the compiler can see the out-of-line storage *definitions* of all of them in
the same compile job, in `.bss` order. My standalone test file therefore
needed scratch-only `int daPyMng_c::m_playerID[4]; …` definitions (in the
address order read from `target_bss.txt`) purely to reproduce this folding —
**those definitions are not part of the deliverable**; whoever assembles the
final `.cpp` needs exactly one copy of each, contiguous, in that order.

## Per-function status

| Function | Status | Notes |
|---|---|---|
| `isEffectStop(int)` | **MATCHING** (23/23 instructions, diff printed nothing) | |
| `isAcceptQuake(int)` | **MATCHING** (9/9 instructions, diff printed nothing) | Turned out to be `return checkPlayer(plrNo);` — see finding below. |
| `initYoshiPriority(daPlBase_c*)` | Near match, NOT claiming MATCHING | 45/46 instructions identical, same order. One line differs: target's `cmpw r3, r0` vs my `cmpw r0, r3` — a symmetric equality test, cosmetic operand-order only (I tried every source-level operand ordering and cast combination; none changed it — see below). |
| `setYoshiPriority(daPlBase_c*)` | Near match, NOT claiming MATCHING | 36/38 instructions identical, same order, same branches. The two loop-invariant locals (`&m_yoshiID[0]` and the saved old priority) land in `r30`/`r31` swapped relative to target, plus the same cosmetic `cmpw` operand-order artifact as above. |
| `isCreateBalloon(int)` | Near match, NOT claiming MATCHING | Same instruction *set*, same field accesses, same final values — but target places the "return true" block before the "return false" block (bit-test jumps forward a short distance; the `mRest` check's failure path is the one pushed to the end), while my compile places them in the opposite order with the branch polarity flipped to match. Every rephrasing I tried (nested-if, De Morgan `&&`, named `bool result`, `||` short-circuit) produced byte-identical output to each other, which is itself informative: MWCC treats them as one canonical form here, so the discrepancy is not about how the boolean logic is phrased. |
| `checkCorrectCreateInfo()` | Near match, NOT claiming MATCHING | Once the `mCoin` bug (below) was fixed, the diff is exactly the address-folding artifact from `scRestMax`/`scCoinMax`/`scScoreMax` (owned by B6, not defined in my test file — see below) plus the register numbers that shift as a direct consequence. With those three constants stubbed `volatile` (to force a real load instead of the constant-fold my tiny test file performs but the target does not), every remaining line is either identical or a pure register-letter swap. |

**Zero functions are guessed** — all six compiled and were diffed against
the real disassembly; the four "near match" rows are reported as such
per the brief's explicit rule, not rounded up to MATCHING.

## A real bug this caught: `mCoin` is indexed by `mPlayerType[i]`, not by `i`

`checkCorrectCreateInfo`'s final clamp does **not** zero `mCoin[0..3]`
directly. The target's block is:

```
addi r3, r31, 0x50        ; &mPlayerType[0]
lwz  r0, 0x50(r31)        ; mPlayerType[0]
lwz  r4, 0x4(r3)          ; mPlayerType[1]
addi r5, r31, 0x90        ; &mCoin[0]
lwz  r8, 0x8(r3)          ; mPlayerType[2]
...
stwx r6, r5, <mPlayerType[i]*4>   ; mCoin[mPlayerType[i]] = 0, for i=0..3
```

i.e. `mCoin[mPlayerType[0]] = mCoin[mPlayerType[1]] = mCoin[mPlayerType[2]]
= mCoin[mPlayerType[3]] = 0;` — indexed by *character type*, exactly matching
the header's own comment on `mCoin` ("Indexed by `PLAYER_TYPE_e`, not by
player slot") and `getCoinAll()`'s own addressing. My first draft wrote the
naive `mCoin[0]=mCoin[1]=mCoin[2]=mCoin[3]=0;` and it compiled and even
*looked* plausible (same instruction count in the wrong shape) until diffed
against the real disassembly — the fixed version reproduces the target's
five-register interleaved-`slwi`/`stwx` pattern instruction-for-instruction
except for register letters.

## Finding: `isAcceptQuake` is a trivial wrapper around `checkPlayer`

The header already declares (as an existing inline):
```cpp
static bool checkPlayer(u8 plrNo) { return mActPlayerInfo & (1 << plrNo); }
```
`isAcceptQuake(int plrNo)` compiles, byte-for-byte, to exactly
`return checkPlayer(plrNo);` (MWCC inlines it — no `bl`). This explains the
`clrlwi r0,r3,24` truncation to `u8` (the implicit `int`→`u8` argument
conversion) and the `neg/or/srwi 31` boolify tail (that's `checkPlayer`'s own
`bool` return canonicalising the raw `&` result) that `MAP.md`'s "Rejected
candidates #4" flagged as "too generic to name one twin" — it isn't a
generic idiom coincidence, it's this specific call, inlined.

## Finding: field `0x1036` on `daPlBase_c` is the yoshi-priority rank byte

Read/written by both `initYoshiPriority` and `setYoshiPriority`, nowhere
else in this batch. Undeclared in the frozen header, so per this project's
established convention (`d_a_player_demo_manager.cpp`'s `field_38c_ref`
etc.) I added a file-scope `static inline u8 &yoshiPriorityRef(daPlBase_c*)`
helper using `reinterpret_cast`, not a header edit.

## Finding: field `0x38e` on `daPlBase_c`/`dAcPy_c` is `dActor_c::mExecStopMask`

This one **is** already declared (`include/game/bases/d_actor.hpp:376`,
`u8 mExecStopMask`), immediately followed by `mLayer` at `0x38f` — which the
`d_a_player_demo_manager.cpp` batch had already independently pinned at
`0x38f` via its own `field_38c_ref` comment ("not `dBc_c::mLayer` — that's
confirmed at `0x38f`"). The two findings corroborate each other exactly.
`isEffectStop` reads `dActor_c::mExecStop & player->mExecStopMask`, both
real named members, no raw-offset cast needed for this one.

## Finding: field `0xafc` on `dInfo_c` is still undeclared

`isEffectStop`'s very first check is a byte read at `dInfo_c::getInstance()
+ 0xafc`. `d_info.hpp`'s current layout has this deep inside `pad11[0x712]`
(the padding block before `mFukidashiActionPerformed`), so it is not yet
resolvable to a named member. Used via a scratch-local
`static inline u8 &infoField_0xafc_ref(dInfo_c*)` helper, not a header edit.
Reporting for whoever eventually decompiles `d_info.hpp`'s neighbourhood.

## Return-type / signature notes

- `initYoshiPriority`/`setYoshiPriority`: still genuinely `void` — neither
  function ever writes `r3` on any exit path. The header's guess stands.
- `isEffectStop`/`isCreateBalloon`: both `bool` per the header. Neither shows
  the `neg/or/srwi` bool-canonicalisation tail, but that's expected — both
  are structured as explicit `return true;`/`return false;` branches, which
  compile identically for `bool` and `int` destinations; this batch's bodies
  can't distinguish the two (unlike `dScStage_c::m_goalType`'s case, which
  needed a raw arithmetic return to show the tail).
- `checkCorrectCreateInfo`: `void`, matches header, no incoming/outgoing
  register activity beyond `blr`.

## Data objects

**No new named data object is defined by this batch** — matching the
brief's expectation that B8 owns none. For the record, everything this
batch's functions *reference* is already accounted for elsewhere:

- `m_playerID`, `m_yoshiID`, `mPlayerType`, `mPlayerMode`, `mCreateItem`,
  `mRest`, `mCoin` — `.bss`, already declared in the frozen header, storage
  definitions not yet placed by any batch (see the folding note above; the
  lead needs one contiguous block of `daPyMng_c::<member>` definitions in
  `.bss` address order for any of B4/B5/B6/B7/B8's multi-array functions to
  assemble correctly as a whole file).
- `scRestMax`, `scCoinMax`, `scScoreMax` — `.sdata`, owned by **B6** per
  `BATCHES.md`. `checkCorrectCreateInfo` reads all three; I did not redefine
  them in the deliverable.
- `mKinopioMode`, `mScore` — already declared, no new storage.

## Contradiction report (not reconciled, per house rule)

`STATICS.md` describes `scRestMax`/`scCoinMax`/`scScoreMax` as file-scope
anonymous-namespace **`const int`**. In my isolated standalone compile, a
`const int` (and even a plain non-`const` `int`) with no writes anywhere in
the (tiny) test file gets **fully constant-folded** into an immediate
(`cmplwi r6,0x63`, `lis+subi` for the 9-digit one) — but the target
disassembly demonstrably does a real `lwz …@sda21` load, hoisted once above
the unrolled loop, never folded to an immediate. Marking my scratch stand-ins
`volatile` reproduces the real *load*, but then over-conservatively defeats
the loop-invariant *hoist* the target still performs (I get two loads inside
the loop instead of one hoisted above it) and forces a second reload for the
`mScore`/`scScoreMax` compare-then-store pair that the target satisfies from
one already-loaded register. Neither knob available to me reproduces the
target's exact behaviour ("real load, still hoisted, still reused") in a
6-function file — this is very likely a whole-translation-unit artifact
(`-ipa file` sees only 6 functions in my test vs. all ~67 in the real file,
and/or the presence of writes to these globals elsewhere that I don't have),
not a defect in the referencing code. Reporting rather than reconciling,
per the brief. Every *other* difference in `checkCorrectCreateInfo`'s diff
(register letters, one branch displacement) is a direct downstream
consequence of this single artifact, not an independent problem.

## Section bound

This batch owns the last function in the unit. `checkCorrectCreateInfo` in
my draft compiles to `0x1A4` bytes (`105` canonicalised instructions incl.
the constant-fold-driven `volatile` artifact's extra load; the semantically
equivalent target size is also `0x1A4`), matching `target_text.txt`'s
`# .text:0x27C0 | 0x80061160 | size: 0x1A4` exactly, ending at `0x80061304`
— consistent with the brief's stated `.text` upper bound
(`0x8005E9A0..0x80061310`, with `0x80061304..0x80061310` as the closing
`gap_03_80061304_text` padding this batch does not author).
