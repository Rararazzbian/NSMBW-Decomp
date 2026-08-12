# Brief: decompiling an actor TU (NSMBW byte-exact decomp)

Repo: `C:\Users\Razz\Documents\Projects\NSMBW-Decomp`, branch
`claude/game-decompilation-setup-bw30s7` (already checked out — **do not** switch
branches, commit, stash, or checkout).

Your working directory is in your task prompt. Work only there. Do **not** edit
anything under `source/`, `include/`, `slices/`, or `syms.txt` — copy any header
you need to patch into your own directory and inject it with a leading
`-i <yourdir>`. The lead integrates; you author.

## Goal

Reconstruct C++ that CodeWarrior compiles to **byte-identical** machine code —
same instructions, registers and order — for one whole translation unit.

## Read this first

`HANDOFF.md` in the repo root, sections **"The actor-TU playbook"** and
**"Techniques established"**. The playbook was written from a TU
(`d_a_en_super_bigpile.cpp`) that matched 46/46 functions, and it will give you
most of your file in the first ten minutes. `source/dol/bases/d_a_en_super_bigpile.cpp`
and its header are the reference implementation — read them.

## The compile-and-diff loop

```
compilers\Wii\1.1\mwcceppc.exe -c -proc gekko -fp hard -O4 -inline noauto
  -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS
  <yourdir>\draft.cpp -o <yourdir>\draft.o -DREVOLUTION -I-
  -i include -i include\lib -i include\lib\MSL -i include\lib\MSL\internal
  <plus the seven -i include\lib\revolution\BTE\... paths from build.ninja>
.\bin\dtk-windows-x86_64.exe elf disasm <yourdir>\draft.o <yourdir>\draft.txt
python <SP>\fndiff.py <target.txt> <yourdir>\draft.txt <MangledName>
```

Run from the repo root. Build native-exe argument lists as a PowerShell array and
splat (`& $exe @args`). Do **not** run `ninja`, `configure.py` or `progress.py`.

`<SP>\trackC\c1\diffall.py` diffs *every* function in one shot and normalises
compiler-pool symbol names — far faster than calling `fndiff.py` per function.
Copy and adapt it.

**`fndiff.py` caveat:** it used to print `IDENTICAL` when it could not find the
function. That is fixed — it now hard-exits — but verify with a negative control
(diff a function you know is absent) before trusting a clean run.

## Deliverables

1. **The full `.cpp` and `.hpp`**, complete and compilable.
2. **Per-function diff status.** MATCHING only if the diff tool printed nothing.
   Report exact remaining diffs otherwise — a well-characterised near-miss is a
   good result; a false MATCHING is the worst possible outcome and has cost this
   project a full day.
3. **The slice `memoryRanges` for every section your TU touches**, as offsets
   ready to paste into `slices/wiimj2d.json`. This is new, and it is the most
   valuable thing you can hand back — integration time goes here, not into code.
   See below.
4. Any **new declarations** needed in shared headers, and any **symbol addresses**
   for undecompiled callees (`syms.txt` entries, `name=0xADDRESS`).

## Section bounds — do this properly, it is where the time goes

Section bases: `.text` 0x80006780 · `.ctors` 0x802EDCE0 · `.rodata` 0x802EDFE0 ·
`.data` 0x802FE6A0 · `.bss` 0x80351980 · `.sdata` 0x80427980 · `.sbss` 0x80429EA0 ·
`.sdata2` 0x8042B360. Offsets in the slice are section-relative.

For each section your object emits, find the TU's true range by listing the
symbols on **either side** of your objects in `bin/dtk/wiimj2d_symbols.txt`. The
neighbours pin down both ends. A previous TU's slice ending exactly where yours
begins is strong corroboration — check `slices/wiimj2d.json` for the file before
yours.

Do not forget `.ctors` (4 bytes, the `__sinit` pointer), `.bss` and `.sbss` —
omitting a section entirely is the single most common integration failure here.
Pad the end to the alignment of the next TU's first object.

**The `.text` end is the part that keeps being wrong — twice in the last batch.**
Your TU does not end at `__sinit`. After it come the `sFStateID_c<YourClass>` /
`sFStateVirtualID_c<YourClass>` template instantiations, and they are yours. Walk
forward until you hit a function belonging to a *different* class, and make that
your end. dtk's `auto_*_text.o` split points are arbitrary and are **not** TU
boundaries — do not use them for either end. One agent lost 0x3A8 bytes this way
and another 0x3AC.

