# Work order — round 36

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## `dPSwManager_c` LANDED

Twelve of twelve, first round on a new unit. I verified it three ways before
trusting it — your `fndiff` run, my own recompile from your source with my own
harness invocation, and then the real gate:

    python tools/auto_decomp/land.py --unit dol/bases/d_p_sw_manager.cpp ...

    ACCEPTED -- all five binaries are byte-identical to the original.
    Total: Decompiled 738648/6500368 code bytes (11.363%)

`source/dol/bases/d_p_sw_manager.cpp` and
`include/game/bases/d_p_sw_manager.hpp` are in the tree, the slice is in
`slices/wiimj2d.json`, and your `d_bg_parameter.hpp` change went in as
described — additive at the end of the class, every existing offset untouched.
The build proved it harmless.

Your landing manifest was accurate enough to act on without a single
correction: the slice ranges, the header change, and the promotion list were all
right. That is what made this a ten-minute landing instead of an afternoon.

**11.353% → 11.363%.** Small in absolute terms, and the first thing to actually
land this session.

## One process note for next time

You hand-rolled `extract_target.py` to pull the retail listing out of a
disassembly file. It worked, but the project already has the tool:

    python tools/auto_decomp/prepare.py --unit dol/bases/d_foo.cpp \
        --range 0x800B8130-0x800B8388

It collects every dtk object whose start address falls in the range,
disassembles them, concatenates in address order, and writes
`tools/auto_decomp/work/<unit>/target.txt` plus a `draft.cpp` stub. Read its
docstring — it carries a warning that cost this project real time: **the range
is a hypothesis until proven**, and a TU does not end at its `__sinit`; the
`sFStateID_c<YourClass>` instantiations after it belong to you too. After
preparing, check that the last function in `target.txt` belongs to your class
and the next one does not.

---

## Round 36 — `dPanelObjList_c`

    0x800145B0    624 B    17 functions

Seventeen functions in 624 bytes, so most are tiny — a constructor, a
destructor, and a run of accessors:

      60 B  __ct__15dPanelObjList_cFv
      64 B  __dt__15dPanelObjList_cFv
       8 B  getValue__15dPanelObjList_cCFv
      20 B  isChange__15dPanelObjList_cCFv
       8 B  setChange__15dPanelObjList_cFb
       8 B  getPosX__15dPanelObjList_cCFv
      ... 11 more

Work in **`scratch/gemini_panelobj/`**. Use `prepare.py` to get the target, then
your own harness for the compile/disassemble loop, and `fndiff.py --all` as the
scoreboard.

### 1. Prepare and verify the range

**Acceptance:** `target.txt` produced by `prepare.py`, the boundary sanity-check
done and stated — last function in range belongs to `dPanelObjList_c`, next one
does not — and a table of all seventeen with target words / frame / GPR / FPR.

### 2. Decompile, smallest first

Eight-byte accessors first. They are one or two instructions each and they bank
quickly. Constructor and destructor next. Anything substantial last.

- **Function definition order is part of the object.** Address order, not
  logical grouping.
- **Compile or it did not happen.** Every reported function gets an object and
  an `fndiff.py` line.
- **Grep `source/` before inventing an idiom.** 170 landed files now, all
  byte-exact, and accessors are exactly the kind of thing where the house style
  is consistent and easy to copy.

**Acceptance:** all seventeen attempted, with the count at `DIFFS 0` as your
headline.

### 3. The landing manifest

Same as last round, and last round's was good: the slice `memoryRanges`, any
new header, any shared-header change classified as additive or modifying, and
anything you need me to declare. If the unit reaches 17/17 I will land it
immediately.

**Acceptance:** a landing readiness statement — landable yes or no, and the
exact remaining list.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Constraints

Work only in `scratch/gemini_panelobj/`. `prepare.py` writes to
`tools/auto_decomp/work/` — that is its own output directory and it is fine for
it to do so, but do not edit anything else under `tools/**`. Do not touch
`scratch/gemini_round24/`, `scratch/gemini_pswmgr/`, `scratch/round33/`,
`scratch/qwen_ice/`, `scratch/claude_kokoopa/`, `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`** — I run the landing.

---

## Reporting

- **Lead with the count at `DIFFS 0` out of 17.**
- The seventeen-function target table.
- Per-function results with `fndiff.py` lines.
- **GAINED and LOST by name.**
- `poolcheck.py` output.
- The landing manifest and readiness statement.
- `NOT REACHED`, if anything.
