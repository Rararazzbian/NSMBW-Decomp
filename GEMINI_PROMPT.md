# Work order for Gemini — round 3

**`AGENT_CONTEXT.md` is the standing briefing** — rules, tooling, evidence
hierarchy, MWCC gotchas. This file is only round 3.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 2 verdict: both tasks landed, and Task A was the best work so far

**The `startSystemSe` deadlock is broken and in the tree, all five binaries
byte-identical.** I had tried this, failed, backed it out, and written it up as a
dead end that would need "its own piece of work". It did — and you did it.

What made it work was that you **audited the binary instead of reasoning about
the code**. All seven call sites target `FUiUl`; none targets `FUlUl`. So the
default resolution was right — but it was right *by accident*, and nobody had
checked, which is exactly the kind of unexamined assumption that turns into a
four-binary failure later.

And you found the fix was **not in the header at all**. An `int` converts to
`unsigned int` and to `unsigned long` at identical overload-resolution rank, so
with both overloads visible nothing can resolve. Fixing the *arguments* — three
`(u32)` casts, and `const int SoundEffects[]` → `const u32` — resolves everything
and emits the same bytes. **I explicitly asked you to check for that root cause
before proposing casts, and you did, and it was there.** Landed exactly as
proposed, plus the `syms.txt` pin at `0x801954B0`.

One detail worth noting for your model of the build: it passed *before* I added
the pin, because a declared-but-uncalled function needs no definition. The pin
only starts mattering when `d_a_player_manager.cpp` lands and actually calls it.

Task B's shortlist is good and I am acting on it below.

---

## Round 3: turn `d_nand_thread.cpp` into a ready-to-author unit

You ranked it first: `0x800CED00`–`0x800CFCE0`, 24 functions, 4,064 B, **169 B
per function**, **zero `__sinit`s**, hard-bracketed on both sides in
`dtk_splits_wiimj2d.txt` by `d_multi_manager.cpp` and `d_next.cpp`. That is the
cleanest candidate anyone has produced for this project, and the bytes-per-
function figure is less than half the worst candidate on the list.

`d_a_player_manager.cpp` is close to landing. **I want to start this one the
moment it does, without a reconnaissance round.** Produce the pre-flight.

### What I need, in this order

**1. The class, from its vtable.** `__vt__13dNandThread_c` is at `0x80317D48`.
With slot offsets computed, **the vtable IS the class declaration**: it gives the
base class, which base virtuals are overridden, and the new virtuals *in
declaration order*. Remember `(vtable size - 8) / 4` = the slot count, and that
for `fBase_c`-derived classes the vtable pointer sits at object offset **0x60**,
not 0 — check which applies here before computing anything.

There is a second vtable in range: `__vt__6mMutex` at `0x80317D60`. Establish
whether `mMutex` is a separate class this TU also defines, or a member. If
`dNandThread_c` embeds an `mMutex` by value, its `sizeof` is load-bearing — that
exact hazard is what made the current unit hard.

**2. The function table.** All 24, in address order, with mangled name, address,
size, and one line on what each does. Flag any that are unnamed in the map
(`fn_XXXXXXXX`) and say whether each is a class member or a file-scope static —
**CFront mangling does not mark static members**, so the test is whether `r3`
holds a real argument rather than a `this`.

**3. Section bounds and a complete data inventory.** All eight sections, each
labelled with how it was derived and how strong that is. Use
`dtk_splits_wiimj2d.txt` for hard brackets.

Then the part that actually decides whether a unit lands: **every data object in
those ranges, with an explicit note on whether any function in the range
references it.** On the last two units, *every* defect that blocked the link was
in data placement, and none was visible to a per-function diff. Two specific
traps to check for by name:

- **An object nothing references is still ours to emit.** A 0x40 float table that
  the entire binary never reads still failed a link when it was left out.
- **An object dtk labels as padding may be real.** `setHipAttackQuake` reads and
  writes three ints that dtk calls `gap_..._bss`. Because they sat inside a
  larger gap, *no bounds check could have caught it* — only a function touching
  them did.

**4. SDK dependencies.** You flagged NAND SDK calls as the risk. Enumerate every
external function the range calls, and for each say whether it is (a) already
defined by a banked TU, (b) declared in a header but undecompiled — needing a
`syms.txt` pin at its original address, or (c) not declared anywhere, needing a
header addition. Category (b) is what fails a link with an undefined symbol, and
it is the single most common blocker on this project.

### What would make this round a failure

Telling me it looks clean. I already believe that — you established it in round
2. What I need is the **specific list of things that will go wrong**, because the
last two units were both "clean" right up until the link.

If some part cannot be settled from the binary, say which part and what would
settle it. `AGENT_CONTEXT.md` §4 has the two occasions where an honest "I could
not tell" was worth more than an answer.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (agents are working `d_a_player_manager.cpp` there),
  `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`, or any `CODEX_*.md`.
  Codex is on link-blocker analysis and `EGG::Effect`; do not enter either.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
