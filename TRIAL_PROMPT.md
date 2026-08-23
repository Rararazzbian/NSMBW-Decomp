# Trial — decompile `dPosShake_c` to byte-exactness

You are being evaluated on a real task from an ongoing Wii game decompilation
project. The goal of this project is to write C++ source that, compiled with the
original 2009 Metrowerks CodeWarrior compiler, produces **byte-identical machine
code** to the retail game. Not equivalent. Identical.

Success is objective and cheap to check, which is why this is a good test: a
function either assembles to the same bytes as the original or it does not.

**Read `AGENT_CONTEXT.md` in the repository root first.** It is the accumulated
knowledge base for this project and it will save you a great deal of time.

Write your report to **`TRIAL_RESPONSE.md`** in the repository root.

---

## Your unit

`dPosShake_c` — a screen-shake helper. Three functions, 304 bytes,
`0x800D81A0-0x800D82D0`. It is a complete, contiguous translation unit.

     28 B  init__11dPosShake_cFffffff      0x800D81A0
    244 B  move__11dPosShake_cFv           0x800D81C0
     16 B  startShake__11dPosShake_cFf     0x800D82C0

Everything you need is set up in **`scratch/trial/`**:

- `target.txt` — the retail disassembly. **It contains neighbouring classes
  too; ignore them.** Find your three functions with `grep -n '^\.fn '` and read
  each block with `sed -n`.
- `build.py` — compiles a draft, disassembles it, and reports its shape:

      python -c "import sys; sys.path.insert(0,'scratch/trial'); import build; \
                 print(build.build('NAME.cpp','NAME','move__11dPosShake_cFv'))"

  It returns `(words, frame, GPR saves, FPR saves, obj, txt)` and raises on a
  compile error with the compiler's message.
- `shadow/game/bases/` — put your header here. Include it as
  `#include <game/bases/d_pos_shake.hpp>`, **never** as a relative include; the
  angle-bracket form is the only one that works both here and after landing.

Score with:

    python tools/auto_decomp/fndiff.py scratch/trial/target.txt scratch/trial/NAME.txt --all

`DIFFS 0` means byte-exact. It also reports word count, stack frame size, and
which callee-saved registers are used, for both sides.

---

## What the task actually requires

**There is no header for this class.** Writing it is part of the deliverable.
You must reconstruct the member layout from the load and store offsets in the
disassembly, decide each member's type from the instruction used to access it
(`lfs`/`stfs` means `f32`, `lha`/`sth` means `s16`, `lwz`/`stw` with `cmpwi`
means `int`, and so on), and then write bodies that compile to the same
instructions in the same order.

Three things about this compiler that are worth knowing going in:

- **Declaration order controls register allocation.** Two source orderings that
  are semantically identical can produce different register assignments.
- **Function definition order is part of the object.** Define them in address
  order, not grouped logically.
- **Expression form matters.** `a * b / c` and `a * (b / c)` are the same
  arithmetic and different instructions. So is where you place a cast.

## Rules

- **Compile or it did not happen.** Every function you report on must have a
  real object file and a real `fndiff.py` line. A layout table with no compiled
  object is analysis, not a result.
- **Report the true count.** Your headline is the number of functions at
  `DIFFS 0` out of 3. If it is 0, say 0.
- Work **only** inside `scratch/trial/`. Do not modify `source/`, `include/`,
  `slices/`, `syms.txt`, `tools/`, or any other `scratch/` subdirectory.
- **Do not run `ninja`, `configure.py`, `progress.py`, or `land.py`.** Those
  rebuild or modify the whole project. `build.py` and `fndiff.py` are all you
  need and they only touch your own directory.

---

## What to put in `TRIAL_RESPONSE.md`

1. **Headline: the count at `DIFFS 0` out of 3.**

2. **The class layout you derived**, as a table: offset, type, your name for it,
   and — this matters — **the exact instruction line that proves each entry.**
   State the size of the class and how you know.

3. **The argument mapping for `init`.** Which incoming float register is stored
   to which offset. Do not assume the obvious answer; check it.

4. **Per function:** the `fndiff.py` line, and for anything not at `DIFFS 0`,
   the word count / frame / GPR / FPR for both sides and one sentence on what
   you think is wrong.

5. **Your reasoning on the hardest part.** `move` is 244 bytes of floating-point
   comparisons and branches. Explain how you worked out its structure — what
   you inferred, what you tested by compiling, and what you got wrong on the way.
   I am as interested in the method as the score.

6. **Anything you could not determine.** If you are uncertain about a member's
   type or a branch's meaning, say so explicitly rather than presenting a guess
   as a finding. Confident wrong answers are worse than flagged uncertainty.

---

## Stretch task, only if you finish the above

`dRotShake_c` at `0x800DF950-0x800DFA78` is a sibling class: 2 functions,
`init__11dRotShake_cFssssssss` (40 B) and `move__11dRotShake_cFv` (252 B). It is
also a complete contiguous TU, and its members are 16-bit rather than floats.
Prepare it yourself with:

    python tools/auto_decomp/prepare.py --unit scratch_trial/d_rot_shake.cpp \
        --range 0x800DF950-0x800DFA78

and report it the same way, as a separate section.

---

## A note on how this will be judged

I already know the correct answers for this unit — it was analysed
independently before you were given it. Your output will be checked against
that analysis and against the compiler, so there is no benefit to overstating a
result. A report of "1 of 3, and here is precisely why the other two miss" is
more useful, and scores better, than "3 of 3" that does not reproduce.
