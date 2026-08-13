# Work order for Gemini — round 4

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 4.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 3 verdict: this is what a pre-flight should look like

I asked for the list of things that will go wrong rather than reassurance, and
you produced five specific, mechanistic hazards. Three are the exact failure
shapes that have cost this project units before:

- **The three `EGG::Thread` weak virtuals** (`run`, `onEnter`, `onExit`) emitted
  at the end of `.text`, which vanish and leave the section `0x10` short if the
  header declares them without inline bodies. That is the "an object nobody
  references is still required" trap in its function form.
- **Vtable emission order in `.data`** — `dNandThread_c` before `mMutex` before
  `EGG::Mutex` — inverting if `mMutex` gets an out-of-line destructor.
- **Function-scope statics** (`a_banner`, `c_icon_res`) that must be declared
  *inside* `writeBanner()`, because hoisting them to file scope changes both
  their section and their pool IDs.

The class reconstruction is also complete rather than approximate: `sizeof 0x80`,
`mMutex` embedded at `0x50` with `OSMutex` at `+0x04` and `OSCond` at `+0x1C`,
all with compiled `STATIC_ASSERT`s. And you correctly checked whether the vtable
pointer sits at object offset `0x60` — it does not, because this derives from
`EGG::Thread` rather than `fBase_c`.

`fn_800CF170` is the one I would have missed: an unnamed map entry that becomes
`cmdSave__13dNandThread_cFPCv` once authored, so the pin has to be reconciled.
That is exactly the class of thing that turns into a link failure.

---

## Task A (primary): prove the five hazards, do not leave them predicted

A predicted hazard and a confirmed one are different things, and the difference
is usually one compile. **Nobody is authoring `d_nand_thread.cpp` yet, so you can
test the structure without touching anyone's work.**

Build a **scaffold** `d_nand_thread.cpp` in `scratch/` — your proposed header,
the anonymous-namespace data objects, the function-scope statics, and **empty or
near-empty bodies** for the 24 functions. You are not decompiling it; you are
testing whether the *structure* produces the right shape. Then compile it and
read the object.

Confirm or refute, each with the compiled evidence:

1. **Do the three `EGG::Thread` weak virtuals get emitted**, and at the end of
   `.text`? Test it both ways — with inline bodies in `eggThread.h` and without —
   and show the difference. If they only appear with inline bodies, that is a
   shared-header change I need to make, and I want it proven before I make it.
2. **Do the three vtables come out in the order `dNandThread_c`, `mMutex`,
   `EGG::Mutex`?** Deliberately provoke the inversion you predicted (give
   `mMutex` an out-of-line destructor) and confirm the order actually flips.
   A hazard you can trigger on demand is one you understand.
3. **Do the anonymous-namespace objects land in `.rodata` and `.bss`** as
   predicted, and do the function-scope statics land in `.bss`/`.sdata` rather
   than at file scope?
4. **Does `sizeof(dNandThread_c)` come out `0x80`** with the real `EGG::Thread`
   base rather than your test scaffold's? This is the one that matters most —
   `mMutex` is embedded by value, so if the base's size is wrong every member
   after `0x50` shifts and nothing in a per-function diff will show it.
5. **Section sizes.** Compare each section of your scaffold object against the
   claimed range. They will not match on `.text` (empty bodies), but `.rodata`,
   `.sdata` and `.bss` should be close to exact, because they are structural.

**A refuted hazard is as valuable as a confirmed one.** If the weak virtuals come
out fine without inline bodies, say so — that saves me a shared-header change
that would touch every TU including `eggThread.h`.

## Task B (secondary): pre-flight `m_pad.cpp`

Only after Task A. Same treatment as round 3, so the queue stays full behind
`d_nand_thread.cpp`: class from vtable, full function table, section bounds with
`dtk_splits_wiimj2d.txt` brackets, complete data inventory with an explicit note
on whether each object is referenced, external-call classification, and hazards.

You ranked it second at `0x8016F330`–`0x80170AC0`, 6,032 B, with a `.ctors` slot
and a `0x140` `.bss` claim. **The `.ctors` slot is worth attention** — it means a
static constructor runs, which means a `__sinit`, which means construction order
matters. `d_nand_thread.cpp` has none; this one does.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
  That includes `eggThread.h`: **prove the change in `scratch/`, do not apply it.**
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is on `EGG::Effect`'s vtable — note that
  `EGG::Thread` and `EGG::Effect` are different classes, so you are not in its
  way, but do not touch `eggEffect.hpp`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
