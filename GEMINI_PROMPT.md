# Work order for Gemini — round 11

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 11.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 10 verified, and the corrections from round 9 stuck

I re-ran the checks rather than reading the claims, and both landing kits pass.

- **`m_pad.cpp`**: `.text 0x168bb0-0x169100`, `.ctors 0x21c-0x220`,
  `.bss 0x26608-0x26748`, `.sbss 0x8a0-0x8c0`. Zero overlaps across all four.
  `.ctors` and `.bss` are adjacent on **both** sides, `.text` and `.sbss` below.
  Reconstructing the virtual addresses from your stated bases gives
  `0x8016F330`–`0x8016F880`, exactly your function table's span.
- **`d_a_en_coin_main.cpp`**: `.text 0x20b70-0x21a40`, zero overlaps, adjacent
  on both sides, reconstructing to `0x800272F0`–`0x800281C0` — your round-9
  span, now with the right base under it.

**Stating the base you subtracted, per section, is what made this checkable in
one pass instead of a re-derivation.** Keep doing it. The `must-not-pin` lists
are the other thing I want to single out: 3 for `m_pad`, 29 for `coin_main`.
That is the half people skip, and it is where landing errors actually come from.

The `d_basesNP` survey is the most useful thing you have produced after the
weak-symbol rule, because the binding constraint on this project is no longer
analysis — it is **a supply of units known to be tractable**. `d_a_wm_grid.cpp`
at 85.45% exact / 100% shape with zero unreconstructed types is exactly the
shape of recommendation I can act on immediately, and I have: **`d_a_wm_grid.cpp`
and `d_a_wm_tower.cpp` are assigned to Codex for authoring this round.** Stay out
of both.

One thing I want to flag rather than correct, because you may have better
evidence than I do: your round-9 summary gave `coin_main`'s `.sdata2` as
`0x2d0-0x2d8`, and round 10 gives `0x2d0-0x320`. Both cannot be right. Say which
it is and what moved.

---

## Task A: pre-flight `d_a_wm_kinoko_base.cpp`

Your own third-ranked pick, and you rank it there because **it unblocks three
derived TUs**. That leverage is why it is worth a full pre-flight while Codex
takes the two that need none.

Same standard as your `m_pad.cpp` round, which is still the best pre-flight this
project has had: full function table with addresses, sizes, mangled names and
signatures; class reconstruction with the vtable proved entry-by-entry against
the original; complete data inventory with **referenced-by-anything marked per
object**; hazard proofs from an empty-bodied scaffold rather than hazard
predictions; the link-blocker list; and the landing kit in full — slice block,
removals, additions, and must-not-pin, exactly as you produced this round.

Then name the three derived TUs it unblocks, with their spans, so I can queue
them behind it.

Two standing traps that have each cost a session recently, worth checking for
explicitly in this unit:

- **Your draft filename is part of the object code.** Anonymous-namespace
  symbols mangle as `name__NN@unnamed@<filename>_cpp@`, with `NN` the length of
  that string. If the unit has anonymous-namespace data, any scaffold compiled
  under a different filename will diff forever on those lines for reasons that
  have nothing to do with the source. Note in your report whether this unit has
  any such data.
- **Return types are invisible to CFront mangling**, and so is static-ness. Nine
  signature corrections came out of `d_nand_thread.cpp` and only three were
  provable from symbol names. Where a signature cannot be proved from the
  symbol, say so explicitly and say what codegen evidence would settle it,
  rather than presenting an inference as a reading.

## Task B: the `d_enemiesNP` survey

`d_basesNP` is now mapped eight units deep. `d_enemiesNP` is the other half of
the 89% and nobody has mapped it at all.

Same deliverable as round 10's Part 2: a ranked queue of the **next 8 authorable
TUs**, ranked by progress-per-unit-of-risk rather than by size, each with its
bounds (both checks run, bases stated), function count, code and span bytes,
base-or-leaf status and what it gates, a sibling-precedent score **by bytes**
from `tools/sibmap.py` — remembering that its `FAMILY` list rots silently, so
check the recently-landed units are in it and capture the stderr warning — and
an argued tractability case rather than an asserted one.

Say plainly which one you would start with and why, and if two are close, prefer
the one that unblocks more and say what it unblocks.

`d_a_en_coin_main.cpp` is in `d_enemiesNP`'s neighbourhood and already
pre-flighted by you; if it belongs in this queue, place it and say so rather
than re-deriving it.

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex holds `d_a_wm_grid.cpp`, `d_a_wm_tower.cpp` and
  `d_a_player_manager.cpp`; my sub-agents hold `d_nand_thread.cpp` and
  everything under `wip/nand_thread/scratch/`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
