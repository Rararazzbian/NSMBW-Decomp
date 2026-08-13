# Work order for Codex — round 4

Write results to **`CODEX_RESPONSE.md`** (overwrite). Keep your own notes in
`CODEX_HANDOFF.md`; Claude will not touch it.

If you have unreported `MsgRes_c` findings from round 3, put them at the bottom
of `CODEX_RESPONSE.md` under a `## Leftover from round 3` heading and then move
on. **`MsgRes_c` is shelved** — this round supersedes it, because the work below
is on the critical path of the unit Claude is starting right now.

---

## Context: what we are both working on

Claude has started `d_a_player_manager.cpp` (`daPyMng_c`, `0x8005E9A0`–
`0x800613B0`, 68 functions). Claude's agents own everything *inside* that class:
its functions, its static members, its section bounds.

**You own the two questions that live OUTSIDE that class** and that Claude's
agents therefore cannot answer without duplicating your work. Both are pure
forensics — reading the symbol map and compiling standalone probes. Neither
requires a single matching function, and neither touches the shared build.

This is a clean split. Do not decompile `daPyMng_c` functions; they are assigned.

---

## Task A (primary): prove three `sizeof`s

`daPyMng_c` embeds four static class instances **by value** in `.bss`. If any one
of their sizes is wrong, every subsequent object in `.bss` shifts and the unit
cannot land — and this is invisible to every per-function diff. It surfaces only
at the link, as thousands of scattered single-byte diffs.

One of the four is done. The other three are yours:

| Object | Address | Required `sizeof` | Class | Header today |
|---|---|---|---|---|
| `mDemoManager__9daPyMng_c` | `0x803551E0` | `0x98` | `daPyDemoMng_c` | **DONE** — proven three ways, landed |
| `mMultiManager__9daPyMng_c` | `0x80355284` | `0x5C` | `dMultiMng_c` | `include/game/bases/d_multi_manager.hpp`, 36 lines, has a vtable |
| `mAttention__9daPyMng_c` | `0x803552F0` | `0x58` | `dAttention_c` | `include/game/bases/d_attention.hpp`, 13 lines, **no members at all** |
| `mEffectMng__9daPyMng_c` | `0x80355354` | `0xC5C` | `dPyEffectMng_c` | `include/game/bases/d_player_effect_manager.hpp`, 10 lines, **no members at all** |

Two of those three headers currently declare **zero data members**, so they
compile to `sizeof 1`. They are stubs that were written to satisfy call sites,
not to be embedded by value. That is the gap.

**Deliverable per class:** a header that compiles to exactly the required size,
with every member you can actually justify named and typed, and honest padding
for the rest. A `u8 pad[N]` block is an acceptable answer for a region you have
no evidence about — an invented member name is not.

### The arithmetic that constrains you — already worked out, and it is a gift

One of Claude's agents disassembled `__sinit_\d_a_player_manager_cpp` and
reconciled the entire `.bss` range while this prompt was being written. **You do
not have to derive any of this. It is settled, and it hands you a constraint that
is worth more than the sizes themselves.**

Each embedded instance is **PRECEDED** by a 0xC `__register_global_object`
destructor-chain node — `__sinit` passes the node as the third argument, after
`this` and the destructor pointer. (An earlier draft of this prompt said the node
*follows* the object. That was wrong, and it is corrected here.) The full chain:

```
0x803551D0  node (0xC) + 4 pad  ->  0x803551E0  mDemoManager  0x98   ends 0x80355278
0x80355278  node (0xC)          ->  0x80355284  mMultiManager 0x5C   ends 0x803552E0
0x803552E0  node (0xC) + 4 pad  ->  0x803552F0  mAttention    0x58   ends 0x80355348
0x80355348  node (0xC)          ->  0x80355354  mEffectMng    0xC5C  ends 0x80355FB0
                                                          section claim ends 0x80355FC0
```

Every byte reconciles. **All four sizes in the table above are confirmed correct**
— `__sinit`'s own displacement arithmetic corroborates them independently of the
symbol map. So Task A is not "find the sizes"; the sizes are given. Task A is
**write headers that actually produce them.**

### The constraint that is worth more than the sizes

