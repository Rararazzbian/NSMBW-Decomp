# AGENT_CONTEXT.md — shared briefing for every AI working this repo

**Read this once per session, before your round's prompt.** It is the material
that does not change between rounds. Your prompt file
(`QWEN_PROMPT.md` / `GEMINI_PROMPT.md`) contains only what is specific to your
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
| **Claude** (lead + sub-agents) | The current translation unit. Runs the shared build. The **only** one who edits `slices/wiimj2d.json`, `syms.txt`, and any shared header. | `QWEN_HANDOFF.md` |
| **QWEN** | Whatever `QWEN_PROMPT.md` assigns. `QWEN_HANDOFF.md` is its private notebook. | `wip/`, `HANDOFF.md`, `GEMINI_*.md`, `AGENT_CONTEXT.md` |
| **Gemini** | Whatever `GEMINI_PROMPT.md` assigns. | `wip/`, `HANDOFF.md`, `QWEN_*.md`, `AGENT_CONTEXT.md` |

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
   - Claude claimed a 4-byte gap proved a class contained a `double`. QWEN
     searched, found no `lfd`/`stfd` anywhere, and said "cannot distinguish".
     Claude was wrong — the real rule is that **MWCC aligns a `.bss` object to 8
     when its SIZE is a multiple of 8**, nothing to do with members.
   - Claude asked QWEN to name an offset as a new field. It was already
     `mPlayerLayer`, a matched referenced member. QWEN reported the
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
- **Read a function that already MATCHES before theorising about one that does
  not.** This is the highest-yield technique in the project and it has now paid
  twice in one day. Two sessions went into arguing about a codegen idiom while a
  byte-exact function twenty lines away in the same file demonstrated it in
  plain source. Later, a mystery stack area that survived nine invented variants
  was explained in one round by finding the same pattern in the landed
  `dCourseSelectGuide_c::PlayerIconSet`.

  The mechanical version: `grep -rla <the instruction or symbol that puzzles
  you> bin/compiled/wiimj2d`, cross-reference the hits against
  `slices/wiimj2d.json` to keep only banked units, then read their source in
  `source/`. **A matching function is stronger evidence than any A/B compile on
  a draft**, because it is the original authors' own idiom rather than a shape
  you reverse-engineered.

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
- **`-inline noauto` still inlines a member defined IN the class body. It
  declines to inline one defined out of line, even with the definition visible
  and `-ipa file` on.** That distinction is load-bearing and it is the whole
  reason one shape works where an almost identical one does not. A carrier
  struct handed to an **in-class** setter inlines away to nothing and leaves
  behind exactly the target's dead stores; the same setter moved out of line
  emits a real `bl`.
- **A local filled and then passed to a call that inlines away leaves stores
  nobody reads — and those stores are correct, not a bug.** This is how a target
  ends up writing six floats to `r1+0x8` and never reading them. Found by
  reading the landed, byte-exact `dCourseSelectGuide_c::PlayerIconSet`, where an
  `mVec3_c` local does exactly this. Reproducing it also fixed a frame size that
  was wrong by `0x80`.
- **`-inline noauto` blocks inlining an out-of-line member function even when
  its definition is visible in the same TU, and `-ipa file` does not override
  it.** So a local instance of a class with a user-declared constructor and
  destructor always emits real `bl __ct__` / `bl __dt__` calls. Measured: a
  6-float local cost 152 instructions declared-only and 149 with the bodies
  present, against a 121-instruction target. **A function whose target has no
  such calls therefore has no local of that class**, however well the sizes line
  up — which kills the otherwise attractive theory that dead stores survive
  because a destructor keeps the storage alive.
- **`add rD, rBase, rIndex` keeps the result in the BASE's register.** In
  straight-line, call-free, branch-free code MWCC always folds base+index into
  the register holding the *address*, never the one holding the *scaled
  magnitude* -- the index's one-shot value dies at the fold while the base's
  pointer role continues. Derived from a sweep of all 145 landed byte-exact
  objects: 3 clean confirmations and 4 boundary cases where a third register
  wins instead (an intervening `bl` forcing a nonvolatile, the fold becoming an
  immediate call argument forcing `r3`, or a conditional branch between the
  access and the fold).

  **The sweep also produced a striking negative: no landed TU anywhere contains
  the store-first form of the idiom** -- indexed store, then fold, then
  displacement stores -- which is exactly `m_pad::clearWPADInfo`'s shape. 60+
  `st**x` sites across 16 files, all load-first. So when a residual has no
  precedent in the entire matched corpus, that is itself evidence the shape is
  wrong, and worth checking before assuming the allocator is at fault.
- **Declaration order does not drive saved-register assignment for GPRs. It DOES
  for FPRs.** The original measurement stands as far as it goes: all six
  orderings of three hoisted base **pointers** in `beginPad` produced
  byte-identical output.

  **But this entry used to generalise from that to "treat a pure
  register-permutation residual as not source-addressable", and that conclusion
  was wrong and expensive.** Everything about it was measured on GPRs and it does
  not transfer. Levers 12 and 13 both fix register permutations from source, and
  `CalcAdjustPosY` (128w) turned out to have a residual of exactly one FP
  register pair -- `x` and `fabs(b)` swapped between `f29` and `f30` -- fixed
  purely by source shape.

  The FPR rule, measured: **callee-saved `f31…f28` are handed out in DECLARATION
  order, while the instruction schedule follows ASSIGNMENT order.** Retail
  sometimes needs those to disagree, and you decouple them by splitting the
  declaration from the assignment:

        f32 absB;                    // declare here -- fixes which register
        f32 x = GetPos().x;
        absB = std::fabs(b);         // assign here -- fixes where it is computed

  Merely moving `f32 absB = std::fabs(b);` above `f32 x` fixes the registers but
  hoists the `fabs` into the prologue, killing the `fmr` that preserves `b`
  across the first call, and the function comes out one word SHORT. The split
  satisfies both constraints at once.

  A second FPR rule from the same round, and it is genuinely counter-intuitive:
  **a leaf WITH a def-point and a leaf WITHOUT one obey opposite numbering
  directions.** Def-pointed values number ASCENDING in declaration order; bare
  leaves number DESCENDING in evaluation order. So a statement pair with four
  leaves can need its two orderings written backwards relative to each other --
  in `check_term`, declare the Y base first, then compute the X sum first. That
  was found by measuring ~25 variants, and the score went 12 → 8 → 0.

  So: for a GPR permutation, believe the original finding. For an FP register
  permutation, it is source-addressable and levers 12 and 13 are where to start.
- **`r1+0x8` is the outgoing parameter save area, not where locals sit.** Stores
  there that are never read back are more likely argument space for a by-value
  struct, or a struct-return slot written through a hidden pointer, than a local
  variable. Check that before modelling them as one.
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

## `.text` byte-identity does not prove a unit correct

Established by landing `d_a_wm_grid.cpp`, which was called "10/10 complete" and
had **four** independent defects, none of which `.text` could show.

**Two functions with the same body are indistinguishable in `.text`.** An actor
whose `create`, `execute`, `draw` and `doDelete` all compile to `li r3, 1; blr`
will produce a byte-identical `.text` under *any* permutation of those four. The
only evidence for which is which is the **vtable relocations**. Grid had
`doDelete` where the original has `execute`; fixing it meant reordering the
definitions in the `.cpp`. Before landing an actor, read the vtable relocations
and confirm each trivial function sits in the slot the base class dictates.

