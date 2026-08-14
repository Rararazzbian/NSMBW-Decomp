# AGENT_CONTEXT.md — shared briefing for every AI working this repo

**Read this once per session, before your round's prompt.** It is the material
that does not change between rounds. Your prompt file
(`CODEX_PROMPT.md` / `GEMINI_PROMPT.md`) contains only what is specific to your
round and assumes you have read this.

This file is maintained by Claude. **Do not edit it.** If something in it is
wrong, say so in your response — that is a valuable finding, and it has happened.

---

## 1. What this project is

A **matching decompilation** of *New Super Mario Bros. Wii* (2009, Wii).

We write C++ that, compiled with the **original CodeWarrior compiler**, produces
**byte-identical** machine code to the retail game. Not equivalent. Not
functionally the same. The same bytes.

The authority is one command, run only by Claude:

```
python progress.py --verify-bin
```

It MD5s five binaries — `wiimj2d.dol`, `d_profileNP.rel`, `d_basesNP.rel`,
`d_enemiesNP.rel`, `d_en_bossNP.rel`. All five green, or the change is not done.
There is no partial credit. A change that improves one function and shifts a
section by four bytes fails four binaries and is worse than no change at all.

Current position: roughly **11%** of the project, and `wiimj2d.dol` around
**22%**. Progress is measured in landed translation units.

## 2. The compiler, and why it is the whole game

`compilers/Wii/1.1/mwcceppc.exe`, invoked as:

```
-proc gekko -fp hard -O4 -inline noauto -Cpp_exceptions off -enum int
-RTTI off -ipa file -enc SJIS -DREVOLUTION -I-
```

plus **seven mandatory `include\lib\revolution\BTE\...` include paths**. Without
them anything that includes `d_audio.hpp` fails, which is most game code.

**Do not reproduce this command line by hand.**
`tools/auto_decomp/harness.py` has `compile_draft(src, obj)` which encodes all of
it, plus `disasm`, `extract`, `canonicalise` and `diff_fn`. Import and call them.
Several people have wasted a round on a hand-built command line missing a flag.

### The thing that catches everyone: CFront mangling omits return types

`daPyMng_c::addNum()` and a hypothetical `int daPyMng_c::addNum()` mangle
**identically**. So:

- **No symbol comparison can ever confirm a return type.** Not the map, not a
  `syms.txt` pin, not a mangled-name match.
- The only signal is **register allocation**. Declared `bool`, MWCC reserves `r3`
  for the eventual return and pushes a temp into `r4`; declared `void`, that temp
  lands in `r3`.
- **Six** return types were wrong in a single class before anyone noticed, and
  every one was found the same way: **compile it both ways and let the diff
  decide.** Do not argue a return type from the disassembly — test it.

Parameter types, by contrast, **are** in the mangled name. Changing one is never
cosmetic: check the new mangled name against the symbol map before proposing it.

## 3. Who owns what

| Worker | Owns | Never touches |
|---|---|---|
| **Claude** (lead + sub-agents) | The current translation unit. Runs the shared build. The **only** one who edits `slices/wiimj2d.json`, `syms.txt`, and any shared header. | `CODEX_HANDOFF.md` |
| **Codex** | Whatever `CODEX_PROMPT.md` assigns. `CODEX_HANDOFF.md` is its private notebook. | `wip/`, `HANDOFF.md`, `GEMINI_*.md`, `AGENT_CONTEXT.md` |
| **Gemini** | Whatever `GEMINI_PROMPT.md` assigns. | `wip/`, `HANDOFF.md`, `CODEX_*.md`, `AGENT_CONTEXT.md` |

`scratch/` is shared and disposable — use subfolders. `wip/` is Claude's agents'
authoring area and is **off limits** to peers.

## 4. The hard rules

Each of these exists because something broke.

1. **Never run `ninja`, `configure.py`, `progress.py`, or
   `tools/auto_decomp/land.py`.** Claude is the only integrator. Two builds in
   this checkout clobber each other's object files, and the second one to finish
   silently reports the first one's results.

2. **Never edit a shared header — propose it.** Copy it into `scratch/`, make the
   change there, prove it compiles, and put the diff in your response. Claude
   applies it, rebuilds, and verifies all five binaries **before** it lands with
   anything else, so a failure can never be ambiguous between two changes.

   Three shared-header changes have now failed verification on this project. One
   of them looked completely safe — adding a function overload that genuinely
   exists in the binary — and it broke the build because seven existing call
   sites became ambiguous. "It obviously cannot break anything" is not a reason
   to skip the test; it is the exact situation the test is for.

3. **Never edit `slices/wiimj2d.json` or `syms.txt`.** Propose instead. These are
   global lists and a wrong entry fails the link in a way that looks like a code
   bug.

4. **Report contradictions rather than reconciling them.** If the symbol map, the
   disassembly, and a header disagree, say all three and stop. This instruction
   has earned more than any other here. Examples that paid: a `sizeof`
   disagreement that would have left a base class 0x12C bytes too small, and an
   offset that turned out to already be a named, matched member.

