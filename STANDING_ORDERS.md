# Standing orders — autonomous decompilation

You are working continuously on this project. **This file replaces a per-round
brief.** Read it once, then work the queue without waiting to be told what to do
next. When you finish a unit, claim the next one yourself.

**Read `AGENT_CONTEXT.md` before your first unit.** It is the accumulated
knowledge base — every codegen lever, every trap, every rule that has cost this
project a round. It will save you hours. Re-read the sections relevant to a new
unit as you go.

---

## The one rule that governs everything

**A unit counts only at 100%.** The landing gate rebuilds the whole game and
requires all five binaries byte-identical to retail. A unit at 39/40 contributes
**exactly as much as a unit at 0/40: nothing.**

Two units are parked right now at 248/251 and 33/39. Between them they
represent many rounds of work and they have moved the project number by zero.

So: **finish units. Do not accumulate near-misses.** If you have a choice
between polishing a unit at 90% and closing a smaller one completely, close the
smaller one.

---

## The loop

    1. Claim a unit from WORK_QUEUE.md  (edit Status: UNCLAIMED -> IN PROGRESS)
    2. prepare.py the target listing
    3. Check the boundary by name
    4. Write the header, then bodies smallest-first
    5. fndiff --all until every function is DIFFS 0
    6. Byte-verify (see below) -- fndiff alone is NOT sufficient
    7. Produce a landing manifest
    8. Append the unit to READY_TO_LAND.md, set Status: DONE
    9. Claim the next unit. Do not stop to ask.

### 1. Claiming

`WORK_QUEUE.md` is the shared work list. **Edit the `Status:` line to
`IN PROGRESS` before you start**, and to `DONE` when the unit is finished and
appended to `READY_TO_LAND.md`. Other agents read this file; an unclaimed unit
may be taken by someone else.

Take units **in queue order** (easiest first) unless one is already claimed.
Easy units close fast, and closed units are the only ones that count.

### 2. Preparing

    python tools/auto_decomp/prepare.py --unit dol/bases/<name>.cpp \
        --range 0xSTART-0xEND

Then set up a scratch directory of your own — **`scratch/auto_<unitname>/`** —
with `target.txt` copied in, a `build.py` (copy `scratch/trial/build.py` and
change the directory name), and a `shadow/game/bases/` for your header.

### 3. The boundary check — do not skip this

`prepare.py` ranges are a **hypothesis**. After preparing, run
`grep -n '^\.fn ' target.txt` and confirm by name that the **last function in
your range belongs to your class and the next one does not.**

A translation unit does **not** end at its `__sinit`. If there is a
`__sinit_\<yourfile>_cpp`, the static initializer AND any `__arraydtor$NNNNN`
that follows it belong to you. This exact mistake cost a full round yesterday:
a unit was reported as three functions and was actually five.

Getting the end wrong is the single most common error this project has made.

### 4. Writing

- **Include headers as `#include <game/bases/x.hpp>`**, never `"x.hpp"`. The
  relative form works in scratch and fails at landing, and the compiler reports
  it as `undefined identifier 's16'`, two steps from the cause.
- **Function definition order is part of the object.** Define in address order,
  not grouped logically.
- **Compile or it did not happen.** Every function you report on needs a real
  object and a real `fndiff.py` line.
- If a header for your class already exists in `include/`, **copy it into your
  shadow directory and edit there.** Never edit `include/` or `source/`.

### 5. Scoring

    python tools/auto_decomp/fndiff.py scratch/auto_<unit>/target.txt \
           scratch/auto_<unit>/NAME.txt --all

### 6. Byte-verify before declaring a unit done

**`fndiff` both over- and under-reports. Do not trust it as your final check.**

- It **under-reports**: a function it does not mention is *unchecked*, not
  passing. Compiler-generated helpers (`__arraydtor$NNNNN`) get a different
  serial number on each side, so it silently skips them.
- It **over-reports**: it flags branch-label name differences that emit no
  bytes. Yesterday a function showed 3 diffs that all looked cosmetic; the real
  situation was **one** genuine difference hidden among them, and it changed the
  logic.

So for every unit, compare the raw bytes. Extract the 4-byte hex column from
both listings and diff them position by position. If word counts match and byte
diffs are zero, the function is done. Write your own script for this; it is
about fifteen lines.

### 7. The landing manifest — four parts, not two

A manifest that is missing any part will be rejected at link or verification.

  **(a) New header** — path in your shadow directory.

  **(b) Shared header changes** — any existing header in `include/` you had to
  modify, classified as **additive** (only added declarations) or **modifying**
  (changed something that already existed). Modifying changes are risky and must
  be called out explicitly.

  **(c) External FUNCTION addresses** — every function your unit calls that is
  not yet decompiled. A declaration satisfies the compiler; the linker needs an
  address. Look them up with `mcp__nsmbw-decomp__search_symbols` if you have it,
  otherwise grep `bin/dtk/wiimj2d_symbols.txt`.

  **(d) External DATA addresses** — same, for globals. **`search_symbols` lists
  functions only** and will report "no matches" for a data symbol that plainly
  exists. `bin/dtk/wiimj2d_symbols.txt` has everything, with section and size.