**Check every section, not just `.text`.** `progress.py --verify-obj` does this
for units already in the slice file, but only warns -- and `.text` over-claim is
normal, so the real signal hides among benign warnings. For a draft, use
`wip/wm_units/check_sections.py <draft.cpp|.o> <module> '<slice JSON>'`, which
treats `.data`/`.rodata`/`.bss`/`.ctors` over-claim as fatal and dumps the
offending section's symbols.

**Sanity-check the vtable size.** `(vtable size - 8) / 4` is the slot count.
Compare slot-for-slot against a landed sibling. Grid declared 23 where the
original has 22; tower declares 28 where the original has 30.

## Unit bounds: derive them from the object, not from the named symbols

Two errors that each cost a landing attempt:

- **`.data` opens before the first named symbol.** The anonymous string literals
  a header static points at (`"F7C0"`/`"W7C0"` from `dWmLib::sc_ForceList`) lead
  the section. They are `LOCAL`, so they are never merged or dropped, and they
  belong to the unit. A claim starting at `sc_ForceList` is 0x10 too high.
- **`.text` does not end at `__sinit`.** MWCC emits the array destructor for a
  header static LAST in the object, so the unit extends past `__sinit`. Grid's
  `__sinit` at `0x164380` is followed by its array destructor at `0x164410`; the
  `__sinit`'s own relocation to it is how you find it.

**The technique that settles these in one step: compile a landed, byte-exact
sibling and read its object.** `d_a_wm_cloud.cpp` answered both questions
immediately and refuted two hypotheses I would otherwise have tested by
guesswork.

## Seeding the `.rodata` constant pool

To force a constant into the pool that no placed function references, only one
of the three forms works:

| form | result |
|---|---|
| `static` function holding `static const float U[]` | dropped; pool entry lost |
| plain global function | **placed**; `.text` overflows the claim |
| `DECL_WEAK` global function | correct: deadstripped, pool entry kept |

`d_a_wm_cloud.cpp` uses a plain global `DUMMY_UNUSED()` only because its `.text`
claim has room for it. Note that name also already exists in that module.

## When `--verify-bin` fails, diff the binary before re-reading source

1. Section **sizes** vs `original/<mod>.rel` -- an over-size names the section.
2. Section **contents** byte by byte.
3. If all sections match and the md5 still differs, decode the **relocation
   table**. Grid's last defect was three bytes: a permutation of `0x40/0x50/0x60`
   across three relocation entries, i.e. mis-assigned vtable slots.

## Check the vtable SLOT ASSIGNMENT — `wip/wm_units/check_vtable.py`

An actor's `create`, `execute`, `draw` and `doDelete` often all compile to
`li r3, 1; blr` -- **identical bytes**. Any permutation of them across their
vtable slots gives a byte-identical `.text`, so a per-function diff reports a
clean 10/10 for a class that is semantically scrambled. The failure only
surfaces at link time, as a few differing relocation bytes. This was present in
three units in one day.

```
python wip/wm_units/check_vtable.py <draft.txt> <target_data.txt> \
    <target_vt_label> <lo> <hi> <target_text.o> [...]
```

It pairs draft functions to target addresses by **placement order** -- targets
in ascending address, consuming drafts in object order -- because that is what
actually determines a function's address. Do not "improve" it to look
functions up by content: content cannot disambiguate two identical bodies,
which is the whole point, and the first version did exactly that and condemned
the landed, 5/5-verified `d_a_wm_grid.cpp`.

It distinguishes non-defects from real ones: a slot whose target function is
still differing cannot be checked yet (`unverifiable`); everything else is a
real `WRONG SLOT`.

**A slot pointing OUTSIDE the unit is not automatically a blocker.** I said it
was, all session, and that is wrong. Two different things hide there:

- an inherited **inline** virtual whose body a header already defines. Harmless.
  Just do not declare it in your class and let it inherit. dtk often leaves
  these as bare `fn_2_*` because no landed sibling has referenced them by name
  yet -- a display gap in the tooling, not a linkage gap in the binary.
  `d_a_wm_sandpillar.cpp`'s slot 5 pointing at `fn_2_15ABC0` is this:
  `li r3, 0x1; blr`, and `d_wm_demo_actor.hpp` already declares
  `virtual int doDelete() { return SUCCEEDED; }`. An unrelated vtable elsewhere
  points at the same address for the same slot, which only happens if neither
  class overrides it.
- a real **out-of-line** method in an un-decompiled TU. That IS a blocker --
  `d_a_wm_kinoko_1up.cpp` needs `daWmKinokoBase_c`'s ctor and dtor.

Discriminate by size and body: a 2-instruction `li rN, X; blr` at an address a
header defines inline is the harmless case. Check before parking a unit.

Regression cases, both of which must print `VTABLE CLEAN`:
`bin/compiled/d_basesNP/d_basesNP/bases/d_a_wm_grid.o` and `d_a_wm_tower.o`.

**Run all three checks before calling a unit ready:** `verify_anon.py` for the
functions, `check_sections.py` for the section sizes, `check_vtable.py` for the
slot assignment. Any one of them alone will pass a unit that does not link.


## Validate BOUNDS before building — `wip/wm_units/check_bounds.py`

Bounds are the largest single source of wasted effort here: two landings failed
on them today, and a third draft had a `.data` span with the RIGHT SIZE and the
WRONG ADDRESSES (`0x44a9c-0x44cb4` vs the real `0x44a68-0x44c80`, both `0x218`).
A size check reports `ok` on that. This validates a proposed slice block against
dtk's target symbol map BEFORE anything is compiled:

```
python wip/wm_units/check_bounds.py <module> '<slice JSON>' [source-to-skip]
```

- every range must start on a real symbol boundary and end where one ends
- no overlap with any range already in the slice file
- gaps to neighbours reported (an unexplained gap usually means an
  unidentified unit, not free space)
- the two `wm`-family rules, both of which were landing-breaking errors:
  `.data` opening on `g_profile_*` is 0x34 too high; `.data` opening on a `0x24`
  object (`sc_ForceList`) is 0x10 too high, because the two anonymous 5-byte
  strings come first; and a `.text` claim whose next symbol is `0x1c`
  (array-destructor sized) ends too early, because a unit ends after its OWN
  array destructor, past its `__sinit`.

Regression cases: grid's original bounds and the wrong-span ghost claim must
both FAIL; the landed grid, tower and cloud bound sets must all pass.

## The four checks, and why each alone is insufficient

| tool | catches | blind to |
|---|---|---|
| `check_bounds.py` | wrong span, overlap, family bound rules | anything about the source |
| `verify_anon.py` | per-function codegen | section sizes, vtable, pooled constant VALUES, symbol names |
| `check_sections.py` | section sizes | wrong span of the right size; slot assignment |
| `check_vtable.py` | slot assignment, wrong base class | everything else |

**None of them catches an unresolved symbol.** A unit can pass all four and
still fail to link — `d_a_wm_kinoko_1up.cpp` is 9/9 with every check clean and
cannot land, because it inherits from an un-decompiled TU. If `check_vtable.py`
prints `skip (inherited from another TU at 0x...)`, that unit is blocked on
whatever owns that address.


## Function DEFINITION ORDER is part of the object

`verify_anon.py` matching every function proves nothing about their order, and
the linker lays a unit's `.text` down in object order, which is source
definition order. `d_a_wm_smallcloud.cpp` reported a clean **16/16** while
defining `processCutsceneCommand` before `createModel` where the original has it
after `mode_exec`. Every function was byte-identical; the module still failed to
match, because every `bl` past that point had the wrong displacement.

`verify_anon.py` now checks this and prints `FUNCTION ORDER IS WRONG` with the
target's order when the matched draft indices are not ascending.