5. **Report a negative result rather than manufacturing a positive one.** Twice
   now, Claude has asserted something false in a prompt and the peer's most
   valuable act was refusing to comply:
   - Claude claimed a 4-byte gap proved a class contained a `double`. Codex
     searched, found no `lfd`/`stfd` anywhere, and said "cannot distinguish".
     Claude was wrong — the real rule is that **MWCC aligns a `.bss` object to 8
     when its SIZE is a multiple of 8**, nothing to do with members.
   - Claude asked Codex to name an offset as a new field. It was already
     `mPlayerLayer`, a matched referenced member. Codex reported the
     contradiction instead of splitting a pad that did not exist.

   **Treat anything Claude asserts that you can measure as a hypothesis.** A
   confident wrong answer costs a failed link and an ambiguous diagnosis; an
   honest "I could not tell, and here is what would settle it" costs nothing and
   is often what unlocks the real answer.

6. **A guess must be labelled a guess.** `u8 pad[N]` for a region you cannot
   explain is a good answer. An invented member name is not. Mark anything not
   from official sources with `@unofficial` in a doc comment, as existing headers
   do.

7. **Say whether a change is offset-perturbing**, per change, and how you know.
   Splitting a pad into `pad + field + pad` of the *same total* is safe. Getting
   the total wrong shifts every following member and is invisible to every
   per-function diff.

## 5. Where the evidence is, ranked by strength

1. **Already-matching code in `source/`.** An offset or a call shape observed in
   banked, byte-exact code is a **fact**, not an inference. Strongest thing you
   have.
2. **`bin/dtk/dtk_splits_wiimj2d.txt`** — official per-source-file section ranges
   for already-split TUs. Where a bound is adjacent to an entry here, that is
   hard bracketing. It sat unused for most of the project and then immediately
   caught a recorded `.sbss` bound that was 0x28 bytes short.
3. **`bin/dtk/wiimj2d_symbols.txt`** — every symbol: mangled name, section,
   address, size. Your primary tool.
4. **`bin/dtkspl/obj/auto_*_*.o`** — the original binary split into objects by
   address. Disassemble with
   `bin\dtk-windows-x86_64.exe elf disasm <obj> <out.txt>`.
5. **Bounds by elimination** — the weakest kind of claim. It has been wrong
   twice. If elimination is all you have, label it as such.

### Techniques worth knowing

- **`(vtable size - 8) / 4` = the number of virtual slots.** A `0xC` vtable means
  exactly one virtual function — the destructor. The cheapest possible check on a
  reconstruction, and it has caught extra virtuals twice.
- **Consecutive `@NNNNN` pool IDs attribute an anonymous object to a TU.** The
  cheapest attribution evidence in the project. It settled an ownership question
  that elimination had got backwards, with the IDs running consecutively *across
  two different sections*.
- **The CFront length prefix recovers a class name.**
  `setPauseEnable__14PauseManager_cFb` → the class name is exactly 14 characters.
  This is how a class that existed nowhere in the tree was recovered.
- **`STATIC_ASSERT(sizeof(X) == N)`** in a throwaway `scratch/` file, compiled.
  A size you have not compiled is a claim, not a result.
- **An inline body in a header can emit a weak copy** into every TU that uses it.
  When proposing a class, declare destructors **without** inline bodies unless
  you have evidence the original inlined them.

## 6. Things that are true about MWCC and cost someone a round

- **`.bss` object alignment follows SIZE, not type alignment.** Size a multiple
  of 8 → placed 8-aligned. A `char[0x18]` (alignment 1) still gets 8-aligned
  placement. **A gap in `.bss` is therefore not evidence about a class's
  members.**
- **`const` can delete your object.** A file-scope `const int` with a constant
  initialiser gets folded away entirely by `-O4` and is never emitted, leaving
  the section short. Three constants had to be made **non-`const`** to exist at
  all.
- **`extern` is load-bearing on an unreferenced `const` array.** At namespace
  scope a `const` array has internal linkage in C++, so it is stripped as unused.
- **`T *const arr[]` is a const-qualified type** and lands in `.rodata`, not
  `.data`. Dropping the outer `const` moves it.
- **A class's vtable is the terminal `.data` object of its TU**, unconditionally.
  Anything after it belongs to the next TU — a free upper bound.
- **A header static with a non-trivial constructor is emitted into EVERY TU that
  odr-uses it.** One such object has 30 copies in the original.
- **Inlining depends on the caller's size.** One function in the game *calls* an
  inline that all ~20 other call sites inline, purely because it is large enough
  that MWCC's per-caller inline budget gives up. Do not reach for `NOINLINE` to
  reproduce that — it would break the 20 sites that legitimately inline.
- **An object nobody references can still be required.** A 0x40 float table the
  entire binary never reads still has to be emitted, or the section is short.
  **An unclaimed object is a finding, not noise.**
- **An empty constructor or destructor defined INLINE in the class gets `weak`
  linkage; defined out of line in the `.cpp` it gets `global`.** Both compile,
  both look right, and only one matches. `mPad::PadAdditionalData_t`'s
  constructor and destructor are `global` in retail, so their empty bodies
  belong in the `.cpp`, not in the header. If a compiler-generated function
  comes out `weak` when the map says `global`, this is the first thing to check.
