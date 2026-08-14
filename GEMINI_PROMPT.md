# Work order for Gemini — round 10

**`AGENT_CONTEXT.md` is the standing briefing.** It gained four entries and a
new "two checks" section since round 9, one of which comes directly from your
round-9 output. This file is only round 10.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Task A was excellent and it is landed

The weak-symbol placement rule is settled, at a scale I did not expect: 143
units audited, 1,592 weak symbol instances classified into deadstripped (10%),
deduplicated to a home TU (51%), and surviving-definition-in-slice (38%). The
part that makes it a rule rather than an observation is the boundary — you
proved both halves, including three banked units where a weak symbol *is* the
surviving definition and does occupy its slice, which is exactly what stops the
rule being over-applied.

The `keepWeak` / `FORCEACTIVE` mechanism read from `tools/gen_lcf.py` is the
other half nobody had written down: weak symbols are excluded from `FORCEACTIVE`
by default, which is *why* the deadstripping happens, and `keepWeak` is the
escape hatch when the retail binary kept an unreferenced weak symbol anyway.

Your `d_a_player_manager.cpp` accounting — `0x80` deadstripped, `0x64`
deduplicated, `getCourseIn__10dScStage_cFv` at `0x8` genuinely in-slice, net
overflow zero — is what I needed. **The unit is unblocked and reassigned to
Codex for authoring this round.** Your text is going into `HANDOFF.md` close to
as written.

## Task B: the numbers are right, one line of arithmetic is wrong

The function table, the vtable decode (4 differing slots, zero new virtuals),
`sizeof(daEnCoinMain_c) == 0x8C8`, and the 40.41% exact / 49.40% shape sibling
score are all good work, and the sibling number settles the "blockmain just
landed, this will be cheap" intuition with a measurement instead of an opinion.

**But §2.9's `.text` range is wrong, and it would have collided at landing.**
You proposed `"0x232f0-0x241c0"`. I ran the overlap check and it sits inside
`dol/bases/d_a_en_dfpakkun.cpp`'s existing claim of `0x21a40-0x243c0`.

The cause is one subtraction. Your own §2.1 gives the span as
`0x800272F0`–`0x800281C0`, and `0x800272F0 - 0x232F0 = 0x80004000` — that is
**`.init`'s base address, not `.text`'s `0x80006780`.** Every other section in
your proposal used the right base and is exactly adjacent to a neighbour, which
is what made the single wrong one so easy to miss.

Corrected, and I have verified it: **`.text` is `0x20b70-0x21a40`**, size
`0xED0`, zero overlaps, ending exactly where `d_a_en_dfpakkun.cpp` begins and
beginning exactly where `d_a_en_carry.cpp` ends. Same size you computed —
only the base was wrong.

Both checks that catch this are now standing instructions in `AGENT_CONTEXT.md`:
run the overlap-and-adjacency sweep on every proposed range, and **state the
base you subtracted, per section**, so the arithmetic is checkable without
being redone.

---

## Task A: landing kits for the two pre-flighted units

Neither `m_pad.cpp` (your round 8) nor `d_a_en_coin_main.cpp` (your round 9) can
be landed from a pre-flight alone — each still needs the integrator half. That
half is mechanical, it needs no build, and producing it is the single thing that
most shortens the path from your work to a verified binary. I did it for
`d_nand_thread.cpp` this morning; the method is written up in `HANDOFF.md` under
"The landing kit, pre-computed". Follow it exactly and produce, for **both**
units:

1. **The slice block**, with every range passed through the overlap-and-adjacency
   check and the base you subtracted stated per section.
2. **The `syms.txt` removals** — every symbol currently pinned whose address
   falls inside any of the unit's ranges. Once our object defines a symbol,
   pinning it to an address is a contradiction.
3. **The `syms.txt` additions** — derived from the target's own relocations, not
   from reading the source. Parse the relocations out of the disassembly,
   subtract the symbols the TU defines itself (`@NNNNN` pool objects, `@LOCAL@`
   statics, and its `__vt__` tables are all ours), then test each survivor's
   symbol-map address against every landed slice's `.text` range. What is left
   needs a pin.
4. **The must-not-pin list** — symbols the TU references that a landed slice
   already defines. This is the half people skip and it is where the errors are.
   A worked example from `d_nand_thread.cpp`: it calls seven `OS*` mutex and
   condition-variable functions, four of which are defined by the landed
   `lib/revolution/os/OSMutex.c` and must NOT be pinned, while the three
   condition-variable ones sit at `0x801b3280`–`0x801b3370`, outside that
   slice's `.text` claim, and must be. **"The file is landed" is not the same as
   "the symbol is defined."**

Your round-9 §2.8 already lists 4 pins for coin_main; re-derive them by the
relocation method rather than reusing that number, and tell me if it changes.

## Task B: survey `d_basesNP`, because that is where the work actually is

Nobody has mapped it. 89% of everything left in this project lives in
`d_basesNP` and `d_enemiesNP`, both still around 1–2% complete, and the DOL
units we keep landing move the headline number slowly by construction. I am
authoring units one at a time out of a queue that is nearly empty; the
constraint on this project is now **a supply of units known to be tractable**,
and finding those is the thing you are best at.

Produce a ranked queue of the **next 8 authorable TUs in `d_basesNP`**. For each:

- Its boundaries in every section, both checks run.
- Function count, code bytes, span bytes.
- Whether it is a base class or a leaf, and what it gates if it is a base.
- **Its tractability, argued rather than asserted.** The useful signals are:
  precedent from a landed sibling scored by bytes rather than by name (use
  `tools/sibmap.py`, and note its `FAMILY` list rots silently — check the
  recently-landed units are in it and capture its stderr warning, as you did
  last round); how many distinct external types it needs that are not yet
  reconstructed; whether its data inventory contains anything unclaimed; and
  whether any of its functions are large enough to be register-allocation
  gambles rather than transcription.
- Anything that would make it a bad first choice — an unreconstructed base
  class, a `.bss` object of unknown size, a vtable that does not decode cleanly.

Rank by **progress-per-unit-of-risk**, not by size, and say plainly which one
you would start with and why. If two are near-equivalent, prefer the one that
unblocks the most other units, and say what it unblocks.

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is authoring `d_a_player_manager.cpp` this round —
  stay out of that unit entirely. My own sub-agents hold `d_nand_thread.cpp` and
  everything under `wip/nand_thread/scratch/`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
