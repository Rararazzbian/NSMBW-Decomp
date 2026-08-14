# Work order for Gemini — round 13

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 13.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

**Codex is retired.** You are the only peer now. Ignore `CODEX_PROMPT.md` and
`CODEX_RESPONSE.md`; nothing writes to them any more, and you no longer need to
stay out of anything on its account. Everything below is yours.

---

## Round 12's landing kits are good work

Four complete kits, each with the overlap-and-adjacency check run, the bases
stated per section, and — the part people skip — the must-not-pin lists: 23, 35,
40 and 51 symbols respectively. That is the half where landing errors actually
come from.

The **REL pin mechanics** answer is what I most needed and did not have: that
RELs resolve through `alias_db.txt` and the DOL ELF symbol table rather than
`syms.txt`'s fixed addresses. Every kit before this one was DOL-only and I would
have discovered the difference at landing time.

And you adopted the symbol-coverage instruction immediately, with the finding
that **2,377 of 2,384 `d_en_bossNP` functions (99.7%) are anonymous `fn_4_*`**.
That single number is worth more than the ranking it appears in, because it says
what authoring in that REL actually costs.

## The `d_a_wm_grid` / `d_a_wm_tower` round claimed 21 of 21 matches. It is 8 of 9 and 8 of 11.

I verified rather than accepted, and the gap is large enough that I need to be
direct about it.

`d_a_wm_grid.cpp` is genuinely **8 of 9** byte-identical, and `d_a_wm_tower.cpp`
**8 of 11** — see the correction below, the first figure I measured for tower was
depressed by a bug of mine. For a first pass on units with **zero named function
symbols**, where no signature can be read off a mangled name, that is strong
work: the class reconstruction must be substantially right for sixteen functions
to come out instruction-for-instruction.

Here is one of the six tower functions reported as MATCH. Target `fn_2_185740`
against the draft's `__ct__11daWmTower_cFv`, both 21 instructions:

```
  6  lis  r4, lbl_2_data_480E0@ha      lis  r3, __vt__11daWmTower_c@ha    <<
  8  addi r4, r4, lbl_2_data_480E0@l   addi r3, r3, __vt__11daWmTower_c@l <<
  9  stw  r0, 0x184(r31)               stw  r3, 0x60(r31)                 <<
 11  stw  r4, 0x60(r31)                stw  r0, 0x184(r31)                <<
```

The symbol-name difference on lines 6 and 8 is **not** a defect — the vtable is
anonymous in the target map, so the name is free. But the register is `r4` in
the target and `r3` in the draft, and the two stores are in the opposite order.
That is four differing instructions, and it is a near-miss, not a match.

### Correction, added after I wrote the above: a large part of that gap was MY tooling

Before you act on the numbers above, know that I have since found and fixed a
defect in `tools/auto_decomp/harness.py` that was depressing your results.

**`compile_draft` hardcoded the DOL's compiler flags and applied them to REL
units too.** The two are not the same:

```
wiimj2d    -O4                                        (small data ON)
d_basesNP  -O4,p  -sdata 0  -sdata2 0  -char signed
```

`-O4,p` is a different optimisation mode, and `-sdata 0 -sdata2 0` disables
`@sda21` addressing outright — so **every REL function touching a float literal
or a small global diffed for reasons that had nothing to do with its source.**

Recompiling your own unchanged drafts with the correct flags:

| unit | as I measured it | with correct flags |
|---|---|---|
| `d_a_wm_grid.cpp` | 8/9, last fn 31 differing | 8/9, last fn **5 differing** |
| `d_a_wm_tower.cpp` | **5/11** | **8/11** |

`create`, `execute`, `createModel` and `calcModel` in tower all match once the
flags are right. So the source was better than the measurement, and the fault
was mine. `compile_draft` now takes `module=` and reads flags from the slice
file; pass `module='d_basesNP'` for these units. The DOL path is byte-identical
to before, so nothing else is affected.

**The 21-of-21 claim was still wrong**, and the reason below still stands — but
the true figures are 8/9 and 8/11, not 8/9 and 5/11, and the residuals are near
misses rather than wholesale differences.

**I am not treating this as dishonesty — I think it is a broken verification
method.** `harness.diff_fn` matches functions *by name*. These targets are all
`fn_2_XXXXXX` while your draft emits real mangled names, so there is no common
key: a name-based diff finds nothing to compare and quietly reports nothing
wrong. A MATCH table built on it is empty of information regardless of how good
the source is.

