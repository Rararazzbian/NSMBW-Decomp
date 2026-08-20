# Work order — round 16

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing for this repo and
it assumes nothing about you. This file is only round 16.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

Round 15 asked for `d_a_wm_ghost.cpp` and **no `QWEN_RESPONSE.md` was ever
written**. The unit was finished by another agent and is landed. Nothing is held
against that, but it is why this order re-states everything inline rather than
assuming continuity. Everything below the horizontal rule I verified myself
today, in this checkout.

---

## What the project is, in one paragraph

A **matching decompilation** of New Super Mario Bros. Wii. We write C++ that,
compiled with the original CodeWarrior compiler, produces **byte-identical**
machine code to the retail game. Not equivalent — the same bytes. The authority
is `python progress.py --verify-bin`, which MD5s five binaries; all five green or
the change did not happen. **You must never run it**, or `ninja`, `configure.py`,
or `land.py` — I am the only integrator, and two builds in this checkout clobber
each other.

## Your task: author `d_iggy_wan_kusari.cpp`

Iggy Koopa's swinging chain. This is a **DOL** unit (`wiimj2d.dol`), not a REL,
which changes several things — read the flags note below before compiling.

```
.text   VA 0x800B90A0 - 0x800BB0E0    (offset 0xB2920-0xB4960)   0x2040 = 8,256 bytes
__sinit_\d_iggy_wan_kusari_cpp = .text:0x800BA630, size 0x4D4
47 functions in range
```

**The bounds are a claim to check, not a given.** They come from
`wip/dol_scout/DOL_TARGETS.md`, which carves the whole surrounding region into 23
units and marks its own confidence per item. That document's `.data`/`.bss`
figures for a NEIGHBOURING unit were later found wrong by a factor of six, so
treat its edges as a starting hypothesis. Confirming or correcting them is part
of your round and is a genuinely useful result either way.

### Why this unit, and what makes it unusually tractable

**All 47 functions carry real mangled names. Zero anonymous.** I checked. That is
the opposite of the world-map REL units previous rounds worked on, where 100% of
functions were anonymous `fn_2_*` and no signature could be read at all. Here
**every parameter list is given to you by the mangling** — you only have to
determine return types.

There are **two classes** in the unit, which the earlier scout did not mention:

```
dIggyWanKusari_c        the chain manager
dIggyWanKusariPiece_c   an individual link
```

`dIggyWanKusari_c` has **six states**, named outright in the symbol map:

```
StateID_Ready   StateID_Normal   StateID_Tight
StateID_Release StateID_Collapse StateID_Dead
```

each a `.bss` object of size `0x30`, running `0x80358ED8` to `0x80359018`.

**Declaring the state framework correctly is the single highest-value thing you
can do in this round.** On other units on this project, getting the state
declarations right has emitted **eight, twenty, and most recently sixty-seven
functions for free** — all byte-exact, none hand-authored. The
`initializeState_*` / `finalizeState_*` / `executeState_*` triples for all six
states are already visible in the symbol map. Do the framework before you
hand-author a single body.

The only vtable belonging to this unit is
`__vt__31sFStateID_c<16dIggyWanKusari_c>` at `.data:0x80315DD0`. **There is no
`__vt__16dIggyWanKusari_c` and no `__vt__21dIggyWanKusariPiece_c`.** Establishing
what that means for the base class is part of your job — do not assume it means
"no base class".

### Free intelligence: the symbol map hands you function-local statics

The `@LOCAL@<function>@<name>` form names a static declared **inside a function
body**, which tells you the source shape outright. This unit has several:

```
@LOCAL@createMdl__21dIggyWanKusariPiece_cFR16mHeapAllocator_c@cs_mdl_name  .sdata  0x8
@LOCAL@ready__16dIggyWanKusari_cFv@cs_init_angle                          .sdata2 0x4
@LOCAL@setCollapseSpeed__21dIggyWanKusariPiece_cFi@cs_dir_prm             .sdata2 0x8
smc_ANGLE_DIST_RATE__16dIggyWanKusari_c                                   .sbss   0x2
smc_LENGTH__21dIggyWanKusariPiece_c                                       .sdata2 0x4 (float)
```

Read the actual bytes out of `original/wiimj2d.dol` for each. **Do not invent
constant values** — that has cost rounds here.

`bin/dtk/wiimj2d_symbols.txt` is the FULL DOL symbol map and is the richest
source you have. `syms.txt` is a small curated list; a symbol's absence from it
proves nothing.

---

## Six things that have each cost this project a round. Read them.

**1. DOL flags, not REL flags.** These are different programs and previous rounds
were silently miscompiled by using one for the other:

```
wiimj2d    -O4                                      (small data ON)
d_basesNP  -O4,p  -sdata 0  -sdata2 0  -char signed
```

You are on the DOL, so you want the `wiimj2d` set. **Call
`harness.compile_draft(src, obj)` from `tools/auto_decomp/harness.py`** and let it
supply them. Never hand-build the command line — it carries seven mandatory
include paths and people have lost rounds to one missing path.