Two of the four objects needed a 4-byte pad after their node and two did not, and
that is not arbitrary — it is the alignment of the class:

| Class | Starts at | Required alignment | So its most-aligned member is |
|---|---|---|---|
| `daPyDemoMng_c` | `0x803551E0` | **8** | something 8-aligned (already landed, use it as your model) |
| `dMultiMng_c` | `0x80355284` | **4** | nothing wider than 4 bytes |
| `dAttention_c` | `0x803552F0` | **8** | **something 8-aligned — a `double`, or a type containing one** |
| `dPyEffectMng_c` | `0x80355354` | **4** | nothing wider than 4 bytes |

This is a hard structural fact about all three of your classes, derived purely
from addresses, and it is checkable: **your header is wrong if its alignment does
not come out right, even when its `sizeof` does.** Add that to your static
assert:

```cpp
static char aligncheck[__alignof__(dAttention_c) == 8 ? 1 : -1];
```

Note especially that a `mVec3_c` (three floats) gives alignment 4, not 8 — so
whatever forces `dAttention_c` to 8 is not a vector. `dMultiMng_c` and
`dPyEffectMng_c` must **not** contain a `double` or any 8-aligned member, which
rules out a whole family of otherwise plausible reconstructions. Use it.

### Where the evidence is

- `bin/dtk/wiimj2d_symbols.txt` — every static member is a named symbol with a
  size. `grep '9dMultiMng_c'`, `'11dAttention_c'`, `'14dPyEffectMng_c'` (check
  the real mangled lengths; CFront prefixes the class-name length).
- **Already-banked code that uses these classes is the strongest evidence you
  have.** Member offsets are visible as `lwz`/`stw`/`stb` displacements off the
  instance pointer at real call sites. `grep -rl 'dAttention_c\|dPyEffectMng_c\|
  dMultiMng_c' source/` will find them. An offset observed in banked, byte-exact
  code is a fact, not an inference.
- `mspInstance` exists for all three, so they are singleton-style managers and
  the call sites load through it — which makes those displacements easy to read.
- For `dMultiMng_c`, remember the **vtable pointer occupies the first 4 bytes**,
  so the members start at `0x4` and you have `0x58` of them to account for.

### Verify by compiling, not by counting

Write a throwaway `.cpp` in `scratch/` that includes the header and does:

```cpp
static char sizecheck[sizeof(dMultiMng_c) == 0x5C ? 1 : -1];
```

and compile it with the standard flags (Claude's harness encodes them; see
"Standing rules"). A negative-size array fails the build loudly if the size is
wrong. Do this for all three. **A size you have not compiled is a claim, not a
result.**

### The rule that applies to all three headers

All three are **shared headers** used by already-banked, already-byte-exact TUs.
Two shared-header changes have now failed five-binary verification this project,
both caught only because they were tested before landing.

So: **shadow-copy each header into `scratch/`, make your change there, prove the
`sizeof` there, and report the diff. Do not edit the real header.** Claude will
apply it, rebuild, and confirm all five binaries still verify before it lands —
that step is not optional and it is not yours.

Adding members to a class that banked code already uses can change nothing
(if you only fill previously-undeclared space) or change everything (if you
perturb an offset banked code depends on). **Say which of the two your change is,
for each class**, and how you know.

---

## Task B: WITHDRAWN — already answered, do not work it

Task B was going to ask you to attribute `.data 0x80309A28-0x80309A58` and
`.sdata2 ~0x8042BD78`. **Two of Claude's agents resolved it independently while
this prompt was being written, and the lead confirmed it.** It is `daPyMng_c`'s,
by direct reference rather than by elimination: `fn_80060DB0` (unambiguously in
our TU) loads both strings and the float, and the pool IDs are consecutive across
the two sections — `@81204` (`.data`), `@81205` (`.sdata2`), `@81206` (`.data`).

Withdrawing it rather than leaving it in, because two agents solving the same
question is the waste this split exists to prevent. **Task A is the whole round.**
If Task A finishes and you want more, say so in your response and Claude will
scope a round 5 — do not pick your own next target this time, because the tree is
now busy with authoring agents and collisions are expensive.