**Inter-TU `.data`/`.sdata2` alignment is 8.** A lone 4-byte gap between two
objects the compiler would otherwise pack tightly marks a TU boundary. This is
the sharpest boundary signal available — it proved a range was two TUs when
`.data` came out 0x2F8 merged but needed 0x300. dtk's `@NNNNN` pool numbers are
*not* reliable for this; one TU alone spans `@63751`–`@64847`.

**A range may contain a TU you were never told about.** A TU with no file-scope
static objects emits no `__sinit`, so our enumeration cannot see it and silently
folds it into a neighbour. Two were found this way in the last batch. If the
functions in your range split cleanly into two unrelated classes, that is why.

**Weak functions your object emits that nothing references get deadstripped.**
If your TU legitimately owns one, it must be named in `keepWeak` in
`slices/wiimj2d.json` — report it, do not edit the file. Symptom if missed:
`.text` comes out short by exactly that function's size and *every* binary fails
its hash, including ones your TU cannot touch.

`_SDA_BASE_` (r13) = 0x8042F980, `_SDA2_BASE_` (r2) = 0x80433360 — decode
r13/r2-relative offsets in the target with these to place small-data objects.

## Levers that work

1. **Declaration order controls register allocation.** GPRs: leading declarations
   take the top of r6–r9 ascending, ending at r9; later values fill downward.
   FPR direction is **not fixed** — sweep it, do not assume.
2. **Adding one local, changing no logic, can fix register colouring at zero
   instruction cost.** Proven on a pointer, a float constant and a raw byte
   offset. If you are instruction-perfect but register-wrong, try this first.
3. **Size-delta heuristic.** Shorter than target ⇒ you factored out something
   inlined. Longer ⇒ you left something out of line.
4. **Bisect before theorising.** The flags above are correct and verified; every
   "wrong compiler" theory this project has entertained has been refuted.
5. Every empty virtual must be **defined out of line** in the `.cpp`, never in the
   class body, or `-ipa file` inlines it away and the vtable slot breaks.
6. A function-scope `static const int` allocates a word in `.sdata2` — use
   `enum { NAME = value };` unless the binary shows an object.
7. `.sdata2` order within a TU: **creation order** — named file-scope objects and
   anonymous folded literals share one sequence, ordered by where each is first
   created (definition, or first use), interleaving freely. Function-local
   statics come **last**. (An older note claiming "all named first" is wrong.)
8. **`x % y`:** MWCC evaluates whichever operand needs more registers first and
   gives it the lower scratch register. Rewriting the expression will not flip
   it — only making the divisor a **named local that is already live** does.
   Verified on a 40-function standalone micro-benchmark, not just one case.
9. When a single-variable sweep plateaus at a hard floor, **stop sweeping**. The
   hardest functions solved so far each needed two or three coupled changes that
   only work together; none was reachable one variable at a time. Sweeps of 25,
   115 and ~120 builds have all failed where a coupled pair succeeded.
10. **Inlined parameter binding perturbs FPR colouring; field stores do not.**
    `v.set(a, b)` binds two parameters into an inlined call and moves the float
    registers; `v.x = a; v.y = b;` does not. Combine with reading inputs as
    individual accessors rather than one multi-out `getBounds(&a,&b,&c,&d)`.
11. **A dead local aggregate gets scalar-replaced into scattered stack slots**
    regardless of how you construct it. If the target keeps such an aggregate
    contiguous, you are missing a *use* of it — look for that, rather than for a
    cleverer construction. Do not reach for a cast hack; if you must use one to
    match, mark it and say so, because an implausible idiom that happens to
    match is a liability for the next reader.

## Environment

- dtk cannot resolve relative paths with forward slashes on Windows — use
  `.\bin\dtk-windows-x86_64.exe` or an absolute path.
- PowerShell 5.1 parses `0x800E5510` as a **negative Int32**. Do all address
  arithmetic in Python.
- **No background processes.** Everything foreground, confirm exits, check for
  strays before finishing.

## Scope

Finish the whole TU if you can. If you cannot, a solid class layout, verified
section bounds and a documented set of matching functions is still a good
result — say clearly what is done and what is not.