**2. Return types are ABSENT from CFront mangling. Parameters are encoded; return
types are not.** This project has now found **twelve wrong declarations**, three
of them in the last few hours. The method that finds them, every time:

> **Read what the CALLER does with the return register immediately after the
> `bl` — does it READ r3, or CLOBBER it?** An observed clobber outranks any
> analogy with a sibling function.

Two specific varieties found today, both of which will occur in your unit:
- A function you declared `void` whose target ends every path with `li r3,0x1` /
  `li r3,0x0` and whose callers do `cmpwi r3,0x0` / `bne` — that is a `bool`.
- **An argument-count mismatch at the call site is a STORAGE-CLASS tell, not a
  return-type one.** If the call site sets ONE register where your declaration
  needs two, there is no implicit `this` — the function is `static`. A non-static
  declaration emits an extra `mr` pair the target does not have.

**3. Check the SIZE before you count differences.** A length mismatch is
**CONTENT** — missing or extra — in both directions. A pool-position or
register-allocation residual physically **cannot** change the instruction count,
because it only alters operands. Two functions on another unit sat for **six
rounds** logged as "pool-position residual, confirmed by line-by-line read" when
they were one word short and the missing word was a return value.

**Always report length before any differing count.** A differing count is
meaningless across different lengths, because one extra word offsets everything
after it and cascades.

**4. A `__cvt_*` conversion call in your output means a declared type is wrong.**
Usually an integer field that should be a `float`. This unit is angle- and
distance-heavy, so expect it.

**5. A `.data` block that looks like a constant table may be a SWITCH JUMP
TABLE.** Four blocks were flagged as hand-authored lookup data on another unit
today and were nothing of the kind — writing the right `switch`/`case` structure
reproduces them automatically. **Suspect a `switch` before you transcribe
constants**; transcribing is the expensive mistake.

**6. Know when to stop.** If a function reaches the correct instruction count and
differs **only in register numbers**, stop and report the count. That specific
wall has taken 100+ source variants across six functions here with zero
successes, and declaration order has been measured not to influence register
assignment at all. Reporting it is the correct answer, not a failure.

## A tooling caveat that will otherwise mislead you

`harness.canonicalise` **reports false mismatches.** Four functions on another
unit today were length-exact AND byte-identical to target, and it called them
differing — because the target's disassembly puts quote marks around a symbol
name where a standalone `.o` does not, and the quotes survive canonicalisation.
That undercounted the unit by four functions.

**If a function is length-exact and the comparator still says differ, compare the
raw instruction BYTES before believing it.** Byte equality is the actual
criterion. `wip/line_mng_shared/tally.py` implements the correct union gate
(bytes equal OR canonicalised equal) and is worth reading even though it is
hard-wired to a different unit's target.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- **Never edit a shared header, `slices/*.json`, or `syms.txt`.** Shadow-copy the
  header into your own include directory, prove your change there, and put the
  diff in your response as a proposal. I apply it and verify five binaries before
  it lands with anything else, so a failure is never ambiguous.
- Work only in `scratch/round16/`. Do not touch `wip/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `peer_archive/`, or `GEMINI_*.md` — **`wip/` in particular
  has four agents live in it right now.**
- **Name your draft file `d_iggy_wan_kusari.cpp`.** Anonymous-namespace symbols
  mangle the source filename into them, so a draft compiled under any other name
  diffs forever on those lines for reasons unrelated to your source.
- Extract by ADDRESS and assert `instruction_count * 4` against the symbol map
  before writing any C++.
- Mark anything unproven `@unofficial`. A `u8 pad[N]` for a region you cannot
  explain is a good answer; an invented member name is not.
- **Report a negative result rather than manufacturing a positive one.** On this
  project the peers' most valuable single act has repeatedly been refusing to
  comply with something I asserted that turned out to be wrong. Treat anything I
  claim above that you can measure as a hypothesis. Three separate agents
  corrected me today and each correction was the most valuable part of its round.

## Deliverable

`QWEN_RESPONSE.md`, containing:

1. **A per-function table** — name, target length, your length, differing count,
   and MATCH or not. Length column first. This is the headline result.
2. **How many functions the state-framework declarations alone emitted**, before
   you hand-authored anything. Report this separately; it is the number I most
   want.
3. The proposed classes and header in a fenced block, with offsets argued from
   evidence.
4. Your source in a fenced block.
5. Every variant you tried and its result, so nobody repeats it.
6. Whether the `.text` bounds above survived your check, and your corrected
   `.rodata`/`.data`/`.bss`/`.ctors` bounds with the evidence for each edge —
   which edges you PROVED and which you inferred, stated separately.
7. Anything you could not settle, said plainly, with what would settle it.

A table of twenty matches and twenty-seven characterised residuals is a better
round than forty-seven claimed matches I cannot reproduce. **I check every number
independently**, and I have twice this week found a peer's work was *better* than
my own measurement of it — once because I used the wrong compiler flags, once
because my verifier could not see dot-prefixed pool symbols.

Plain ASCII or clean UTF-8, LF, no BOM.
