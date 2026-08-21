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
- **Declaration order does NOT drive MWCC's saved-register assignment.** Tested
  exhaustively: all six orderings of three hoisted base pointers in `beginPad`
  produced **byte-identical** output. So when a residual is "the right
  instructions in the wrong registers", reordering declarations cannot fix it,
  and roughly ten variants across two agents were spent on that assumption
  before it was measured. The allocation is driven by something not exposed at
  the C++ level -- live-range ordering or an internal numbering. **Treat a
  pure register-permutation residual as not source-addressable** and spend the
  effort on unit selection instead.
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

### Levers that are PROVEN NOT to work -- do not spend a round on these

- **Commutative float operand order in the source.** `a * b` and `b * a` (and
  the `+` equivalents) compile to BYTE-IDENTICAL code; MWCC canonicalises
  commutative FP operands before register allocation. Four spellings were
  tested side by side on one function. If an `f0`/`f1` permutation is your
  residual, source operand order will not move it.
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