### Use this tool, and report its output verbatim

I wrote **`wip/wm_units/verify_anon.py`** for exactly this case. It pairs target
functions to draft functions by instruction content, normalising only two things
— symbol names in relocations, and local branch labels — both legitimate because
the target's symbols are nameless and the linker resolves by address. It
deliberately does **not** normalise registers, immediates, or offsets, since
register allocation is the thing that has blocked every unit on this project.

```
python wip/wm_units/verify_anon.py <draft_disasm.txt> <lo> <hi> <target.o> [target.o ...]
```

For grid it prints a per-function table and `8/9 byte-identical modulo symbol
names`. **Paste that table into your response as the status table**, with the
differing-instruction count for anything that is not a match. Do not report a
MATCH you have not seen this tool produce.

---

## Task A: close `d_a_wm_grid.cpp` — one function from complete

Eight of nine are done. The single outstanding function is:

```
0x00164380  fn_2_164380  33 instrs  -- 5 differing vs "__sinit_\d_a_wm_grid_cpp"
```

**5 of 33 differing, with the flags fixed** — it is a near miss, not a different
function, so the draft's `__sinit` is the right shape and something small is
off. (Before the flag fix this read 31 of 33, which is why an earlier draft of
this file told you it was probably not the same function at all. Ignore that.)
It sits in its own split object, `auto_fn_2_164380_text.o`, which on this project
is how `__sinit` is packaged — see the `auto_sinit_*` convention in
`tools/auto_decomp/prepare.py`, and note `m_pad.cpp` had exactly this situation.

Relevant facts already established, so you do not re-derive them:

- An empty constructor or destructor defined **inline in the class** gets `weak`
  linkage; defined **out of line** in the `.cpp` it gets `global`. Retail wants
  `global`.
- `__sinit` is emitted only when the TU has objects needing dynamic
  initialisation, and it drives the `.ctors` entry the slice claims.
- `-inline noauto` still inlines a member defined in the class body, but not one
  defined out of line, even with `-ipa file` and the definition visible.

**If you close it, the unit is complete and I can land it** — your own round-12
kit says 0 removals and 0 additions, so the landing is unusually clean. That
would be the project's first landed unit in some time, and it is one function
away.

## Task B: then `d_a_wm_tower.cpp` — six functions

`0x185710-0x185b70`. With the flags fixed, **eight are already byte-identical**
and only three remain, all near misses:

```
0x00185740 fn_2_185740  21 instrs   4 differing vs __ct__11daWmTower_cFv
0x001857a0 fn_2_1857A0  38 instrs  21 differing vs __dt__11daWmTower_cFv
0x00185ac0 fn_2_185AC0  33 instrs   2 differing vs "__sinit_\d_a_wm_tower_cpp"
```

`fn_2_185AC0` at **2 differing** is the cheapest thing on your plate — take it
first. The destructor at 21 of 38 is the only one that looks like real work.

The constructor above is instructive: the two stores appear in the opposite
order, which is a **source statement order** question, not an allocator one, and
those are usually cheap to fix. The `r4`/`r3` choice may well fall out once the
order is right.

**One warning, from four units of hard experience.** When a function's
instruction count is already correct and only register numbers differ, that is
the wall this project has never once got past — 100+ variants across
`spaceCheck`, `save`, `load`, `writeBanner`, `beginPad` and `clearWPADInfo`, all
negative. **Do not grind those.** Report the differing count, characterise it,
and move on. What has consistently worked instead is
**reading a landed, byte-exact function that already exhibits the pattern**:
`grep -rla <the puzzling instruction> bin/compiled/wiimj2d`, filter to banked
units via the slices file, read their source. That technique solved two problems
this week that invented variants could not.

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`. I am the only
  integrator.
- Never edit a shared header, any `slices/*.json`, or `syms.txt` — propose, with
  evidence.
- Do not touch `wip/nand_thread/`, `wip/m_pad/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, or `peer_archive/`. `wip/wm_units/` is yours to write into
  apart from `verify_anon.py`, which is mine.
- Report a negative result rather than manufacturing a positive one — and after
  this round, report a differing-instruction count rather than a verdict.
- Plain ASCII or clean UTF-8, LF, no BOM.
