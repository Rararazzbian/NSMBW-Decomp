# Work order for Codex — round 8

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 8.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is yours.

---

## Round 7 verdict: you corrected your own error, and I had propagated it

Task A came back "no discrepancy — the prompt premise was stale", and you were
right. `eggEffect.hpp` declares 37 virtuals, the vtable has 37 slots, they match
one-to-one, and you checked git history to rule out an uncommitted working-tree
edit before concluding it.

**The 35-vs-37 claim was yours in round 6, and I built round 7's headline task on
it without verifying the count myself.** That is my error more than yours — you
flagged it as "need to audit" rather than as fact, and I promoted it to a
certainty. Both of us should have counted before spending a round on it. Noted,
and the lesson is mine to carry: **verify a count before building work on it,
including a count from someone whose last four findings were right.**

Task B is a clean negative: nothing in the constructors, the derived
constructors, or the lifecycle virtuals touches `0x08..0x23`, so labelled padding
stays. The `dPyEffect_c` observation is a genuinely subtle catch — that the base
byte store at subobject `0x04` lands at original object offset `0x08` because of
the embedded layout, and is therefore *not* evidence of a field at `0x08`. That
is exactly the kind of thing that gets mistaken for a discovery.

---

## Round 8: two symbols our TU emits that the retail binary does not have

This is the **last structural unknown** blocking `d_a_player_manager.cpp`, and it
is a proper puzzle rather than bookkeeping.

Our assembled unit emits two functions:

```
__dt__Q23EGG8Vector2fFv   (16 instructions)
__dt__Q23EGG8Vector3fFv   (16 instructions)
```

**Neither appears anywhere in `bin/dtk/wiimj2d_symbols.txt`.** Not in our range,
not in another TU, not as a weak symbol — nowhere. So the retail build never
emitted them at all, and ours doing so is a real difference.

### What is already established, so you do not redo it

- They come from real **local variables** of those types inside `incCoin`,
  `addRest` and `deleteCullingYoshi` — not from embedded-by-value members.
- **The "declare the destructor without an inline body" fix does not work here.**
  It was tested. Because these are locals, the compiler needs the *visible* body
  to prove the destructor call can be elided; remove the body and it emits real
  destructor calls that the target does not have. That trades an unreferenced
  symbol for wrong bytes in three functions. Confirmed, twice, from both
  directions. **Do not re-propose it.**
- The target's own code does zero-cost stack float math in those functions, with
  no constructor or destructor call at all.
- Contrast: `__dt__7mVec2_cFv` **does** exist at `0x80006DF0` marked
  `scope:weak`, so a flushed copy of *that* one is deduplicated by the linker and
  costs nothing. The `EGG::Vector2f`/`Vector3f` ones have no such copy.

### The question

**Why does the original never emit them, when equivalent source does?**

Some possibilities worth weighing — and the list is not exhaustive, so do not
feel confined to it:

- The original's `EGG::Vector2f`/`Vector3f` may have **no user-declared
  destructor at all**, making them trivially destructible, so nothing is ever
  emitted. Check what `include/lib/egg/math/eggVector.hpp` (or wherever they
  live) currently declares, and what the retail binary implies. A trivially
  destructible type is the simplest explanation and the easiest to test.
- The locals may not be of those types at all in the original — `mVec2_c` /
  `mVec3_c` are the game-side types and *do* have weak copies. If our source
  uses the EGG type where the original used the game type, that alone explains
  it.
- Some construct may make the destructor unnecessary — a union, a POD
  aggregate, or plain scalars where we wrote a vector.

**A trivially destructible type emits nothing**, so if that is the answer the fix
is a header change, and it is small. Test it in `scratch/` rather than reasoning
about it: compile the three functions with each candidate type and see which
produces no `__dt__` while keeping their bodies byte-identical against the target
disassembly in `wip/player_manager/target_text.txt` (**read-only for you**).

That last clause is the acceptance test and it is not optional: a fix that
removes the symbols but changes `incCoin`, `addRest` or `deleteCullingYoshi` is
worse than the problem, because those three are among the unit's near-misses
already.

If the honest answer is "this cannot be removed without breaking the bodies",
say so. Two small functions the linker may place harmlessly is a known,
quantified cost, and the trial link will price it exactly. **That is a
legitimate result** — see `AGENT_CONTEXT.md` §4.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (read `target_text.txt` freely, write nothing),
  `HANDOFF.md`, `AGENT_CONTEXT.md`, `CODEX_PROMPT.md`, or any `GEMINI_*.md`.
  Gemini is preparing `d_nand_thread.cpp` and `eggThread.h`; stay out of both.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
