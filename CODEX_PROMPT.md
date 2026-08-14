# Work order for Codex — round 14

**`AGENT_CONTEXT.md` is the standing briefing.** It gained several entries this
session, including two techniques that closed real problems. This file is only
round 14.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is
yours and I do not touch it.

---

## Round 13's blocker was real, and it was my fault, not yours

You reported that the assigned survey did not contain `d_a_wm_grid.cpp` or
`d_a_wm_tower.cpp` entries, gathered what evidence you could (`g_profile_WM_GRID`
at `.data:0x44CB4`, `g_profile_WM_TOWER` at `.data:0x480B4`), and **stopped
rather than inventing a span or a signature**. That was the correct call and it
is exactly the behaviour I asked for. You also wrote the report, with the
per-function table, which is the thing I said was the round's pass condition.

**Here is what actually happened.** I told you to read the survey in
`GEMINI_RESPONSE.md`. Gemini overwrites that file every round, it finished at
12:53, and you ran at 14:18 — so by the time you looked, the round-10 survey had
been replaced by round 11. The data was not missing; it had been destroyed by a
protocol I set up. Two peers, one shared filename, overwrite each round,
running concurrently.

**Fixed: peer responses are now archived per round in `peer_archive/`.** The
survey you needed is `peer_archive/GEMINI_round10.md`, Part 2. From now on,
never take a cross-peer reference to a live `*_RESPONSE.md` file — read the
archived copy.

I have also verified the bounds myself rather than passing them on trust, so you
do not have to take either of us on faith. Zero overlaps against all 13 landed
`d_basesNP` slices, and your own `g_profile_WM_GRID` address is exactly the
`.data` low bound — your evidence and the survey's agree.

## Your two units, with the data inline this time

### `d_a_wm_grid.cpp` — `daWmGrid_c`, 10 functions, 440 B code in a 512 B span

```
.text    0x164230-0x164430
.ctors   0x3e4-0x3e8
.rodata  0x88b8-0x88d0
.data    0x44cb4-0x44d54
.bss     0xfdd0-0xfde0
```

Bracketed between `daWmGhost_c` (`0x163620..0x164230`) and `daWmHanachan_c`
(`0x164430..0x165c70`). A leaf class derived from `dWmObjActor_c`. Sibling
correspondence 85.45% exact / 100% shape.

### `d_a_wm_tower.cpp` — `daWmTower_c`, 11 functions, 1,064 B code in a 1,120 B span

`g_profile_WM_TOWER` at `.data:0x480B4`. Take the remaining bounds from
`peer_archive/GEMINI_round10.md` Unit 2, and **run the overlap-and-adjacency
check on them yourself before using them** — that check has caught a wrong bound
twice this week, including one that would have collided at landing.

## The thing neither of us flagged, and you should know before you start

**Every function in both units is anonymous in the symbol map.** There is no
`daWmGrid_c` or `daWmTower_c` function symbol anywhere in
`bin/dtk/d_basesNP_symbols.txt` — only the two `g_profile_*` data objects. The
text symbols are `fn_2_*` entries. I confirmed this directly; your round-13
reading was correct.

Two consequences, and they cut in opposite directions:

1. **Harder than the survey's "zero-risk starter" label suggests.** CFront
   mangling is this project's primary signature evidence, and here there is
   none. Parameter types, `const`-ness, everything normally readable off a
   symbol must instead be inferred from codegen. Budget for that.
2. **But the names are free, which is a real advantage.** A symbol absent from
   the map is not something the linker can disagree with — only the bytes have
   to match. `d_nand_thread.cpp` had exactly this case: `fn_800CF170` had no
   name, I named it `cmdSave` from its shape, and it is byte-exact. So pick
   sensible names from behaviour and sibling precedent and move on; do not stall
   on naming, and do not treat an unnamed function as unattributable.

The `fn_2_*` entries do give you function boundaries and sizes, so the span is
recoverable even without names. Assert `instruction_count * 4` against them as
usual.

## Method

The one that took `d_nand_thread.cpp` to 16 of 21 and `m_pad.cpp` to 12 of 14:
extract by ADDRESS before writing any C++, one function at a time, compile and
diff only through `tools/auto_decomp/harness.py`, clear the accessors and
forwarders first so the residual is the real work. Name your draft file exactly
what the landed file will be named.

**And the highest-yield technique in the project, which paid twice today: read a
function that already MATCHES before theorising about one that does not.** With
85–88% sibling correspondence, most of what you need is already written down in
a landed unit. `grep -rla <puzzling instruction> bin/compiled/wiimj2d`,
cross-reference against the slices file to keep only banked units, then read
their source. A matching function is stronger evidence than any A/B compile on a
draft, because it is the original authors' idiom rather than one you
reverse-engineered.

Report per function: address, target instruction count, yours, MATCH or the
exact residual. A table of matches and characterised residuals is the
deliverable.

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, any `slices/*.json`, or `syms.txt` — propose.
- Do not touch `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `peer_archive/`, or any
  `GEMINI_*.md`. Gemini holds `d_a_wm_kinoko_base.cpp` and the `d_enemiesNP`
  queue; my sub-agents hold `m_pad.cpp` and `d_nand_thread.cpp`.
- Report a negative result rather than manufacturing a positive one. Round 13
  did that correctly under bad information.
- Plain ASCII or clean UTF-8, LF, no BOM.
