# Work order — GXStateSave_c (continuing)

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** in the repository root (overwrite it).

---

## Where you are

Your unit is **`GXStateSave_c`** — `0x80014330`, 624 bytes, 4 functions. It is a
complete contiguous class, and the unit immediately after it in memory is one I
landed today, so if you reach 4/4 it lands.

      12 B  __ct__13GXStateSave_cFv
      64 B  __dt__13GXStateSave_cFv
     248 B  save__13GXStateSave_cFUl
     300 B  restore__13GXStateSave_cFv

Your directory is **`scratch/qwen_gx/`** and it already contains `target.txt`
(from `prepare.py`), a `draft.cpp` you wrote, and a `build.py` with the
interface you know:

    python -c "import sys; sys.path.insert(0,'scratch/qwen_gx'); import build; print(build.build('NAME.cpp','NAME','save__13GXStateSave_cFUl'))"

Score with:

    python tools/auto_decomp/fndiff.py scratch/qwen_gx/target.txt scratch/qwen_gx/NAME.txt --all

## Last round: 0 of 4, and the blocker you reported is not real

You wrote a class reconstruction and then stopped, reporting that the build
could not run because *"the seeded header `game/bases/d_gx_state_save.hpp` and
its GX declarations are absent from this checkout."*

**The GX declarations are present.** They are under `include/lib/revolution/GX/`:

    include/lib/revolution/GX/GXAttr.h
    include/lib/revolution/GX/GXBump.h
    include/lib/revolution/GX/GXDisplayList.h
    ... and the rest of the GX headers

Find the ones declaring the functions your `save`/`restore` call —
`GXGetVtxAttrFmtv`, `GXSetVtxAttrFmtv`, `GXGetVtxDescv`, `GXSetVtxDescv`, the
projection, viewport, scissor and cull get/set pairs — and include them.

`game/bases/d_gx_state_save.hpp` genuinely does not exist yet, because **you are
the one writing it.** A new unit's header is part of the deliverable, not a
prerequisite. Create `scratch/qwen_gx/d_gx_state_save.hpp`, put your class in
it, and include it from your `.cpp`.

This matters more than the round: `AGENT_CONTEXT.md` already carries the rule
*"before concluding the environment is broken, confirm which file the compiler
actually opened."* A missing include path and a missing file look identical
from the error message, and the expensive reading — "the checkout is
incomplete" — was wrong here. Check that a header really is absent before
reporting it as a blocker.

**Credit where it is due:** your derived layout table is good work and it is the
hard part of this unit. The mask at `+0x0`, the vertex-attribute and descriptor
regions, the projection/viewport/scissor/cull blocks and the three flag bytes
are all supported by the get/set pairs you cited. Start from it.

## Include style — a hard rule

    #include <game/bases/d_gx_state_save.hpp>      correct
    #include "d_gx_state_save.hpp"                  WRONG

The relative form compiles in a scratch directory and **rejects at landing**,
because the header ends up in `include/game/bases/` and the source in
`source/dol/bases/`. MWCC reports it as `undefined identifier 's16'` rather than
as a missing header, which is two steps from the cause. This cost a rejected
landing today. Use angle brackets from the first draft.

For your scratch build, put the header at
`scratch/qwen_gx/shadow/game/bases/d_gx_state_save.hpp` — `build.py` passes
`scratch/qwen_gx/shadow/` on the include path automatically if that directory
exists, so the angle-bracket form resolves in scratch *and* after landing.

---

## Order of work

1. **Write the header** from your layout table.
2. **`__ct`** — 12 bytes, so it does almost nothing.
3. **`__dt`** — 64 bytes; the size tells you whether it is virtual.
4. **`save(u32)`** — saves the subset selected by the mask bits.
5. **`restore()`** — the mirror of `save`. Each one checks your reading of the
   other; if a member offset works in one and not the other, the layout is wrong.

Two standing rules:

- **Function definition order is part of the object.** Define them in address
  order, not grouped logically.
- **Compile or it did not happen.** Every function you report gets a real object
  and a real `fndiff.py` line. A layout table with no object is analysis, not a
  result.

## Acceptance

- The header written, and the build actually running.
- All four functions attempted, each with an `fndiff.py` line.
- **The count at `DIFFS 0` out of 4 as your headline.**
- For any that miss: words / frame / GPR / FPR both sides, plus one sentence on
  what you think is wrong.
- A landing readiness statement: 4/4 or not, and what is left.

If you run out of budget, say what you did not reach. Do not run `ninja`,
`configure.py`, `progress.py` or `land.py`. Work only in `scratch/qwen_gx/`.