The same applies to **`.bss` and `.data` statics**: declaration order sets their
layout. smallcloud needed `resAnmNames` declared before `sInit` inside
`createModel` -- with them the other way round the sections were the right SIZE
and the objects sat at the wrong addresses, which showed up as six differing
relocation bytes and nothing else.

**When a unit is byte-perfect in every section and still fails, diff the
relocation tables.** Three landings today came down to that: grid's vtable slot
permutation (3 bytes), smallcloud's `.bss` static order (6 bytes).


## MWCC levers that have actually closed functions

Ordered by how much they moved a real function. Try these before inventing
variants.

1. **`if`/`else if` dispatch chain -> `switch`.** Took one unit's
   `processCutsceneCommand` from 192 to 24 differing in a single change. MWCC
   collapses a branch in the chain form where the target emits explicit
   sequential compares. **It does not always transfer** -- on another unit whose
   dispatch was already a `switch`, forcing a case to the front made it 173 ->
   215. Measure, do not assume.
2. **A ternary between two adjacent small constants compiles to ARITHMETIC**
   -- the most reliable lever found so far, confirmed on two different units.
   (`neg`/`or`/`srawi`/`addi`), not a branch. `x ? 10 : 11` became straight-line
   maths where the target has branch-and-store. Writing it as an explicit
   `if`/`else` took a function 167 -> 78.
3. **Case LABEL declaration order sets body-block layout**, independently of the
   dispatch comparison order -- the compares stayed byte-identical while
   reordering the labels went 78 -> 30. This is the `switch` analogue of
   "definition order sets `.text` placement".
4. **Hoist singleton loads into named locals at the top** if the target loads
   them before first use rather than lazily where used: 184 -> 167.
5. **Branch polarity: settle it from the target's `beq`/`bne` bytes**, never from
   guessed semantics. A `==`/`!=` flip has closed functions three times today,
   most cheaply as `if (!X) {A} else {B}` -> `if (X) {B} else {A}`.
6. **Passing a global's address vs staging through a stack temporary.** The
   target staged `mVec3_c::Zero`'s three floats through a local before the call;
   passing `&mVec3_c::Zero` directly was 2 instructions off.
7. **`!(a < b)` and `a >= b` are NOT the same to MWCC.** The negated-less-than
   form emits the fast `bge`/`ble` branches; the direct `>=` form emits
   `cror`-combined ones. This closed `approach()` outright. Try both phrasings
   whenever a float comparison's branch shape is wrong.
8. **`return a == b;` compiles branchlessly** via `cntlzw`/`srwi`; nested
   `if`/`return` gives the target's `bnelr` early returns.
9. **Function-local `static` -> file scope** changes instruction scheduling. It
   fixed a function stuck at 5/7 through six other permutations -- and
   REGRESSED a different unit 10 -> 18 by pulling a table out of a merged
   `.rodata` pool. Unit-specific; measure.
10. **AGGREGATE COPY defeats CSE -- use it when the target RELOADS a field.**
    The field loads MWCC emits for `local = obj;` are *not*
    common-subexpression-eliminated against earlier scalar reads of the same
    members. So when the target re-reads a field the compiler would rather keep
    live in a register, write:

    ```cpp
    mVec2_c newBase = mUnitBasePos;   // both fields reload here
    newBase.x += 16.0f;               // ...and the add is recomputed
    ```

    not the equivalent-looking `mVec2_c newBase(mUnitBasePos.x + 16.0f,
    mUnitBasePos.y)`, which lets `-O4` reuse a sum computed a line earlier.
    This closed the reload, the re-add *and* the store order on nine functions
    in `d_line_mng`, and took `fn_800C31C0` from 547 to 549. **It is the
    authentic replacement for a `volatile` cast** -- the two compile to
    byte-identical code, so never ship the `volatile`.

