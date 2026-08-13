# Work order for Codex — round 10

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 10.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is yours.

---

## Round 9 verdict

**Task A is closed properly and cheaply, which is what I asked for.** You went
and measured the narrow idea instead of arguing about it: removing the inline
destructor body from a shadow header makes MWCC emit real destructor calls at
scope exits, and `incCoin` grew 9 instructions, `addRest` 9,
`deleteCullingYoshi` 12. A dead end with numbers attached is a finished
question. Taking the fallback.

**Task B produced a usable ranking, and I am going to act on part of it.** But
three things in it need correcting before round 10, and two of them matter.

### Correction 1 — the unit was already assembled; you taxonomised the wrong baseline

Your taxonomy was built from `BATCH1.md`…`BATCH8.md`, which are the **isolated
per-function drafts**. The unit has since been assembled into
`wip/player_manager/assembled.cpp` and verified as a whole TU, and that result
lives in **`wip/player_manager/ASSEMBLY.md`**. That file is the authority. It
says **44 of 65 byte-exact**, so there are **21 near-misses, not 23**.

That is my fault for not pointing you at it, and it is the single most useful
file in the directory for this question.

### Correction 2 — "12 of 23 self-resolve at assembly" is already measured, and it mostly did not

You predicted assembly would take roughly 12 functions green. Assembly had
already happened, and it went **41 → 44**. Of those three, only **`decRest`** was
a real fix; the other two (`addScore`, `fn_80060DB0`) were **comparator naming
artifacts** — the function was always byte-identical and the by-address lookup
was looking under the wrong name.

Specifically: **all seven of your "register allocation only" functions are still
near-missing in the whole-TU compile.** `fn_8005f570`, `getYoshi`, `getCoinAll`,
`addRest`, `startMissBGM`, `deleteCullingYoshi`, `setYoshiPriority` — every one
is in ASSEMBLY.md's near-miss table with its register differences intact. The
isolated-compile-artifact theory is a real mechanism, but it does not cover this
class, and assembling the file has already spent whatever it was going to pay.

**This is the lesson I want carried into round 10: measure against the assembled
whole-TU compile, never against an isolated single-function draft.** The gap
between those two is exactly what round 9 fell into.

### Correction 3 — `decRest` is already fixed

`include/game/bases/d_a_player_manager.hpp:54` reads `static int decRest(int);`
and `assembled.cpp:993` reads `int daPyMng_c::decRest(int plrNo)`. It was
changed before round 9 and it already matches. Your re-derivation of the
return-type diagnosis was independently correct — it is just not remaining work.

### One class you should stop counting as open

The `m_playerID` / `SYM0` / `...bss.0` relocation-naming differences —
`initGame`, `initStage`, `getNumInGame`, `fn_8005f4d0`, and the naming component
of `incCoin` / `checkCorrectCreateInfo` / `setHipAttackQuake`. ASSEMBLY.md
traces these to the raw disassembly and is right about the cause: **dtk names a
`.bss` relocation out of its own object's symbol table, and an unlinked `.o` has
no external reference to justify keeping the name.** Assembly is not linking, so
assembling could never have fixed it. Settling it needs a real project link,
which is mine to run and forbidden to you. **Treat those as not-a-defect and do
not spend round 10 on them.**

---

## Round 10: close three functions. No taxonomy.

You have the ranking; I agree with the top of it. Now produce byte-exact source.

Ground truth is `wip/player_manager/target_text.txt`. Read `ASSEMBLY.md` first.

### The method I want, and it is not the round-9 method

**Compile the whole of `assembled.cpp`**, with your edit in it, and diff the
function out of that object. Do not compile a one-function draft. Shadow-copy
`assembled.cpp` into your own scratch directory and edit the copy —
`wip/` is read-only to you.

Use `tools/auto_decomp/harness.py`'s `compile_draft` / `extract` / `diff_fn`.
Extract **by address** and assert `instruction_count * 4` against the symbol map.

### 1. `createCourseInit` — the biggest payoff, and it gates a second thing

345 draft vs 352 target instructions, 7 short. Two levers are already recorded
and neither has been tried in the assembled file:

- the frame-layout lever in `wip/player_manager/SHARED-BRIEF.md` — hoist one
  `mVec3_c` local to function scope instead of one per branch;
- the `action ∈ {0,1}` range-fold and the bool-materialisation idiom that
  B2's own report flags.

**`getFileP` is your progress gauge, and it is also the second thing this
gates.** `getFileP__5dCd_cFi` is a 32-byte foreign weak copy the target places
at `0x8005EE70` inside our range. It is currently **never emitted at all**,
because our `createCourseInit` is small enough for MWCC to inline it. When
`createCourseInit` reaches its true size the call should become a real `bl` and
`getFileP` should appear. If you get the function byte-exact and `getFileP` is
still absent, say so loudly — that would mean the coupling theory is wrong.

### 2. `incCoin` — 130 target / 126 draft

Four instructions short. ASSEMBLY.md calls it a base-pointer folding gap plus a
branch-structure difference. Your own read was that the extra instructions are
separate array-address materialisation around the `getEntryNum() > 1` branchless
idiom. Those two readings are compatible; test both.

### 3. `checkCorrectCreateInfo` — 105 target / 103 draft

Same instruction count in your reading, two short in ASSEMBLY.md's — **that is a
contradiction between two reports and I want it resolved by measurement, not by
picking one.** Beyond the count, the substance is that the target hoists `.sdata`
loads for `scRestMax` / `scCoinMax` / `scScoreMax` while ours folds them to
immediates. Test whether declaring them as `const int` file-scope objects rather
than letting them fold produces the target's loads.

### Acceptance

Byte-exact against `target_text.txt`, measured in a whole-`assembled.cpp`
compile. **A near-miss reported honestly with the exact remaining diff is a
perfectly good result** — three functions is a lot to ask and I would rather
have one closed and two precisely characterised than three hand-waved.

Report, per function: the final source, the measured diff (or MATCH), and
whether the whole-TU `.text` size moved toward `0x2A10`.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (read it freely, shadow-copy what you need, write
  nothing), `HANDOFF.md`, `AGENT_CONTEXT.md`, `CODEX_PROMPT.md`, or any
  `GEMINI_*.md`. Gemini is on `m_pad.cpp` and `d_multi_mng.cpp`; stay out of both.
- I am authoring `d_nand_thread.cpp` this round with my own agents — `wip/nand_thread/` is not yours.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
