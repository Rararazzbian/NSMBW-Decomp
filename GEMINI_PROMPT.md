# Work order for Gemini — round 6

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 6.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 5 verdict: `eggThread.h` is landed, and the impact audit is why

The patch is **in the tree, all five binaries byte-identical.** The three
virtuals now carry their inline bodies, the vtable pointer costs 4 bytes, the pad
drops `0x4c` to `0x48`, and `sizeof(EGG::Thread)` stays `0x4C`.

**What made it applicable was the audit, not the patch.** You searched all 145
slices and every header for other classes deriving from `EGG::Thread` and found
exactly one — `mDvd::MyThread_c`, not banked. Then you compiled `d_system.cpp`,
the one banked TU that reaches the header, before and after, and confirmed the
allocation size stays `0x4C`, no weak virtuals or vtables appear in
`d_system.o`, and the canonicalised instructions are 100% identical.

That is the difference between a change I can apply and one I have to gamble on.
Three shared-header changes have failed verification on this project; this one
arrived with its blast radius already measured.

---

## Task A (primary): four bytes are unaccounted for, and they are load-bearing

**Your own two rounds disagree, and I want you to resolve it rather than pick
one.**

- Round 5 establishes `sizeof(EGG::Thread) == 0x4C`, proven by `d_system.cpp`
  allocating exactly `0x4c` — and that is now landed and verified.
- Rounds 3 and 4 place `mMutex` at offset **`0x50`** inside `dNandThread_c`, with
  `sizeof(dNandThread_c) == 0x80`, and round 4 stated `sizeof(EGG::Thread) ==
  0x50`.

If the base is `0x4C` and `mMutex` starts at `0x50`, **four bytes between `0x4C`
and `0x50` belong to something.** Three readings, and they are not equally
likely:

1. **`dNandThread_c` declares a member of its own at `0x4C`**, before `mMutex`.
   A 4-byte field there would be invisible to your scaffold if the scaffold
   padded to `0x50` instead of declaring it.
2. **It is alignment padding.** Note `AGENT_CONTEXT.md` §6: MWCC aligns a `.bss`
   object to 8 when its *size* is a multiple of 8 — but that is a placement rule
   for whole objects, **not** a rule about member offsets inside a class, so it
   does not explain this. If you think alignment explains it, say which
   alignment and what forces it.
3. **`sizeof(EGG::Thread)` is really `0x50`** and the `0x4C` allocation in
   `d_system.cpp` is a different constructor path or a different class.

**Why this matters more than it looks:** `mMutex` is embedded by value. If its
offset is wrong, every byte from `0x50` to `0x80` shifts, and **nothing in a
per-function diff will show it** — it surfaces only at the link, as the "wrong
small-data bound" signature. The current unit lost a whole round to exactly this
shape of error, in `.sbss`.

Settle it from the binary: the constructor at the TU's start writes the members
in order, and `OSInitMutex`/`OSInitCond` are called on the mutex, so the address
they receive gives you `mMutex`'s true offset directly. Then account for
everything below it.

**If it is a real member, name it only as well as the evidence supports.** If you
cannot tell what it is, `u8 pad4C[4]` labelled as unexplained is the right answer
— see `AGENT_CONTEXT.md` §4.

## Task B (secondary): audit your own 21-pin list with the filter

Your round 5 pin schedule proposes 21 additions. **Run the banked-slice filter
over it yourself before I do**, using the code from round 5's brief: for each
candidate address, is it inside the `.text` range of a slice that is *not*
marked `nonMatching`?

I am asking because Codex proposed 25 pins for the current unit and **15 of them
would have failed the link** — every one an address inside a banked slice, every
one a duplicate definition. Its list was built from "is this symbol called?"
rather than "who defines it?", and the filter catches that in seconds.

Your list is mostly `NAND*` and `OS*` SDK functions, which are likelier to be
genuinely unbanked — so I expect it to come back clean. **Report the result
either way**, including the count you checked, because a self-audit that finds
nothing is still evidence and takes a minute.

Same for your 4 removals: confirm each is a symbol `d_nand_thread.cpp` will
itself define.

---

## What I do not need this round

The NAND SDK header patches can wait — they are compile-time issues, which are
cheap and surface immediately. The offset question is a link-time issue, which is
neither.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is on two stray `EGG::Vector` destructors in the
  current unit; stay out of `eggVector.hpp`.
- Report contradictions rather than reconciling them — including, as here, a
  contradiction between two of your own rounds. Spotting that is not a
  criticism; both rounds were careful, and the discrepancy is exactly the sort
  that survives careful work.
- Plain ASCII or clean UTF-8, LF, no BOM.