11. **SPLIT THE ASSIGNMENT when a `member * literal` product has its operands
    in the wrong registers.** Write:

        mSpeed.x = mBaseSpeed;
        mSpeed.x *= 0.8910065f;

    not `mSpeed.x = mBaseSpeed * 0.8910065f;`. TWO stacked rules make this
    work, which is why one-shot attempts at such a permutation keep failing:
      1. **A multiply's variable operand only reaches `f1` if it has a
         def-point of its own ahead of the multiply.** Written as one
         expression it is a bare operand and lands in `f0`.
      2. **MWCC puts a float LITERAL in the FIRST `fmuls` source slot**,
         always, regardless of how the source is written. (This is the
         mechanism behind the commutative dead end below.)
    `x *= k` satisfies both at once: the assignment supplies the def, and a
    compound assignment is not a binary expression so rule 2 does not apply --
    the destination IS the first operand. Closed three functions in
    `d_line_mng` outright and improved all eight siblings; 27.8% -> 31.8%.
    Constraints, all MEASURED: apply it to the MEMBER, not to a scalar temp
    (a temp re-schedules the prologue and loses the match); the def must be
    adjacent, since hoisting to function top costs +3 by forcing the
    callee-saved `f31`; and do NOT extend it to a dependent half-product
    (`y = 0.5f * x`), which destroys two of the three matches.

    **Corroborated against the matched corpus, not just by experiment.** A scan
    of all 145 already-matching objects classified every `fmuls` pairing a
    member load with an `.sdata2` literal: 77 CONST-IN-A vs 38 MEMBER-IN-A,
    with a 100%-consistent source correlation and no exceptions. So when a
    residual is this operand order, the source shape is DETERMINED -- read it
    off the disassembly rather than guessing:

    - `fmuls fD, fLIT, fMEM` (const first) <- plain `x = member * literal`.
      A named `static const float` behaves identically to a literal here.
    - `fmuls fD, fMEM, fLIT` (member first) <- one of exactly THREE routes:
        1. compound assignment, `member *= literal`
           (`mSpeedF *= 0.7f` in `d_a_player_base.cpp:1226`);
        2. **division by a constant**, which is rewritten to a reciprocal
           multiply (`mPos.y / 16.0f` in `d_a_en_shell.cpp:733`);
        3. a factor that is a **variable at parse time** and only becomes a
           literal after inlining/const-prop -- e.g. a `static inline` helper
           taking a `float rate` parameter, called with `7.0f`
           (`d_a_en_bigpile.cpp:33`).

    Mechanism: MWCC's FRONT END canonicalises `expr * <syntactic float
    literal>` by hoisting the literal into the first source slot. All three
    routes bypass that canonicaliser because none of them presents a syntactic
    literal to it. Note route 3 requires a real inline-function parameter -- a
    plain `const float` local does NOT work, const-prop folds it before the
    canonicaliser sees it, and it measured strictly worse.

    Retail's own code contains the A/B proof of the commutative dead end
    below: `d_actor.cpp:437` `mVisibleAreaSize.x * 0.5f` and
    `d_a_en_jimen_pakkun_base.cpp:403` `0.5f * mVisibleAreaSize.x` are two
    matched files spelling the same expression both ways, with identical
    output.

    **Known limit.** This rule addresses LOAD order into f0/f1. It cannot fix
    slot choice between two operands that are ALREADY LIVE in registers. In
    `executeState_Left30Right` retail wants const-first at one occurrence and
    member-first at another *of the same source statement*; `/ 2.0f`, `*= 0.5f`
    and `* 0.5f` each fix one and break the other. That last word is a
    different phenomenon.

12. **FP REGISTER NUMBERS ROTATED but the instructions are otherwise right?
    That is EVALUATION ORDER, and the same def-point rule fixes it -- on `+`
    and `-`, not just `*`.** The mechanism, measured:

    - MWCC numbers a statement's FP leaves **descending from N-1 in
      EVALUATION order**.
    - Evaluation order is **heavier-subtree-first** (Sethi-Ullman). When the
      two subtrees TIE, it falls back to source order.
    - Retail numbers in **source order**.

    So a balanced statement matches by accident and an unbalanced one does
    not. `mPos.y = (mUnitBasePos.y - 16.0f) + (mPos.x - mUnitBasePos.x)` ties
    and already matched; `mPos.y = mUnitBasePos.y - (mPos.x - mUnitBasePos.x)`
    has a heavier right subtree, so the draft evaluated it first and numbered
    `mPos.x=f2, mUnitBasePos.x=f1, mUnitBasePos.y=f0` where retail has
    `f2/f1/f0` over `mUnitBasePos.y, mPos.x, mUnitBasePos.x`.

    **Fix: give the LIGHT LEFT operand a def-point of its own ahead of the
    operator.**

        mPos.y = mUnitBasePos.y;
        mPos.y -= (mPos.x - mUnitBasePos.x);

    Same constraint as lever 11: it must land on the **member**, not a scalar
    temp. A named local for the left operand fixes an `initializeState_`
    variant but takes its `executeState_` sibling to 13 diffs; the self-assign
    form fixes both. The `+` form is identical:
    `mPos.x = mUnitBasePos.x; mPos.x += 16.0f;`.

    Closed five functions in `d_line_mng` in one round.

    **CORRECTION.** This entry used to say "do NOT reach for it when the residual
    is in a DOUBLE-precision `fadd`/`frsp` path -- that is a different class".
    **That was wrong and it cost time.** The def-point split is exactly what
    closes the double-precision class too; it was written off after one failed
    attempt whose real defect was something else. Measured A/B, everything else
    in the body held fixed:

        mPos.x = mUnitBasePos.x + 8.0;              -> fadd f2, f2, f3  (literal first, 1 diff)
        mPos.x = mUnitBasePos.x; mPos.x += 8.0;     -> fadd f2, f3, f2  (member first,  0 diff)

    One instruction moved, nothing else in a 102-word body shifted. **So levers
    11 and 12 govern the OPERATION, not the PRECISION** -- `fadd`/`fmul` behave
    exactly as `fadds`/`fmuls`, and the compound-assignment route bypasses the
    literal-hoisting canonicaliser identically.

    What IS genuinely different about the double path, and the thing that
    actually misled the earlier attempt: assigning a double-typed expression into
    a `float` member emits a visible `frsp`. Introducing an `f32 t = ...` local to
    "hold" the intermediate reproduces that `frsp` **in the wrong register and
    about ten slots too early**, which then drags a whole scheduling window out
    of alignment and looks like a deep structural mismatch. There is no float
    local in the original -- the store to the member is dead-store-eliminated and
    only its rounding survives. Write the member, not a temp.

    This closed `executeState_Left60Down` and `executeState_Right60Down`
    (102w + 104w, both to zero diffs) and their two `initializeState_` siblings.

- **Lever 13: a member READ that is reused needs its own local -- and a SECOND
  local after a call.** This is the counterpart to levers 11 and 12, and it
  qualifies their "put the def-point on the MEMBER, never on a scalar temp" rule,
  which is stated too absolutely for this shape.

  The distinction is which side of the assignment the member is on:

  - Levers 11/12 cover a member being **written** by an arithmetic statement.
    There the def-point must be the member itself -- `mPos.x = a; mPos.x += b;`
    -- and a temp actively breaks it.
  - Lever 13 covers a member being **read** and then reused later in the same
    function. There a bare re-read gets a low-priority scratch register. Hoisting
    it into a named local (`f32 baseSpeed = mBaseSpeed;`) gives that value its own
    def-point and elevates it to the register retail uses.

  Same underlying mechanism -- a def-point raises register priority -- applied to
  the read side rather than the write side.

  **The second half is the non-obvious part.** When the value is needed again
  after a call, do NOT reuse the outer local. MWCC does not keep a local live
  across a call here; it re-reads the member from the object either way, and
  reusing the outer local forces a spill that made the function THREE WORDS
  LONGER. Declare a fresh second local inside the branch:

        f32 baseSpeed = mBaseSpeed;
        mSpeed.x = baseSpeed;
        mPos.x += baseSpeed;
        if (check_term()) {
            mPos = old;
            f32 baseSpeed2 = mBaseSpeed;   // fresh local, NOT baseSpeed
            mSpeed.x = baseSpeed2;
        }

  Reads as redundant; it is not. It gives the post-call reload its own def-point
  without requesting a cross-call live range.

  Closed all four of `executeState_Side`, `executeState_Height`,
  `executeState_CornerSideLine` and `executeState_CornerHeightLine` (51w, 51w,
  54w, 54w) with the identical shape. Note these have no multiply at all, so
  lever 11 does not apply to them -- if a same-length residual has no `fmuls` in
  it, this is the lever to reach for. Both mirrored pairs behaved symmetrically.

### Levers that are PROVEN NOT to work -- do not spend a round on these

- **Removing `mVec3_c`'s copy constructor to force a bitwise struct copy.**
  The diagnosis is CORRECT: `include/game/mLib/m_vec.hpp:140`'s user-declared
  copy constructor is exactly what suppresses MWCC's bitwise copy, and removing
  it flips a struct copy from float `lfs`/`stfs` to retail's integer
  `lwz`/`stw`. Measured and confirmed. The destructor (line 128) and the
  `(const&, float)` constructor (line 146) are red herrings; neither matters.
  **Do not land it anyway.** Blast radius measured across all 66 landed sources
  that use `mVec3_c`: **160 currently-matching functions regress**, across 49
  files, including a `__sinit_*` / `__arraydtor$*` pair per TU. Nothing is
  gained -- the instruction COUNT is identical either way (6 loads + 6 stores),
  so the copy shape alone never closes a length gap. If retail really was built
  with a POD `mVec3_c`, matching it is a whole-project migration, not a header
  tweak, and it must not be attempted one file at a time.
- **`fmuls` slot choice when BOTH operands are already live in registers.**
  The def-point rule (levers 11 and 12) governs which register a value is
  LOADED into. Once both operands are live, the slot is set by the front-end
  canonicaliser alone and nothing in the source moves it: `mSpeed.x * 0.5f`
  and `0.5f * mSpeed.x` are byte-identical in that position too. If retail
  wants opposite slot orders at two occurrences of the SAME statement, that is
  a source inconsistency in the original, not a context effect -- check the
  sibling functions to confirm, then spell the two occurrences differently.
  Forcing a member-first route at both just relocates the defect.

- **Commutative float operand order in the source.** `a * b` and `b * a` (and
  the `+` equivalents) compile to BYTE-IDENTICAL code; MWCC canonicalises
  commutative FP operands before register allocation. Four spellings were
  tested side by side on one function. If an `f0`/`f1` permutation is your
  residual, source operand order will not move it. See lever 11 for what DOES
  move it.
- **Naming a float constant, in any foldable form.** File-scope `const`, anon
  namespace, class static defined in the same TU, `#define`, `const` local,
  function-scope `static const`, address-taken -- all seven compile
  BYTE-IDENTICAL to the inline literal, because MWCC re-folds them. Only
  NON-foldable forms (extern, array element, struct member) change codegen,
  and those emit a symbol. Check the retail symbol map before reaching for
  one: an anonymous `scope:local` 4-byte `.sdata2` entry is what an inline
  literal compiles to and nothing else does, so if the map shows one, the
  original source spelled the number inline.
- **Translation-unit ordering and float literal-pool position.** 17 variants
  -- dummy earlier uses, injecting the statement into an earlier function,
  moving or reversing function order, shrinking the pool -- moved a register
  permutation not at all, and that includes a variant that reproduces retail's
  exact relative pool order. (Pool order IS a useful completeness fingerprint
  for a partially-written TU; it is just not a codegen lever.)
