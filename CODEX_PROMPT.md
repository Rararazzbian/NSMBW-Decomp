# Work order for Codex — round 6

**Read `AGENT_CONTEXT.md` first** if you have not this session. It now holds all
the standing material — the rules, the tooling, the evidence hierarchy, the MWCC
behaviours that have cost people rounds. This file is only what is specific to
round 6.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is yours.

---

## Round 5 verdict: everything landed, and the refusal was the best part

`dPyEffect_c` and the `dInfo_c` split are **both in the tree, five binaries
byte-identical.** I probed `sizeof(dEf::followEffect_c)` independently before
applying and it is `0x114` exactly as you said, so the trailing fields land and
the `STATIC_ASSERT(sizeof(dPyEffect_c) == 0x13C)` now holds against a real
embedded member instead of a pad. `sizeof(dInfo_c)` stays `0xB5C`.

**Task B1 is the round's best result, and you got it by refusing.** I asked you
to name `daPlBase_c + 0x1036` as a Yoshi priority field. You found it is already
`mPlayerLayer` — a matched, referenced member — and reported the contradiction
rather than splitting a pad that does not exist.

You were right, and the resolution is now clear: `initYoshiPriority` and
`setYoshiPriority` **reuse `mPlayerLayer` as the priority rank**, which is
coherent, because a Yoshi's draw layer *is* its priority. So the raw cast goes
away and we gained a fact instead of a fabricated field. That is twice running
your most valuable output was declining to comply with something I asserted.

Also noted: you caught a four-byte arithmetic error in your own sub-agent's
`dInfo_c` report before sending it, and corrected the numbers in your response
rather than shipping the sub-agent's text. Keep doing that.

---

## Task A (primary): find the link blockers before the link does

This is on my critical path and it is the highest-value thing you can do this
round.

`d_a_player_manager.cpp` is being assembled right now. The next step is a **trial
link** — putting it into the build while some functions still do not match — and
the doctrine exists because on the previous unit it found **two independent
blockers that no per-function diff could see**, one of which was an undefined
symbol at link time.

**You can find those blockers ahead of time, from the symbol map alone.**

### What to produce

For **every function our TU calls**, decide which of three categories it is in:

1. **Defined by an already-banked TU** — fine, nothing needed.
2. **Defined by a TU that is still undecompiled** — then nothing in the link
   provides it, and it needs a **`syms.txt` entry pinning it to its original
   address**, or the link fails with an undefined symbol.
3. **A weak inline that our TU emits itself** — fine, and there are exactly two
   known ones (`getCourseIn__10dScStage_cFv`, `getFileP__5dCd_cFi`).

Deliver a table of every category-2 symbol with the exact `syms.txt` line it
needs: `<mangled_name>=0x<ADDRESS>`.

### How to do it

The call list is recoverable from the target disassembly itself —
`wip/player_manager/target_text.txt` is **read-only for you**, but reading it is
exactly right here. Every `bl <symbol>` in it is a call our TU makes. Extract the
unique set.

Then for each callee:
- Is it defined in `source/`? (`grep` for the demangled name; check the file is a
  matching slice in `slices/wiimj2d.json`, not `nonMatching` — **a `nonMatching`
  slice is not linked at all**, so a symbol it would define is still missing.)
- Is it already in `syms.txt`?
- If neither, it is a blocker. Give me the line.

### The trap that has bitten twice, so check for it explicitly

**`syms.txt` is a two-way trap.** A symbol needs an entry while its defining TU
is undecompiled, and the entry must be **deleted** the moment that TU lands, or
the link fails on a duplicate definition. So also report the reverse: **any
existing `syms.txt` entry for a symbol that `d_a_player_manager.cpp` will now
define**, which must be removed when it lands. On the previous unit this was hit
twice, in both directions.

Do not edit `syms.txt`. Give me the lines to add and the lines to remove.

## Task B (secondary): `EGG::Effect`'s 0x110 of unnamed data

Only start this once Task A is reported.

`include/lib/egg/util/eggEffect.hpp` has a full virtual table reconstructed but
its data members are unnamed — `sizeof` is `0x114`, of which `0x4` is the vptr
and **`0x110` is unexplained**.

This is the base of the entire effect system. Dozens of actor TUs embed effects
by value, and you just proved `dPyEffect_c` does too. Every future unit that
touches an effect pays for this region being anonymous, so naming even part of it
compounds.

Attack it the way you attacked `dPyEffect_c`: the constructor and destructor tell
you the shape, and a member initialised by a single call at a fixed offset is
probably an embedded object rather than scalars. The many virtuals that take
`(value, ERecursive)` pairs will each touch a specific field — `setLife`,
`setEmitRatio`, `setEmitInterval`, `setPowerYAxis` and friends are effectively a
labelled map of the struct, one setter per field.

**`sizeof` must stay `0x114`** — it is pinned by `dPyEffect_c` and by every TU
that embeds an effect. Honest `u8 pad[N]` for what you cannot justify; partial
progress is a fine result here and this task has no deadline pressure.

---

## Reminders that apply every round

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (my agents are assembling there), `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `CODEX_PROMPT.md`, or any `GEMINI_*.md`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one. `AGENT_CONTEXT.md` §4 has the two
  cases where this saved the project from my own wrong assertions.
- Plain ASCII or clean UTF-8 in `CODEX_RESPONSE.md`, LF, no BOM.
