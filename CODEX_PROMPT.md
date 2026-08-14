# Work order — round 15

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing for this repo and
it assumes nothing about you. This file is only round 15.

Write results to **`CODEX_RESPONSE.md`** (overwrite it).

You are new to this project, so this order carries its own facts inline rather
than pointing at other people's files. Everything below the horizontal rule I
have verified myself today.

---

## What the project is, in one paragraph

A **matching decompilation** of New Super Mario Bros. Wii. We write C++ that,
compiled with the original CodeWarrior compiler, produces **byte-identical**
machine code to the retail game. Not equivalent — the same bytes. The authority
is `python progress.py --verify-bin`, which MD5s five binaries; all five green
or the change did not happen. **You must never run it**, or `ninja`,
`configure.py`, or `land.py` — I am the only integrator, and two builds in this
checkout clobber each other.

## Your task: author `d_a_wm_ghost.cpp`

A world-map actor in `d_basesNP.rel`. **13 functions, 3,024 B of code in a
3,088 B span.** Nobody has started it and nothing else in flight touches it.

Bounds, which I have run the overlap check on against all 13 landed
`d_basesNP` slices — zero overlaps, and every section is exactly adjacent to
`d_a_wm_dokan_route.cpp` below it:

```
.text    0x163620-0x164230     (3,088 B span, 3,024 B code, 13 functions)
.ctors   0x3e0-0x3e4
.rodata  0x8880-0x88b8
.data    0x44a9c-0x44cb4
.bss     0xfdc0-0xfdd0
```

Class: `daWmGhost_c`, profile `g_profile_WM_GHOST`, a leaf deriving from
`dWmObjActor_c`. Sibling correspondence against already-landed code is about
**40% exact / 55% shape** — lower than the units next to it, so expect real work
rather than transcription.

Target objects live under `bin/dtkspl/d_basesNP/obj/`. Find the ones covering
the range and disassemble them with `harness.disasm`.

---

## Five things that have each cost this project a round. Read them.

**1. The REL flags are not the DOL flags.** `tools/auto_decomp/harness.py`
hardcoded the DOL's for a long time and silently compiled every REL unit as a
different program:

```
wiimj2d    -O4                                       (small data ON)
d_basesNP  -O4,p  -sdata 0  -sdata2 0  -char signed
```

`-O4,p` is a different optimisation mode and `-sdata 0 -sdata2 0` disables
`@sda21` addressing outright. **Call `harness.compile_draft(src, obj,
module='d_basesNP')`.** Never build a command line by hand — the flags include
seven mandatory include paths and people have lost rounds to a missing one.

**2. Every function in this unit is anonymous.** All 13 are `fn_2_*` in
`bin/dtk/d_basesNP_symbols.txt`; there is not one mangled name. Two consequences
that cut opposite ways:

- There is **no signature evidence at all**. CFront mangling is normally how we
  read parameter types, and here there is none. Everything comes from codegen.
- But **the names are free**. A symbol absent from the map is not something the
  linker can disagree with; only the bytes must match. Pick sensible names from
  behaviour and move on. Do not stall on naming, and do not treat an unnamed
  function as unattributable.

**3. `harness.diff_fn` matches functions BY NAME and will silently lie to you
here.** Your draft emits real mangled names, the target has `fn_2_*`, so there
is no common key: it finds nothing to compare and reports nothing wrong. A
previous round reported "21 of 21 matches" that way. Use only:

```
python wip/wm_units/verify_anon.py <draft_disasm.txt> 0x163620 0x164230 <target.o> [target.o ...]
```

It pairs functions by instruction content and normalises **only** relocation
symbol names and local branch labels — legitimate, because the target's symbols
are nameless and the linker resolves by address. It does not normalise
registers, immediates or offsets. **Paste its table into your response verbatim
and never claim a MATCH you have not seen it print.**

**4. Read a function that already MATCHES before theorising about one that does
not.** This is the highest-yield technique here and it has repeatedly beaten
invented source variants. `source/d_basesNP/bases/` contains landed, byte-exact
siblings — `d_a_wm_cannon.cpp`, `d_a_wm_cloud.cpp`, `d_a_wm_dokan_route.cpp`,
`d_a_wm_peach_castle.cpp` and more. These leaf actors are highly stereotyped:
`classInit`, constructor, destructor, `create`, `execute`, `draw`, `doDelete` are
near-boilerplate across the family. Read the closest sibling and follow its idiom.
`grep -rla <puzzling instruction> bin/compiled/wiimj2d` finds more.

**5. Know when to stop.** If a function reaches the correct instruction count and
differs **only in register numbers**, stop and report the count. That specific
wall has taken 100+ source variants across six functions on this project with
zero successes, and declaration order has been measured not to influence
register assignment at all. It is not a failure to report it; it is the correct
answer.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- **Never edit a shared header, `slices/*.json`, or `syms.txt`.** Shadow-copy the
  header into your own include directory, prove your change there, and put the
  diff in your response as a proposal. I apply it and verify five binaries
  before it lands with anything else, so a failure is never ambiguous.
- Work only in `scratch/round15/`. Do not touch `wip/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `peer_archive/`, or `GEMINI_*.md` — other work is live in
  those.
- **Name your draft file `d_a_wm_ghost.cpp`.** Anonymous-namespace symbols mangle
  with the source filename inside them, so a draft compiled under any other name
  diffs forever on those lines for reasons unrelated to your source.
- Extract by ADDRESS and assert `instruction_count * 4` against the symbol map
  before writing any C++.
- Mark anything unproven `@unofficial`. A `u8 pad[N]` for a region you cannot
  explain is a good answer; an invented member name is not.
- **Report a negative result rather than manufacturing a positive one.** On this
  project the peers' most valuable single act has repeatedly been refusing to
  comply with something I asserted that turned out to be wrong. Treat anything I
  claim here that you can measure as a hypothesis.

## Deliverable

`CODEX_RESPONSE.md`, containing:

1. **The verifier's table, verbatim** — every function with MATCH or its
   differing-instruction count. This is the headline result.
2. The proposed class and header in a fenced block, with offsets argued from
   evidence.
3. Your source in a fenced block.
4. Every variant you tried and its result, so nobody repeats it.
5. Anything you could not settle, said plainly, with what would settle it.

A table of eight matches and five characterised residuals is a better round than
thirteen claimed matches I cannot reproduce. I check every number independently,
and I have twice this week found that a peer's work was **better** than my own
measurement of it — once because I used the wrong compiler flags, once because
my verifier could not see dot-prefixed pool symbols. So report what you measure,
and if you think one of my facts above is wrong, say so.

Plain ASCII or clean UTF-8, LF, no BOM.
