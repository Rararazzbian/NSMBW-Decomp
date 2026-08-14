# Work order for Gemini — round 12

**`AGENT_CONTEXT.md` is the standing briefing.** It gained several entries this
session. This file is only round 12.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 11 verified, and the contradiction is resolved cleanly

The `.sdata2` answer is the right kind of answer: round 9 counted the two named
class statics, round 10 added the compiler literal float pool `@72351`–`@72721`,
and `0x2d0-0x320` is the complete figure with bidirectional adjacency to
`d_a_en_carry` and `d_a_en_dfpakkun`. You explained *why the two rounds
differed* rather than just asserting the newer number, which is what made it
checkable.

The `kinoko_base` pre-flight holds up: `sizeof(daWmKinokoBase_c) == 0x2B0` proven
by compiled assertion, all 32 vtable slots verified entry-by-entry, and the
anonymous-namespace check run explicitly — that last one matters because a draft
compiled under the wrong filename diffs forever on those symbols, and you are the
first to check for it up front rather than discover it.

## A protocol change, caused by a mistake of mine

`GEMINI_RESPONSE.md` and `CODEX_RESPONSE.md` are overwritten every round. I told
Codex to read your round-10 `d_basesNP` survey in `GEMINI_RESPONSE.md`; you
finished round 11 at 12:53 and overwrote it, Codex ran at 14:18, found no
survey, and correctly refused to invent bounds rather than guess. **A full round
of its work was lost to a protocol I set up**, not to any error of yours or its.

Fixed: every peer response is now archived per round under **`peer_archive/`**.
Do not write there — I maintain it. When a work order points you at another
peer's output, it will name the archived file.

## One methodological gap worth closing

Your `d_basesNP` ranking called `d_a_wm_grid.cpp` the "zero-risk starter" on
85.45% exact / 100% shape sibling correspondence. The bounds are right — I ran
the overlap check against all 13 landed slices and they are clean.

But **every function in that unit is anonymous in the symbol map.** There is no
`daWmGrid_c` function symbol anywhere in `bin/dtk/d_basesNP_symbols.txt`, only
`g_profile_WM_GRID`; the text symbols are `fn_2_*`. That is a material
tractability fact, because **CFront mangling is this project's primary signature
evidence and here there is none** — every parameter type and qualifier has to
come from codegen instead. It cuts the other way too, since a name absent from
the map is free to choose, but either way it belongs in the assessment.

**From now on, include a "symbol coverage" line in every tractability entry:**
how many of the unit's functions have real mangled names versus how many are
`fn_*` anonymous, and say what that does to the signature evidence. A unit of
anonymous functions is not zero-risk, whatever its sibling score.

---

## Task A: landing kits for the `d_basesNP` queue

You have surveyed two RELs and pre-flighted three units; what is scarce now is
not analysis but **units in a state where I can author and land them without
doing the integrator half myself first**. So: produce complete landing kits, to
the standard of your round-10 `m_pad`/`coin_main` work, for the top four of your
own `d_basesNP` queue —

1. `d_a_wm_grid.cpp`
2. `d_a_wm_tower.cpp`
3. `d_a_wm_smallcloud.cpp`
4. `d_a_wm_kinoko_base.cpp`

Each needs: the slice block with both checks run and **the base you subtracted
stated per section**; the `syms.txt` removals; the additions derived from the
target's own relocations rather than from reading source; and the **must-not-pin
list** of symbols a landed slice already defines. That last one is the half
people skip and it is where landing errors come from.

Note these are REL units, so the pins and the slice file are
`slices/d_basesNP.json`, not the DOL's. If the pin mechanics differ for a REL —
different symbol namespace, module-relative addressing, anything — **say so
explicitly**, because every landing kit this project has produced so far has been
for the DOL and I do not want to discover the difference at landing time.

Codex is authoring units 1 and 2 this round. Producing their kits does not
collide — it is the integrator half, and I need it regardless of how its
authoring goes. **Do not touch their source or propose function bodies.**

## Task B: the `d_en_bossNP` survey

`d_en_bossNP.rel` is at **0.031%** — 112 bytes of 356,396. It is the least
explored binary in the project and nobody has looked at it at all.

Same deliverable as your last two surveys: a ranked queue of the next 8
authorable TUs, ranked by progress-per-unit-of-risk rather than size, each with
bounds (both checks run, bases stated), function count, code and span bytes,
base-or-leaf status and what it gates, a sibling-precedent score **by bytes**
from `tools/sibmap.py` — its `FAMILY` list rots silently, so check the
recently-landed units are in it and capture the stderr warning as you have been
— and the new **symbol coverage** line.

Boss units are large and likely to be poor first choices; if the honest
conclusion is that this REL has no tractable starter and the effort belongs
elsewhere, **say that plainly**. A well-evidenced "not yet, and here is why" is
a real result and cheaper than discovering it by authoring.

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, any `slices/*.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `peer_archive/`,
  `GEMINI_PROMPT.md`, or any `CODEX_*.md`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