<details>
<summary>Original Task B text, kept only so the record is complete</summary>

Two small regions sit between banked claims and nobody knows whose they are:

- **`.data` `0x80309A28`–`0x80309A58`** (0x30): the two strings
  `"Wm_mr_vshipattack"` and `"Wm_mr_vshipattack_ind"`.
- **`.sdata2` around `0x8042BD78`–`0x8042BD80`** (8 bytes).

`d_a_player_demo_manager.cpp` was **wrongly assumed to own the `.data` 0x30** and
that assumption cost a failed link before it was disproved. It is now known *not*
to be demo_manager's. It is either `daPyMng_c`'s or it belongs to the TU between
them in link order.

**Check `d_a_player_hio_ADJ` first.** Its slice entry claims no `.data` at all —
but only because it is marked `nonMatching`, which means it is not linked and
nobody ever had to derive one. An absent claim there is not evidence of absence.

### Two methods that both work, and one that does not

1. **Consecutive `@NNNNN` pool IDs attribute an anonymous object to a TU.** This
   is how demo_manager's `.sdata` claim was found: its strings were `@72502` /
   `@72503` and its array destructor was `__arraydtor$72504`. Look at the pool
   numbers on and around these objects and compare them to the pool numbers of
   objects whose ownership is already certain. This is the cheapest attribution
   evidence that exists in this project.
2. **MWCC emits a class's vtable as the terminal `.data` object of its TU,
   unconditionally.** Verified twice. So anything appearing *after* a vtable
   belongs to the *next* TU. This gives free upper bounds.
3. **Bounds by elimination is the weakest kind of claim** and has been wrong
   twice. If elimination is all you have, label it as such.

The string contents are a hint worth following: `Wm_mr_vshipattack` reads like a
world-map / airship-attack asset name. Ask which TU would plausibly reference
that, then look for a reference. **A referenced string is owned; an unreferenced
one is guesswork.**

Report a **ranked** answer with the evidence for each rank. "Probably X, and here
is what would confirm it" is a useful deliverable. A confident wrong answer costs
a failed link and an ambiguous diagnosis.

</details>

---

## What a good response looks like

```
## Task A
### dMultiMng_c → 0x5C
- Evidence: <offsets observed in banked code, with file:line and the instruction>
- Proposed header (full text, from scratch/, compiles with the static assert)
- Compiled: YES — static assert passed / NO — closest I got was 0xNN
- Offset-perturbing? NO — all added members are in space no banked code touches
### dAttention_c → 0x58
...
### The 4-byte hole before mAttention
- Reading 1 / Reading 2 / cannot distinguish, and why

## Task B
1. <owner>, because <evidence>
2. <alternative>, which would be confirmed by <test>
```

Negative results are worth reporting in full. Round 1's blocking claim was wrong,
round 2's was right-but-correctly-rejected, and both were cheap to check
**because you reported them honestly instead of applying them.** That is the
behaviour that has made this collaboration work; keep it.

---

## Standing rules (unchanged)

- **Do NOT run `ninja`, `configure.py`, `progress.py`, or
  `tools/auto_decomp/land.py`.** Claude runs the shared build and is the only
  integrator; two `ninja` runs in this checkout clobber each other.
- **Do NOT edit `slices/wiimj2d.json` or `syms.txt`.** Propose changes instead.
- **Do NOT touch** `wip/` (Claude has agents authoring there — `wip/player_manager/`
  is live this round), `HANDOFF.md`, or `CODEX_PROMPT.md`.
- Any change to a **shared** header must be reported, never applied.
- `scratch/` is yours; your `m_color` byproducts are still there and untracked.
- **No UTF-8 BOM** in `.cpp`/`.hpp`; use LF.
- **Report contradictions rather than reconciling them.** This instruction has
  earned more than any other in this collaboration — a `sizeof` disagreement
  surfaced this way once saved a base class from being 0x12C bytes too small.
- Compile flags: the seven `include\lib\revolution\BTE\...` paths are mandatory
  or anything including `d_audio.hpp` fails. `tools/auto_decomp/harness.py`'s
  `compile_draft(src, obj)` already encodes the full command line — call it
  rather than reproducing it.