- **Compiler flags.** ~145 variants: every `-O` level and `-opt` sub-keyword,
  every scheduling setting, every accepted `-fp`/IEEE/fsel/fmadd option, all
  inlining and IPA modes, small-data thresholds. None moved an `f0`/`f1`
  permutation, the project's current flags scored joint-best, and the result
  is stable across compiler versions 1.0/1.3/1.7. Such permutations are
  decided upstream of scheduling and are purely source-shape-driven.
- **Hoisting a repeated product into a named local** to fix such a permutation.
  Tested on the same function: it perturbed register assignment across the
  whole body and made the diff strictly worse.


## A misaligned diff invents structural conclusions

`verify_anon.py` compares by raw position, not by realigned content. One missing
or extra instruction early makes everything after it read as "differing", and
worse, it makes the two sides look structurally different when they are not.

This produced a wrong diagnosis twice today. Once a residual was read as "the
target's vtable dispatch is missing from our draft" when the call was present
six instructions later. Once four individually-correct fixes were followed by a
RISING count (176 -> 182 -> 190 -> 192) because a structural defect still
dominated the score.

**When a residual suggests a structural difference, re-extract both sides by
raw content and compare them directly** before believing it. And never revert a
change purely because the count went up.


## Get the TYPE declaration right before writing any bodies

Declaring a templated member correctly instantiates a whole family of methods
that may already match the target with **zero code written**. On
`d_a_wm_sandpillar.cpp`, declaring
`sFStateMgr_c<daWmSandPillar_c, sStateMethodUsr_FI_c> mStateMgr` pulled in
`sStateMgr_c::initializeState/executeState/finalizeState/refreshState/getState/
getNewStateID/getStateID/getOldStateID`, `sFStateFct_c::build/dispose`,
`sFState_c::initialize/execute/finalize` and `sFStateID_c::...` -- all weak, all
generated from already-landed headers, and **15 of them matched immediately**.

The unit went 0/66 to 40/66 in a single round on the strength of the class
declaration plus 14 trivial stubs. Spend the time on the layout and the member
types first; the bodies are the cheap part.

## Localise a `sizeof` gap by comparing constructor offsets, not by adjusting globally

Compile the skeleton, dump your own constructor's member-construction offsets,
and lay them beside the target's. The first divergence is where a member is
missing, and the divergence SIZE is how big it is. On sandpillar this turned a
44-byte "something is missing" into **three separate gaps** (`0x4` + `0xc` +
`0x1c`) at three exact points -- the divergence grew at two places and then held
flat, which is what says "more than one gap".

Watch for assumptions superseded by measurement: `mStateMgr` had been assumed
`0x58` from a subtraction two rounds earlier; the compiled skeleton measured it
at `0x3c`, which is what revealed the third gap sitting AFTER it.


## THE STACK-SLOT WALL, SOLVED: pass by-value args through the INLINE wrapper

For a long stretch this blocked five units. Drafts matched a function's every
instruction and register but assigned the *same set* of stack slots to the
by-value argument temporaries in a different ORDER.

**Cause:** `m3d::mdl_c::create()` has two overloads -- a 5-arg real one taking a
trailing `size_t *`, and a 4-arg **inline wrapper** forwarding `nullptr`.

```cpp
mModel.create(resMdl, &mAllocator, BUFFER_..., 1, nullptr);  // WRONG
mModel.create(resMdl, &mAllocator, BUFFER_..., 1);            // RIGHT
```

The emitted call is byte-identical either way (same mangled 5-arg target, the
wrapper inlined away at `-O4`). But passing a by-value struct through an inlined
wrapper's parameter binding anchors its temporary as an **early, forward-order**
allocation; passing it directly leaves it in the same reverse-order pool as
every other pending temporary. That is the whole "non-loop groups forward, loop
groups reversed" asymmetry.

On `daWmKinokoBase_c::createModel` this took **10 differing to 0** and the unit
to 17/17.

**It does NOT fix every instance of the symptom.** `d_a_wm_koopa_castle.cpp`'s
`createModel` shows the same 3-way slot permutation (`0x8/0xc/0x10` against
`0x10/0x8/0xc`) and the wrapper form changes **not a single emitted
instruction** there -- confirmed by three measured experiments. So the wrapper
overload is one CAUSE of the symptom, not the whole story: check it first
because it is cheap and it did solve one unit outright, but a residual that
survives it is still open. **Check every `create()`-family call site for a spelled-out trailing
`nullptr`.**

**The pattern is family-wide, not specific to `mdl_c`.** `d_a_wm_sandpillar.cpp`
had three of them in one function, and fixing all three took it 12 differing
to 4:

| header | wrong | right |
|---|---|---|
| `m_3d/mdl.hpp:52` | `mModel.create(resMdl, &alloc, opt, 1, nullptr)` | drop the `nullptr` (4-arg) |
| `m_3d/anm_chr.hpp:19` | `mAnim.create(resMdl, resAnmChr, &alloc, nullptr)` | drop it (3-arg) |
| `m_3d/anm_tex_srt.hpp:30` | `mAnimTexSrt.create(resMdl, res, &alloc, nullptr, 1)` | drop it (4-arg) |

Before concluding a residual is this pattern, **check the header for an
alternate overload first**. `setAnm` has a single signature and no wrapper, so a
2-slot swap there is a different, smaller problem -- not every stack-slot
residual is this one.

Eleven levers had been ruled out first -- declaration order, loop shape, naming,
`const`, storage class, array size, statement position, loop-folding, an extra
temporary, argument order, and compiler version. None touched call-site ARITY.
The lesson: when permutations of *statement* structure all fail, question which
*overload* is being called.

## A missing virtual override is invisible to every check except `check_vtable.py`

If a class does not declare an override, the compiler silently fills the slot
with the inherited method. The class compiles, sections are fine, and every
function may match -- but the vtable points at the wrong function.
`d_a_wm_sandpillar.cpp` had this at slot 24. **Run `check_vtable.py` every
round**, not once at the end.


## Definition order again: interleave by ADDRESS, not by logical grouping

State-machine units are the sharp case. `d_a_wm_sandpillar.cpp` had its 27
state-triple methods grouped tidily by state at the end of the file; the target
interleaves them by address across all nine states. Restructuring the
definitions into exact target address order cut `verify_anon.py`'s order
violations from **15 to 1**.

Group source however the target's `.text` is laid out, never however reads best.
`verify_anon.py`'s `FUNCTION ORDER IS WRONG` output lists the target order --
follow it literally.

## A `u32` placeholder array hides types

`mUnkTrailing[7]` looked fine until `approach()`'s `lfs`/`stfs` proved three of
its seven slots are `float`. Sized placeholders are a legitimate way to get a
layout right, but **replace them with individually typed fields as soon as any
function reads them** -- an array of the wrong element type will silently
mis-shape every access.


## A weak symbol defined ONLY in an un-landed region will be PLACED, and breaks the build

This is a property of how the build works, and it changes when
"unreferenced weak symbols are not placed" applies.

The build reconstructs each `.rel` by compiling the landed slices fresh and
copying everything else **verbatim** from the original binary. Un-landed regions
are not linkable objects, so the linker cannot weak-dedupe a landed unit's copy
against them.

So if a landed unit REFERENCES a weak inline function whose only other
definition lives in an un-landed TU, that unit's own copy is the sole candidate,
gets placed inside or after its claim, shifts every byte downstream, and breaks
the whole-binary MD5. It is not a benign `.text` over-claim.