Also give the **slice ranges**: the byte range your unit occupies in each
section. Convert a virtual address to a slice offset by subtracting the section
base:

    .text   0x80006780      .data   0x802FE6A0      .bss    0x80351980
    .ctors  0x802EDCE0      .sdata  0x80427980      .sbss   0x80429EA0
    .sdata2 0x8042B360      .rodata 0x802EDFE0

**Slices must tile.** Your range ends where the next unit's begins, which means
it includes trailing alignment padding — not just your last byte of data. Check
what the next symbol's address is and end there.

Sections are easy to miss. Look at what your compiled object actually emits
(`grep '^# \.' yourfile.txt`) and make sure every section it produces has a
range in your manifest. A `.sdata` contribution omitted from a manifest cost
four rejected attempts yesterday.

If your unit has a `__sinit`, it also has a 4-byte `.ctors` entry. Find it by
parsing the DOL header in `bin/wiimj2d.dol` and scanning `.ctors` for a pointer
to your `__sinit`'s address.

### 8. Handing off

Append a section to **`READY_TO_LAND.md`** (create it if absent):

```
## <unit path, e.g. dol/bases/d_foo.cpp>
- Source: scratch/auto_foo/d_foo.cpp
- Header: scratch/auto_foo/shadow/game/bases/d_foo.hpp
- Functions: N of N at DIFFS 0, all byte-verified (word counts and hex compare)
- Slice: {".text":"0x...-0x...", ...}
- Shared header changes: <none | file + additive/modifying>
- External symbols needed:
      name=0xADDR
- Notes: <anything the lander should know>
```

**I run the landing.** You do not. See the prohibitions below.

---

## When you get stuck

Register-allocation mismatches are the hard part of this project, and they have
a specific character: **the word count, stack frame size and callee-saved
register set all match, and only operands or register numbers differ.** When you
see that, the control flow and allocation shape are already right.

**Do not reason about the compiler's allocator. Test.** The levers that work,
in rough order of yield:

  1. **Declaration order of locals** — this controls both which register a
     value gets and the order temporaries are assigned stack slots.
  2. **Binding a subexpression to a named local** vs inlining it. Yesterday a
     function went from 18 diffs to 0 purely by moving one cast into a local,
     because it changed which conversion got the lower stack slot.
  3. **Expression shape** — `a * b / c` vs `a * (b / c)`; where a cast sits.
  4. **Addressing form** — a cursor pointer vs independent offset reads.
  5. **Guard form** — `if (!x) { body }` with one shared return, vs an early
     `return` inside the guard. These produce different tail structures.

Write a sweep script that compiles a dozen variants and scores them. That is
faster and more reliable than argument, and it is how both remaining functions
got closed yesterday.

### The stall rule

If a single function has made **no progress across three separate attempts**,
stop working it. Do one of:

  - **Park the unit.** Write the full state into `READY_TO_LAND.md` under a
    `## PARKED:` heading — the score, the exact remaining diffs, what you tried,
    and what you would try next. Then set the queue Status to `PARKED` and
    **claim the next unit.**
  - Never write "closed" or "done" for a parked unit. Record the state.

A parked unit with good notes is a real contribution. A unit ground on for six
rounds is not.

---

## Reporting

Keep a running log in **`AUTO_LOG.md`** — append, never overwrite. One short
entry per unit as you finish or park it:

    ## <unit> — <N>/<N> — LANDED-READY | PARKED   <what happened in 2-3 lines>

Lead every status statement with **the true count**. If a unit is at 2 of 5, say
2 of 5. An inflated headline is the one thing that makes your work unusable,
because I have to re-derive the truth before I can trust anything else in the
report. A precise "1 of 3, and here is exactly why the other two miss" is worth
more than an unreproducible "3 of 3", and it is scored higher.

If you discover something general — a codegen lever, a trap, a rule — append it
to `AGENT_CONTEXT.md` under a new `##` heading. That file is how this project
compounds.

---

## Hard prohibitions

- **Never run `ninja`, `configure.py`, `progress.py`, or `land.py`.** These
  rebuild or mutate the whole project. Landing is mine; it is the verification
  gate and it must stay with one owner.
- **Never modify** `source/**`, `include/**`, `slices/`, `syms.txt`,
  `tools/**`, `bin/**`, or `HANDOFF.md`.
- **Never touch another agent's scratch directory.** Yours are
  `scratch/auto_*` and `scratch/trial/`. `scratch/qwen_*`, `scratch/gemini_*`
  and `scratch/round33/` belong to other agents — you may **read** them, never
  write.
- `prepare.py` writes to `tools/auto_decomp/work/`, which is its own output
  directory and is fine.
- Do not `git commit`, `git push`, or change branches.

## Do not stop

When a unit is done or parked, **claim the next one and keep going.** You do not
need permission to continue, and you should not wait for a reply between units.
The only reasons to stop are: the queue is empty, or every remaining unit is
claimed by someone else.

If the queue empties, say so in `AUTO_LOG.md` and then extend it yourself using
the vetting method described at the top of `WORK_QUEUE.md` — find candidates
with `mcp__nsmbw-decomp__find_targets`, verify each is a whole contiguous TU
with `search_symbols`, and append the survivors.
