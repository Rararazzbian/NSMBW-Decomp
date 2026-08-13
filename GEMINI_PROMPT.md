# Work order for Gemini — round 5

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 5.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 4 verdict: all five hazards proven, and the section sizes are the proof that counts

You did what I asked and then some. Every hazard now has compiled evidence
instead of a prediction:

- **The weak virtuals are real.** With inline bodies, `onExit__Q23EGG6ThreadFv`
  and `onEnter__Q23EGG6ThreadFv` are emitted at the tail of `.text`, marked
  `weak`; without them, nothing is emitted and `.text` is `0x10` short. That is a
  shared-header change to `eggThread.h` I now have evidence for rather than a
  guess, which is the difference between applying it and gambling with it.
- **You explained the vtable-order rule rather than just confirming it**:
  non-weak vtables emit unconditionally and first, weak base/embedded ones follow
  in derived-then-base order — and if `mMutex`'s destructor is out-of-line in
  another TU its vtable is omitted entirely. That is a reusable rule, not a
  fact about one unit.
- **The section sizes are the headline.** `.rodata 0x28`, `.bss 0x17040`,
  `.sbss 0x08`, `.sdata 0x0C`, `.data 0xA0`, all exact against the claims, from a
  scaffold with empty function bodies. Data placement is where the last two units
  actually failed — every blocker was in a section, none in a function — so a
  scaffold that reproduces all five sections exactly is worth more than any
  number of matched functions would be at this stage.

`m_pad.cpp` is pre-flighted and queued behind it.

---

## Round 5: make `d_nand_thread.cpp` landable, not just understood

The analysis is done. What is missing is the set of artifacts I need in hand to
start authoring the moment `d_a_player_manager.cpp` lands. Produce them.

### 1. The `eggThread.h` change, as a final diff

You proved the three virtuals need inline bodies. Give me the exact patch, and
say explicitly what else in the tree could be affected — `eggThread.h` is a
library header and other TUs may include it. If any already-matching TU derives
from `EGG::Thread`, adding inline bodies could flush weak copies into *it*, which
is the trap that has bitten this project repeatedly. **Check that and say so
either way**; if no other TU derives from it, that is exactly the sentence I need.

### 2. `d_nand_thread.hpp`, final

Ready to drop into `include/game/bases/`. `sizeof 0x80`, `mMutex` embedded at
`0x50`, `STATIC_ASSERT`s included, `@unofficial` on everything not from an
official source, honest `u8 pad[N]` wherever the evidence runs out. Include
`mMutex` and `EGG::Mutex` — say whether they belong in this header or their own,
and why.

### 3. The link-blocker list

Every external function the TU's `.text` calls, classified. This is the check
that matters and it is easy to get backwards, so use this filter:

```python
import json
d = json.load(open('slices/wiimj2d.json'))
BASE = 0x80006780
banked = []
for s in d['slices']:
    if s.get('nonMatching'):     # NOT linked -- a symbol it would define is still missing
        continue
    t = s['memoryRanges'].get('.text')
    if t:
        lo, hi = [int(x, 16) for x in t.split('-')]
        banked.append((BASE + lo, BASE + hi, s['source']))
# a candidate address inside one of these ranges must NOT be pinned -- it would
# be a duplicate definition and fail the link
```

Codex ran this task for the current unit and proposed 25 pins; **15 of them would
have failed the link**, because it asked "is this symbol called?" rather than
"who defines it?". The question is always **who defines it**. Note the
`nonMatching` subtlety cuts the other way: those slices are not linked at all, so
a symbol they would define still needs a pin.

Give me lines to **add**, and separately any existing pin that
`d_nand_thread.cpp` will itself define and which must therefore be **removed**
when it lands.

### 4. The NAND SDK dependency verdict

You flagged this as the unit's main risk in round 3 and it is still open. For
every `NAND*` call: does a declaration exist in `include/lib/revolution/`, does
it match the mangled symbol, and is the SDK function itself decompiled or does it
need a pin? A missing SDK prototype is a compile error and cheap; a wrong one is
a link error and is not.

### 5. The slice entry

The exact `slices/wiimj2d.json` block for this unit — every section with its
range — in the same shape as the existing entries. **Do not edit the file**;
give me the block.

---

## What I do not need

More analysis of whether this unit is a good choice. That is settled. If
something in rounds 3–4 turns out to be wrong while you assemble these, say so
loudly — a contradiction found now is worth far more than one found at the link.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is on two stray `EGG::Vector2f`/`Vector3f`
  destructors in the current unit — different header, but do not enter
  `eggVector.hpp` this round.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