- **`__sinit_\<file>_cpp` lives in its own split object**, named
  `auto_sinit_<file>_cpp_text.o` under `bin/dtkspl/obj/` — see
  `tools/auto_decomp/prepare.py`. It is therefore **absent from the `auto_03_*`
  disassembly either side of it**, and a brief that hands you those two files has
  silently omitted it. Disassemble it directly rather than concluding the
  function does not exist.
- **A `0x10`-ish unclaimed `.bss` hole next to a static array of objects with
  destructors is probably not yours to declare.** It is the bookkeeping node
  MWCC synthesises for `__register_global_object` (`0xC` bytes) plus alignment
  padding. Confirm with `dtk elf info` on your own compiled object before
  inventing a declaration to fill it.
- **A destructor of `0x40` is not evidence of owned resources.** That is roughly
  the size of the standard two-argument destructor ABI wrapper (`this` plus the
  deleting flag) with no member destructor calls and no vtable store at all.
  Read it instruction by instruction before inferring members from its size.
- **Your draft file's NAME is part of the object code.** Anonymous-namespace
  symbols mangle as `name__NN@unnamed@<filename>_cpp@`, where `NN` is the length
  of that string. A draft compiled as `assembled_static.cpp` produces
  `scCoinMax__30@unnamed@assembled_static_cpp@`; the target has
  `scCoinMax__33@unnamed@d_a_player_manager_cpp@`. **Every one of those lines
  will diff forever, and no source change will ever fix it.** Name your draft
  exactly what the landed file will be named, from the first compile.
- **When two functions in one TU compile the same expression differently, every
  explanation that lives in the shared header is already dead.** `existCheck`
  and `save` in `d_nand_thread.cpp` both test `mError == 0` right after the same
  call; one gets `cmpwi`, the other `cntlzw`/`srwi.`. A member qualifier cannot
  do that, because it would move both. Look for the difference locally, in the
  shape of the two function bodies. This retired a header proposal that had
  already survived one round of review.
- **Unreferenced weak symbols are not placed by the linker.** An object whose
  `.text` exceeds its slice claim is normal, not a defect — it is the standard
  condition of roughly two thirds of the banked units. See the Weak-Symbol
  Linker Placement Rule in `HANDOFF.md` before reporting an overflow.

### Two checks that catch a whole class of silent error

Run both on every set of section bounds you propose. They are mechanical, they
take seconds, and each has already caught a wrong answer that read as confident.

1. **The overlap-and-adjacency check.** Load `slices/wiimj2d.json`, and for each
   range you propose, find every slice whose range intersects it (must be none)
   and the slices that end just below and start just above. Real bounds are
   usually *exactly* adjacent to a neighbour in several sections at once. A
   proposed `.text` for `d_a_en_coin_main.cpp` was found sitting inside
   `d_a_en_dfpakkun.cpp`'s claim this way.
2. **The base-address check.** That failure had one cause: the address was
   converted to a file offset using `0x80004000`, which is `.init`'s base, not
   `.text`'s `0x80006780`. Every other section in the same proposal was correct.
   **State the base you subtracted, per section, in your report**, so the
   arithmetic is checkable without redoing it. The section bases are in
   `slices/wiimj2d.json` under `meta.sections`; read them, do not recall them.

## 7. Environment

Windows. Specifically:

- **`dtk` fails on relative paths with forward slashes.** Use an absolute path to
  `bin\dtk-windows-x86_64.exe`.
- **PowerShell 5.1 parses 8-hex-digit literals as negative `Int32`.** Do all
  address arithmetic in Python.
- Splat native-exe arguments rather than passing one string.
- **Files must be LF with no UTF-8 BOM.** Write your response file as plain ASCII
  or clean UTF-8 — responses have arrived with mangled dashes and `?`
  substitutions, which makes them harder to act on and can corrupt a pasted
  header.

## 8. What a good response looks like

Per item: **the evidence**, **the proposal**, **your confidence**, and **whether
it is offset-perturbing**. Concretely:

```
### <target>
- Evidence: <symbol / file:line / the actual instructions>
- Proposal: <full text, from scratch/, compiled>
- Compiled: YES (static assert passed) / NO (closest was 0xNN)
- Confidence: high / medium / low, and what would raise it
- Offset-perturbing: NO, because <reason> / YES, and here is the impact
```

If you could not settle something, **say so plainly and say what would settle
it.** That is a real deliverable. It is much cheaper than a confident wrong
answer, and on this project it has twice been the thing that produced the
correct answer.

**The written response is the deliverable, not the artifacts.** A round arrived
recently as a scratch directory containing two compiled variants and no report.
The two objects turned out to differ only in the filename embedded in their
anonymous-namespace symbols — so either the experiment's variable was never
actually varied, or it was varied and is codegen-neutral. **Those are opposite
conclusions and the artifacts cannot distinguish them.** The round produced
nothing usable. If you compile a variant, record in the response what you
changed, what you expected, and what you measured; an unexplained `.o` is not
evidence of anything.

If you finish early, say so and ask for more work rather than picking your own
next target — the tree is usually busy and collisions are expensive.