Measured on `d_a_wm_sandpillar.cpp`: calling `dScWMap_c::getWorldNo()` -- an
inline member declared in a shared header, so weak -- emitted
`getWorldNo__9dScWMap_cFv, weak` as a real 3-instruction function at the tail of
`.text` (`+0x10`), byte-identical to the target's own `fn_2_171400`. Not
stripped. The target resolves that call to WM_MAP's placed copy, and WM_MAP is
not landed.

**So the `extern "C"` FUN-address convention is the correct tool for a
REL-internal call into an un-landed TU**, not a placeholder that happens to
work. Do not "improve" it into a real typed call until the TU that owns the
symbol has landed.

Two other things confirmed while measuring this, worth knowing:
- `NOINLINE` genuinely expands here (`__CWCC__` is defined), so a `NOINLINE`
  inline member really is emitted out-of-line rather than folded away.
- **Calling the same function as `ClassName::method()` rather than through a
  bare `extern "C"` declaration changed MWCC's register scheduling** around the
  call site -- same statements, same order, 11 of 63 instructions differed. The
  call spelling is not neutral.

## Byte-equality is a REAL false positive, and here is a confirmed live instance

The matching gate is the UNION of raw-byte equality and canonicalised-text
equality. **Both halves can lie.** The raw-byte half lies because *relocated
address and pool-offset fields are zeroed in both disassemblies*: two
instructions that reference completely different things compare byte-identical.

This is not theoretical. Measured on `d_enemy_toride_kokoopa.cpp`:

`calcRootJntPos` and `calcShellJntPos` were both claimed as matches on the
strength of a single pooled-float store. The draft said `mRootJntPos.z = 0.0f;`
The instruction pattern matched perfectly. **The actual retail float at that pool
address is `5500.0f`.** Reading the IEEE-754 bytes straight out of
`original/wiimj2d.dol` -- map VA to file offset with `dtk dol info` -- is what
caught it. `harness.canonicalise` cannot catch this class of error: it numbers
pool symbols by order of first appearance and never inspects their value.

**So: any function small enough that its whole body is one pooled-constant
reference is under suspicion until you have decoded the constant.** The smaller
the function, the weaker the byte evidence, which is the opposite of the
intuition.

The same applies to `bl` targets. A call to the wrong function is byte-identical
to a call to the right one. Resolve the symbol; do not trust the opcode.

**There is now a tool, so there is no excuse: `python tools/auto_decomp/pool.py
@54951_8042CB1C`.** It takes a bare address or a dtk pool symbol pasted straight
out of a listing (the VA is embedded in the name after the underscore) and prints
both the 4-byte float and the 8-byte double reading. Which one is correct depends
on whether the instruction was `lfs` or `lfd` -- and that distinction is exactly
what tells you whether the original source wrote a trailing `f`.

It exists because **two agents in a row stalled on "I could not determine the
value of this constant from the disassembly", and one of them then brute-forced
constants until the bytes matched.** That always succeeds and always lies. It
reported five matched functions built on fabricated values like
`1303.79833984375f`; the real constants were `16.0f`, `-16.0f`, `32.0f` and
`0.0f`. Once decoded, the whole nine-function family was regular and obvious --
and the one constant that had "blocked" it was plain `0.0f`.

Two lessons beyond the tool. **A nonsensical constant is evidence you are wrong,
not evidence the original was strange**: a circle initialiser does not have a
coordinate of 1303.798. And **an uneven word count within an otherwise identical
family is usually not a source difference** -- there, the radius argument and
`vec.x` both travel in `f1`, so when they are equal one `lfs` serves both (13
words) and when they differ `f1` is loaded twice (14 words). Same source shape,
different operands. Do not go looking for a structural explanation until you have
decoded the operands.

Corollary for anyone reporting a percentage: **check that every function you are
counting was actually emitted.** In the same file, `tenmetsuFin` was reported as
a match while having no definition anywhere in the source -- declared virtual in
the header, never defined, never emitted. And `__sinit` was reported as a match
while sitting in a gap that the reporter's own reference dump did not cover, so
their comparison script had never looked at it; the real diff was 314 of 1,446
instructions. **A function absent from your reference dump must be reported as
UNKNOWN, never as MATCH.**

## Unsuffixed `double` literals are ORIGINAL-SOURCE style, not a bug to fix

A float expression silently promotes to `double` the moment a `double` enters
it, and MWCC then emits `fadd`/`fmul` followed by `frsp` instead of the
single-precision `fadds`/`fmuls`. The instinct on seeing `fadd`+`frsp` is to
treat it as our mistake. Often it is the opposite: the original programmers
wrote unsuffixed literals, and matching them REQUIRES us to.

Measured in `d_line_mng.cpp`, in code that is byte-exact against retail --
eleven functions, all in the `initializeState_*` / `executeState_*` families:

    mPos.y = (mUnitBasePos.y - 16.0f) + 0.5 * (mPos.x - mUnitBasePos.x);
    mPos.x -= 0.5 * (mUnitBasePos.y - mPos.y);
    mPos.x = mUnitBasePos.x + 0.5 * (mUnitBasePos.y - mPos.y);

Note `0.5` unsuffixed sitting directly beside `16.0f` suffixed, in the same
expression. That mix is deliberate and it is what retail does.

So for a same-length mismatch in float-heavy code, **the constant suffixes are a
finite checklist, not an open hypothesis.** Decode each constant out of the
target's literal pool -- 4 bytes means the source wrote `f`, 8 bytes means it did
not -- and compare that list against the draft's suffixes. Do not normalise
suffixes for consistency; consistency is not what the original had.

Related, and worth preserving when you copy a matching sibling: in `0.5 * (a -
b)` the literal is on the LEFT with the heavier subtree on the right. That
interacts with lever 12's evaluation-order rule, so reproduce the operand order
exactly rather than tidying it.

## The register rules COMPOSE within one statement pair -- they are not alternatives

Two agents independently failed to close the same 10-instruction residual, both
concluding it was not source-addressable, and between them measured 12 negative
variants. A third closed all three functions. The difference was not a new rule:
it was applying **three known rules at once, to different values in the same two
statements.**

