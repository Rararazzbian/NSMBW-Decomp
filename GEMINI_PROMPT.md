# Work order — round 37

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## `dPanelObjList_c` LANDED — 17/17, second in a row

    ACCEPTED -- all five binaries are byte-identical to the original.
    Total: Decompiled 739272/6500368 code bytes (11.373%)

Two units, two rounds, both 100% first attempt. 11.353% → 11.363% → 11.373%.

I verified it the same three ways — your score, my own recompile from your
source against a target I prepared myself for the exact range, and then
`land.py`. Your slice ranges and the `.sdata2` adjacency check were right, and
your overlap check saved me from doing it.

### One correction, because it cost a rejected landing

Your `.cpp` opened with:

    #include "d_panel_obj_list.hpp"

A relative include works in `scratch/` where both files sit together, and fails
the moment the header is in `include/game/bases/` and the source in
`source/dol/bases/`. The build rejected it — correctly, and it rolled everything
back cleanly. What made it slow to read is that MWCC does not say "header not
found"; it says:

    Error: undefined identifier 's16'
    Error: undefined identifier 'u8'

two steps downstream of the real cause. **Always use
`#include <game/bases/x.hpp>`.** That is the house style in all 170 landed
files, and it is the only form that works both in scratch and after landing.

---

## Round 37 — `dIceEfMaker_c`

    0x800B8130    600 B    6 functions

      44 B  init__13dIceEfMaker_cFiP13dIceEfScale_c
     156 B  execute__13dIceEfMaker_cFv
       4 B  fin__13dIceEfMaker_cFv
     132 B  setEfScale__13dIceEfMaker_cFRC13dIceEfScale_c
     124 B  createEffect__13dIceEfMaker_cFQ213dIceEfMaker_c8EfKind_e
     104 B  hahenEffect__13dIceEfMaker_cFv

Those six are **every** `dIceEfMaker_c` symbol in the binary and they run
contiguously to 0x800B8388, so this is a complete TU. The file name is confirmed
by a symbol in it: `l_mdl_scale_tbl__32@unnamed@d_ice_effect_maker_cpp@`, so the
unit is `dol/bases/d_ice_effect_maker.cpp` and that table is a file-scope static.

**I gave this to the other agent first and it reached 1/6** — only the 4-byte
`fin`. It got stuck on exactly the part you are good at: the class layout has to
be reconstructed from the binary before any body can be written, and there is a
second type, `dIceEfScale_c`, in the signatures. Its partial work is at
`scratch/qwen_ice/` — **you may read it, do not write there.** Read its notes
before starting; it did real analysis even though it did not land the code.

Work in **`scratch/gemini_icefx/`**.

### What I already know from the binary, so you do not re-derive it

- **`init(int, dIceEfScale_c *)`** stores `0` to `+0x0` and the `int` to `+0x4`,
  then: if the pointer argument is null it substitutes
  `l_mdl_scale_tbl + (arg << 5)` — so the table has **32-byte entries** indexed
  by that int — and tail-calls into the `setEfScale` body.
- **`execute()`** frame `0x30`, uses `_savegpr_27` / `_restgpr_27`. It reads
  `+0x848` for an actor, calls `getCenterPos__12dBaseActor_cCFv` into a stack
  temp, then loops `i = 0..7` testing bit `i` of the word at `+0x0`; for each
  set bit it calls a virtual through **slot 0xC** on the pointer at
  `+0x828 + 4*i`, and clears the bit if the call returns 0.
- So the layout is at least: a **u32 bitmask at +0x0**, an **int at +0x4**, an
  **array of 8 pointers at +0x828**, and an **actor pointer at +0x848**.
  `0x828 + 8*4 = 0x848` confirms the array length.
- `execute` also loads a float from `.sdata2` symbol `@61403` and stores it into
  the stack temp at `+0x10` before the loop.

### Order of work

1. **Reconstruct the class and `dIceEfScale_c` from the listing**, and state the
   evidence for each member — the instruction and offset that proves it. This is
   the step that blocked the other agent and it is the whole unit.
2. **Then bodies, smallest first**: `fin` (4 bytes, empty), `init`, `hahenEffect`,
   `createEffect`, `setEfScale`, `execute`.
3. **Fold, score with `fndiff.py --all`, run `poolcheck.py`.**

Use `prepare.py` for the target; the range is `0x800B8130-0x800B8388`. Check the
boundary by name afterwards.

**Acceptance:** the layout table with per-member evidence; all six attempted with
an `fndiff.py` line each; the count at `DIFFS 0` out of 6 as your headline; and a
landing manifest with the slice `memoryRanges`, any new header, and any shared
header change classified as additive or modifying.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Constraints

Work only in `scratch/gemini_icefx/`. You may **read** `scratch/qwen_ice/`.
`prepare.py` writes to `tools/auto_decomp/work/`, which is its own output
directory and fine; do not edit anything else under `tools/**`. Do not touch any
other `scratch/` directory, `wip/**`, `source/**`, `include/**`, `slices/`,
`syms.txt`, `configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`.
**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — I land.

---

## Reporting

- **Lead with the count at `DIFFS 0` out of 6.**
- The class layout table with evidence per member.
- Per-function results with `fndiff.py` lines.
- **GAINED and LOST by name.**
- `poolcheck.py` output.
- The landing manifest and readiness statement.
- `NOT REACHED`, if anything.
