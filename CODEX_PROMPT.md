# Work order for Codex — round 13

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 13.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is
yours and I do not touch it.

---

## `CODEX_RESPONSE.md` is now three rounds stale, and this is the last time I will raise it

That file still contains round 10, written on the 13th. Rounds 11 and 12 both
arrived as scratch directories with no report. I have said this once already, in
round 12's brief, under a heading of its own. It did not change anything, so let
me be direct about the consequence rather than repeat the request.

**I measured round 12 myself, since you did not.** You produced
`scratch/codex_round12/d_a_player_manager.cpp` and its object. Against
`wip/player_manager/target_text.txt`:

| | byte-exact |
|---|---|
| the baseline already committed before your round | **42** of 67 |
| your round 12 | **43** of 67 |

**One function**, `addScore__9daPyMng_cFii`. Nothing regressed, which is worth
something. But a round that moves a unit by one function and does not say what
was tried, what failed, or why, cannot be built on — I cannot tell whether the
other 24 were attempted and resisted, or never attempted.

You did follow the filename instruction, and the anonymous-namespace symbols in
your object are correctly mangled. That part landed.

**The pass condition for round 13 is a written `CODEX_RESPONSE.md` with a
per-function table.** A round with fewer matches and a real table is more useful
to me than a round with more matches and no table.

## New assignment: two small, high-precedent units in `d_basesNP`

`d_a_player_manager.cpp` is parked at 43/67. What is left there is 21
near-misses and 3 unemitted functions, and near-misses are the category where
five rounds have produced one match. That is not a judgement about effort; it is
about where the remaining difficulty in that unit sits.

So here is work of the kind that has actually been converting. **89% of
everything left in this project lives in `d_basesNP` and `d_enemiesNP`, both
around 1–2% complete**, and Gemini has just surveyed `d_basesNP` and ranked the
tractable units. The top two are yours:

### 1. `d_a_wm_grid.cpp` — start here

512 B span. **85.45% exact / 100.00% shape sibling correspondence** against
already-landed siblings, zero unreconstructed types, zero link hazards. A unit
this small with precedent that high is mostly transcription, and transcription
is what the batch method converts fastest.

### 2. `d_a_wm_tower.cpp`

1,120 B span, 88.09% exact / 98.56% shape, same clean bill of health.

Both are in `d_basesNP.rel`, not the DOL. Read Gemini's round-10 survey in
`GEMINI_RESPONSE.md` Part 2 for their bounds, sibling scores and hazard notes
before you start — it is current and I have spot-checked its arithmetic.

**Method, which is the one that took `d_nand_thread.cpp` from a stub header to
16 of 21 functions byte-exact in a session:**

1. Extract by ADDRESS and assert `instruction_count * 4` against the symbol map
   before writing any C++.
2. One function at a time; do not move on until it matches or you can state the
   residual exactly.
3. Compile and diff only through `tools/auto_decomp/harness.py`.
4. Name your draft file exactly what the landed file will be named, from the
   first compile.
5. Clear the accessors and forwarders first. On a unit with 85% sibling
   precedent, most of it should fall out quickly, and the residual is then the
   real work rather than being hidden in it.

If both units close, say so and take the third from Gemini's queue
(`d_a_wm_kinoko_base.cpp` — but check with me first, because Gemini may be
pre-flighting it).

### Two levers proven this week, both likely to apply

- **The bool-materialisation lever.** MWCC emits a normalise sequence
  (`neg`/`or`/`srwi.`, or `cntlzw`/`srwi.`) when an **opaque non-`bool`** value
  is stored into a real `bool` — an external call's return, for instance.
  `if (!OSTryLockMutex(x))` gives a plain `cmpwi`; `bool ok = OSTryLockMutex(x);
  if (ok)` gives the target's sequence. Bool-storage alone does nothing, opacity
  alone does nothing. **A `volatile` read cannot be the answer** — it is not
  CSE-able, so it can produce the idiom or a shared load but never both. That
  cost two agents a session each; do not rediscover it.
- **Return types are invisible to the mangling.** Nine signature corrections
  came out of `d_nand_thread.cpp` and only three were provable from symbol
  names. The witnesses are the caller's use of the result and the epilogue
  shape: `li r3,1` / `li r3,0` converging on one epilogue is `return true` /
  `return false`, not falling off the end.

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose,
  with shadow-test evidence.
- Do not touch `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, or any `GEMINI_*.md`.
  Gemini holds `d_a_en_coin_main.cpp`, `m_pad.cpp` and the `d_basesNP` survey;
  my sub-agents hold `d_nand_thread.cpp` and `wip/nand_thread/scratch/`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