The shape, where BOTH fields are computed from the same literal (so the literal
is CSE'd into one shared register):

    origin.y = p1.y - 16.0f;
    origin.x = p1.x + 16.0f;

There are two independent register-priority groups here, and each was wrong for
its own reason:

1. **The RESULTS** (`origin.y`, `origin.x`). Written straight to the struct
   member they number DESCENDING in evaluation order. Declaring two bare `f32`
   locals and assigning them later -- same statement positions, same schedule --
   flips them to ASCENDING declaration order, which is what retail has.
2. **The LEAVES** (the shared literal and `p1.x`). Both bare, so both number
   descending, putting the literal in the higher register. Giving `p1.x` a named
   local fixes which register it gets, but NOT its operand slot.
3. **The OPERAND SLOT.** One `fadds` still had its two live operands the wrong
   way round. Fixed by the compound-assignment route from lever 11 --
   `ox = px; ox += 16.0f;` rather than `ox = px + 16.0f;`.

Measured, each step landing exactly what was predicted: 10 diffs -> 3 -> 1 -> 0.

**Lever 11's compound-assignment route governs `+` exactly as it governs `*`.**
That was the last instruction and the least obvious: getting two values into the
right REGISTERS does not put them in the right ORDER within the instruction.

The general lesson is the one the two failed rounds missed. Levers 11 (write-side
def-point on the member), 12 (evaluation order) and 13 (read-side def-point into
a local) read like a menu to choose from. They are not. **A statement pair with
four FP values in play can need all three at once, applied to different values.**
When a residual survives every single-lever variant, try composing them before
concluding the residual is not source-addressable -- especially now that the
"not source-addressable" verdict is retired for FP registers.

## A def-point is not free: it moves the value ABOVE the bare leaves

This corrects an over-eager reading of levers 11-13 that cost a round, and it is
the counterweight to the "the levers compose" section above. **Adding a def-point
is not a neutral nudge to one value's register — it re-ranks the whole
statement.**

Measured on the `move_on_circle` family. The residual was a `+ 16.0f` that retail
compiles VALUE-first (`fadds f0, f1, f0`) where a plain expression gives
literal-first. Four different value-first routes were tried, each predicted to
land the chain in `f1`:

| route | measured |
|---|---|
| scalar local + `dstY += 16.0f` | value-first, but chain in **f3**; 1 -> 9 diffs |
| two-arg ctor arg + `dst.y += 16.0f` | **f3**; 1 -> 6 |
| `static inline addOff(a, b)` | **f3**; 1 -> 6 |
| `mVec2_c::incY(16.0f)` member | **f3**; 1 -> 6 |

Every route that creates a def-point flipped the def-pointed value ABOVE the bare
leaves, where retail has it BELOW them. **Retail's numbering here is a clean
descending run in evaluation order with no def-point anywhere.** So when the
target's numbering is a plain descending run, a def-point is the wrong tool no
matter how it is spelled -- you need value-first WITHOUT one.

The route that achieves that was already in this file, in the byte-exact
`lineB_cross_chk`:

    mVec2_c dst = mUnitBasePos;   // lever 10 aggregate copy
    dst.x += radius;              // lever 11 compound assignment ON THE MEMBER
    dst.x -= 16.0f;

The aggregate copy's own stores are dead-store-eliminated so the word count is
unchanged, and the compound assignments land on struct MEMBERS -- which is lever
11's stated constraint all along. A scalar temp was the thing failing.

**Order the operations per COMPONENT, not per operation.** `x` fully, then `y`
fully, measured 0 diffs; interleaving as `x+=r; y-=r; x-=16; y+=16` measured 6.

### Three corrections and one new diagnostic from the same round

- **Lever 11 route 3 is NOT register-neutral.** The `static inline` helper-parameter
  route does bypass the literal-hoist canonicaliser, but it creates a def-point and
  rotates the allocation exactly like a named local. Only use it where a def-point
  is wanted anyway. (Confirmed in passing that `-inline noauto` still inlines a
  free `static inline` helper.)
- **An unnamed `mVec2_c(...)` temporary takes the LOW stack slot; a named local
  takes the HIGH one** -- regardless of which branch comes first in the source.
  That is the lever for a pure stack-slot swap with everything else correct.
- **`lha` versus `lhz`+`extsh.` IS source-addressable**, previously written off.
  Retail keeps an `s32` holding the ALREADY-sign-extended angle, so write
  `s32 rel = (s16)mAngle;` and cast again at the condition (`if ((s16)rel < 0)`).
  Declaring `s16 rel = mAngle - 0x4000;` forces MWCC to keep the extended value
  as the variable; retail keeps the unextended difference in `r3` and extends
  only into `r0` for the condition code.

**The `- (-K)` diagnostic, worth knowing generally.** Rewriting `y - r + 16.0f`
as `y - r - (-16.0f)` produced `fsubs f0, f1, f0` -- exactly the target's register
topology with the wrong opcode. That **isolates "operand slot" from "register
allocation" in a single compile**, which otherwise takes several variants to
separate. It also self-checks the constant: the word count rose by one because
`-16.0f` needs its own pool entry, independently confirming retail's constant is
`+16.0f`.

## The LIMIT of the declaration-order rule: it does not move a SCHEDULING position

The FPR declaration-order rule has been unusually productive today, so here is
where it stops -- established by ~35 measured variants on `line_cross_chk2`, not
by giving up early.

**What it governs:** which register a value lands in, and which operand slot it
occupies within an instruction.

**What it does NOT govern:** *where in the instruction stream a load is
scheduled.*

`line_cross_chk2` is 100/100 words with 27 differing instructions and a single
root cause. Retail's first `0.0f` pool load lands in the register freed once
`p4.x`'s raw value is consumed, mid-subtraction. Our draft hoists that same load
to position 8 -- immediately after the prologue's `fmr f31, f1`, before the
`f30`/`r31`/`r30`/`r29` spills have even happened. Every downstream difference is
that one instruction's position cascading into renumbering. **Register CONTENTS
are already correct on both sides throughout; only the numbering differs.**

Two diagnostic probes pinned the trigger exactly:
- a **single** `0.0f` comparison placed anywhere BEFORE the first call reproduces
  the full early hoist;
- a `0.0f` comparison placed ONLY AFTER the call does not hoist at all.

So the effect is tied to "compared before the first call, in a prologue that must
save `f30`+`f31` for two calls" -- not to how many times the constant occurs, nor
to its textual position within the guard.

Measured and rejected, all worse or neutral, none landed: guard restructuring in
six forms (29-90 diffs); a named `f32 zero` local in four scopes and orders
(84-90 diffs, and one variant promoted `zero` into `f30` where `intercept`
legitimately lives, pushing the function to 99w); named write-side results for
`p4.x`/`p5.x` (neutral at 27 alone, 33-34 combined with a named constant); named
leaves used directly as the compared values (94); aggregate-copy and two-argument
constructor forms (33-101); whole-vector `operator-=` (byte-identical to manual
field subtraction, so that shape is codegen-neutral); reversing the
`slope`/`intercept` declaration order (30 -- a real effect, wrong direction).

**All four of the function's pool constants have since been decoded and are
correct** -- `0x8042CB1C`=0.0f, `0x8042CB48`=16.0f, `0x8042CB4C`=-0.1f,
`0x8042CB50`=+0.1f, matching the draft's literals exactly. So "we are comparing
against the wrong constant" is ruled out, not merely assumed away, and the
residual is confirmed to be schedule alone.

**So: when register contents are right everywhere and the residual traces to ONE
load sitting in the wrong place, reach for something other than the def-point
levers.** They control allocation, not schedule. Whether the schedule is
reachable from source at all in this shape is still open -- what is settled is
that naming, scoping and reordering the values does not reach it.

Note the contrast with `line_cross_chk1`, its sibling, closed in the same round
by exactly the axis that fails here: a bare `f32 zero;` declared and assigned
later flipped both `fcmpu` operand slots and closed it byte-exact. Same file,
same constant, same technique -- it governs the slot and not the schedule.

## `fcmpu` operand order IS addressable -- and the scope of the "commutative dead end"

The proven negative reads "commutative float operand order" and has been cited to
close out reversed-`fcmpu` residuals three times. **Its real scope is narrower
than the name suggests, and reading it broadly cost several functions.**

What is genuinely immune: **flipping the comparison's TEXT.** `0.0f == d3` and
`d3 == 0.0f` compile identically, as do the `!=` negations. MWCC canonicalises a
comparison against a syntactic float literal to literal-first, exactly as it does
for `fmuls`. Measured repeatedly; that half of the note stands.

What is NOT immune: **the operand slot itself.** Two independent routes reach it:

1. **Lever 11's route 3, applied to `fcmpu`.** Pass the constant through an
   identity helper so it is not a syntactic literal at parse time:

       static inline f32 zero_ref(f32 z) { return z; }
       ...
       if (d3 == zero_ref(0.0f)) { ... }

   The argument is a plain parameter in the AST and only becomes `0.0f` after
   inlining and const-prop, so the literal-first canonicaliser never sees it.
   Flipped both residuals to variable-first with no other instruction changed.
   Closed `line_cross_chk3` (32w) and `line_cross_slope_check` (22w).

2. **The declaration/assignment split**, where what you need is control of which
   REGISTER the constant occupies rather than bypassing the canonicaliser. A bare
   `f32 zero;` declared and assigned later closed `line_cross_chk1` (121w) in the
   same round.

Reconciling this with the warning above that route 3 is not register-neutral:
both are true. **Route 3 creates a def-point and re-ranks the statement.** Use it
where the operand slot is what is wrong and the allocation is already right (or a
def-point is wanted anyway); do not reach for it when the target's numbering is a
plain descending run with no def-point in it.

### Three smaller results from the same round

- **`return A && B;` versus nested ifs is a CFG choice, not a style choice.** The
  `&&` form compiled to a `bnelr` early-return shape; retail wanted a shared
  `return false` label, which needs `if (A) { if (B) return true; } return false;`.
- **Which block is "then" matters.** `if (b >= a)` emitted `cror`; swapping the
  bodies to `if (b < a)` gave retail's plain `bge`. Same logic, different branch
  polarity, different instruction.
- **A named local is not always right, and the mirror pair proves it.**
  `fn_800C3B20` needed named locals for BOTH reads of `mPos.x` (lever 13, fresh
  second local after the branch join). Its Y-axis mirror `fn_800C3B60` needed the
  OPPOSITE: naming the second `mPos.y` read, or naming the subtraction result,
  each dropped `baseY` from `f2` to `f1`. Only leaving it as a repeated bare
  expression -- relying on `-O4` CSE, no local at all -- reproduced retail. Two
  functions that are line-for-line mirrors of each other, needing opposite
  treatment. Do not assume a mirror takes the mirrored fix.

## RESOLVED: the `global` tag in `target.txt` is not a linkage signal

Recorded earlier as a contradiction: `fn_800C3B20` / `fn_800C3B60` are declared
`static` in the draft, while `wip/line_mng_shared/target.txt` lines 3109 and 3128
read `.fn fn_800C3B20, global`. **The tag carries no information.**

- **All 182 `.fn` lines in `target.txt` carry `global`**, and `global` is the
  only tag that appears anywhere in the file. A field with one value everywhere
  cannot discriminate.
- It is provably wrong for symbols that are not global. `target.txt` tags
  `__dt__49sFStateMgr_c<10dLineMng_c,20sStateMethodUsr_FI_c>Fv` as `global`; the
  identical template shape in another unit,
  `__dt__59sFStateMgr_c<20dCourseSelectGuide_c,20sStateMethodUsr_FI_c>Fv`, is
  `scope:weak` in `bin/dtk/wiimj2d_symbols.txt`. Template instantiation
  destructors are weak by construction, so the tag is simply a constant the
  disassembly wrapper emits.

**Where real linkage lives:** the `scope:` attribute in
`bin/dtk/wiimj2d_symbols.txt` (`scope:global` / `scope:local` / `scope:weak`).
Check that, never the `.fn` tag. But note it is present on only 811 of 16,285
functions -- `fn_800C3B20`, `fn_800C3B60`, `fn_800C31C0`, `fn_800C1EE0` and every
named `d_line_mng` function have **no** `scope:` at all, so absence proves
nothing either. The DOL is linked and stripped; `static` versus `extern` leaves
no trace in it.

**So the linkage of these two is not determinable from the target, and does not
need to be.** They match byte-for-byte as `static`, `static` is the correct
choice for a file-scope helper with no external callers, and it is what keeps the
symbol out of the link. Ship them `static`. Generally: **do not open a
contradiction against the `.fn` tag** -- confirm the field varies before reading
meaning into it.

## Report the DIFF of your matched set, not just your new closures

A verified round showed a peer reporting "8 functions closed, 3,288 bytes" while
the unit's total moved by almost nothing. The reconciliation:

    151 true  - 8 silent regressions + 10 constant fixes + 8 new + 1 unreported = 162

**Eight previously-matching functions had regressed in the same round and none of
them was mentioned.** Two were not regressions at all but outright deletions --
`calcKokoopaMdl()` and `getTorideFunfareTime()` were declared in the header and
had no body left in the `.cpp`, so they vanished from the count without ever
appearing as a failure. The cause was a from-scratch class rewrite that reordered
virtual slots and reshuffled member offsets; every function that read a moved
member broke, and the author only diffed the functions they were working on.

This is not a peer-specific failing, it is what per-function workflows do by
default. Two rules:

1. **After any change to a shared type -- member order, a new virtual, a retype
   -- re-run the FULL tally, not the functions you touched.** An offset shift is
   silent in the function that caused it and loud in twenty that did not.
2. **Report matched-set membership as a set difference against the previous
   round: GAINED and LOST, both by name.** A net count cannot distinguish "eight
   closed" from "sixteen closed and eight broken", and those need opposite
   responses. If the arithmetic of your own report does not close, that gap IS
   the finding -- chase it before reporting the headline.

A missing function body is invisible to a per-function diff *by construction*:
there is nothing to diff. Only a whole-unit count catches it.

## A high tally score does NOT mean a unit is landable — the tally never links

`d_line_mng` measured 181/182, 98.7% by bytes, and the landing attempt still
broke all five binaries. Nothing was wrong with the measurement; the measurement
simply cannot see the class of defect that stopped it.

`tally.py` compiles one `.o` and compares disassembled text. It never runs the
linker. So it is blind to, at minimum:

- **Undefined symbols.** `dLineMng_c::smc_UNIT_SIZE_X` is declared
  `static const float` in the header and was never defined anywhere. Every
  function using it compiled and matched perfectly — because an unresolved
  external produces exactly the `lfs ...@sda21(r0)` retail has. The link is the
  first thing that ever objected.
- **Weak symbols the link places that retail resolves elsewhere.** Our object
  defines `__ct__7mVec2_cFv`; retail resolves it to `0x8007F800` in an un-landed
  unit. Ours got placed inside our claim and pushed `.text` out by 0x10. Same
  failure class as `d_a_wm_sandpillar`.
- **Data section ORDER within the unit.** Canonicalisation normalises the pool
  symbol name, so two different `.sdata2` layouts compare equal.

**So treat the tally as necessary and not sufficient.** Before claiming a unit is
ready, additionally check: every symbol it references resolves; no weak symbol it
defines is one retail takes from elsewhere; and its data-section object order
matches retail's addresses.

### The static-const-float trap, unresolved

`smc_UNIT_SIZE_X__10dLineMng_c` is at `.sdata2:0x8042CB18` in retail — the FIRST
object in `d_line_mng`'s `.sdata2`, with the compiler's literal pool following it
from `0x8042CB1C`. Reproducing that runs into two requirements that fight:

| definition placed | `.sdata2` order | codegen |
|---|---|---|
| before all uses | correct (symbol first) | **WRONG** — MWCC folds 16.0f and strength-reduces `pos.x / smc_UNIT_SIZE_X` into a multiply-by-reciprocal. `init` grows 0x118 -> 0x11c, `check_term` 0x124 -> 0x138 |
| after all uses | **WRONG** (symbol last) | correct — 181/182 restored |
| not defined at all | n/a | correct, but the link fails |

Retail has it first AND unfolded, so the original source achieves both and we do
not yet know how. Do not land this unit until that is settled. Note the header's
own comment already anticipated the reciprocal: the fold is the documented
failure mode, now measured.

**The `.text` overflow was exactly 0x20 and fully accounted for:** 0x10 from the
stray `mVec2_c` constructor, 0x4 from `init`, 0x14 from `check_term`, rounded to
alignment. When a claim overflows, do not guess — enumerate the placed functions
from `bin/wiimj2d.elf`'s symbol table and diff the size multiset against the
target's. Two oversized functions and one extra symbol fell straight out.
