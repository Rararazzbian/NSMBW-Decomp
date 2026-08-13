# Work order for Gemini — round 1

Welcome. Write your results to **`GEMINI_RESPONSE.md`** (overwrite it each
round). If you want a scratch notebook that persists between rounds, use
`GEMINI_HANDOFF.md` — it is yours, and nobody else will touch it.

---

## Who is working on what, so you never collide

This repository is a **matching decompilation** of *New Super Mario Bros. Wii*.
"Matching" means the C++ we write must compile, with the original 2009
CodeWarrior compiler, to **byte-identical machine code**. The authority is a
single command that MD5s five binaries, and it is either all green or the change
is not done. Close is not a partial win; it is a fail.

Three workers are active right now:

| Worker | Owns | Do not touch |
|---|---|---|
| **Claude** (lead + 8 sub-agents) | `d_a_player_manager.cpp` — 65 functions, being authored and assembled right now. Runs the shared build. Is the only one who edits `slices/wiimj2d.json`, `syms.txt` and any shared header. | `wip/`, `HANDOFF.md`, `CODEX_*.md`, `GEMINI_PROMPT.md` |
| **Codex** (a peer AI) | Reconstructing `dPyEffect_c`'s internals in `d_player_effect_manager.hpp` | `CODEX_HANDOFF.md` is Codex's |
| **You** | The five header gaps below. Nobody else is looking at them. | Everything above |

Your work is **header reconstruction from disassembly**. It does not require you
to match a single function, and it touches no file anyone else is in.

---

## Why this task exists, and why it is worth doing well

An agent authoring `daPyMng_c::update()` — a 0x2B8-byte per-frame dispatcher —
got it structurally right but could not finish, because **six things it calls do
not exist anywhere in the tree**. It reported them rather than inventing them,
which was correct. Your job is to make them exist, with evidence.

`update()` is blocked on you. That is the whole reason this is round 1 rather
than something more exploratory.

---

## Your five targets

### 1. `PauseManager_c` — a class that does not exist at all

`update()` ends with, in effect:

```cpp
if (mPauseDisable == 0) {
    PauseManager_c::m_instance->setPauseEnable(true);
} else {
    PauseManager_c::m_instance->setPauseEnable(false);
}
```

There is **no `PauseManager_c` anywhere in `include/` or `source/`**. Find its
real name and shape. Start from `bin/dtk/wiimj2d_symbols.txt` and search for
mangled names containing `Pause`. The mangling is CFront-style: a method is
`name__<len><ClassName><F><params>`, so `setPauseEnable__14PauseManager_cFb`
would be a class whose name is 14 characters long taking one `bool`. **The
length prefix is the reliable part** — use it to recover the exact class name,
because the name in the draft above is a guess by an agent that could not find
it.

Deliver: a header declaring the class, the singleton pointer, and at minimum the
method `update()` calls. Note there is already a `d_pause_manager.hpp`-shaped
gap in the naming convention — check whether such a file exists before creating
a new one.

### 2. `dScStage_c::getGameDisplay()`

Called by `update()` to get a `dGameDisplay_c *`. Not declared in
`include/game/bases/d_s_stage.hpp`. Determine whether it is a static accessor
over a singleton (the common shape in this codebase) or a member, and whether it
is emitted out of line or inlined.

**Read `d_s_stage.hpp` first** — it already contains two worked examples of
exactly this pattern, `getInstance()` and `getExitMode()`, and one of them
(`getExitMode`) carries a `NOINLINE` marker because the original calls it out of
line rather than inlining it. Which of those two shapes applies here is a real
question with an answer in the disassembly.

### 3. Four `dGameDisplay_c` methods

`update()` calls, in this order:

```cpp
disp->setPlayNum(<int[4]>);   // takes a 4-int buffer copied from mRest
disp->setCoinNum(<int>);
disp->setScore(<int>);
disp->setCollect();
```

Find their real mangled names and signatures. Note the first one is passed a
**local copy** of a four-element array, not the array itself — that detail is
already established from the target and you should preserve rather than
re-litigate it, but the parameter type (pointer? reference to array?) is yours
to determine.

### 4. `dStageTimer_c`, field at offset `0xC`

`update()` does the equivalent of:

```cpp
if (mStopTimerInfo != mStopTimerInfoOld) {
    dStageTimer_c::m_instance->mStopped = (mStopTimerInfo != 0);
    mStopTimerInfoOld = mStopTimerInfo;
}
```

A field at `+0xC` written with a byte store. Confirm the offset, the width, and
whether the existing header has padding covering it that should be split.

### 5. Three missing bits in `dQuake_c::FLAGS_e`

`update()` tests `dQuake_c::m_instance->mFlags` against **`0x38`**, then against
**`0x20`**, then **`0x08`**. Find `dQuake_c`'s existing `FLAGS_e` enum and
establish what those three bits are. `0x38 == 0x20 | 0x10 | 0x08`, so the first
test is a mask of three flags and the later tests distinguish two of them — that
structure should help you name them.

---

## The rules, and why each one exists

These are not bureaucracy. Every one of them was written after something went
wrong.

1. **Never run `ninja`, `configure.py`, `progress.py`, or
   `tools/auto_decomp/land.py`.** Claude runs the shared build and is the only
   integrator. Two builds in this checkout clobber each other's object files.

2. **Never edit a shared header — propose it.** Write your proposed version into
   `scratch/` and put the diff in your response. Two shared-header changes have
   failed five-binary verification on this project, and both were caught only
   because they were tested before landing. Claude applies, rebuilds, and
   verifies. An addition that "obviously cannot break anything" still gets
   verified, because one of the two failures looked exactly like that.

3. **Never edit `slices/wiimj2d.json` or `syms.txt`.** Propose instead.

4. **Report contradictions rather than reconciling them.** If the symbol map,
   the disassembly and an existing header disagree, say all three and stop. This
   instruction has earned more than any other here: a `sizeof` disagreement
   surfaced this way once saved a base class from being 0x12C bytes too small —
   an error invisible to every per-function diff and detectable only at the link.

5. **Report a negative result rather than manufacturing a positive one.** Last
   round, Codex was told by me that a 4-byte gap proved a class contained a
   `double`. It searched, found no `lfd`/`stfd` anywhere, and reported "cannot
   distinguish" instead of inventing the member I had implied. **I was wrong** —
   the real rule turned out to be that MWCC aligns a `.bss` object to 8 when its
   *size* is a multiple of 8, nothing to do with members. Had it deferred to me,
   we would have shipped a fabricated field. Treat anything I assert that you
   can measure as a hypothesis, not a fact.

6. **A guess must be labelled as a guess.** `u8 pad[N]` for a region you cannot
   explain is a good answer. An invented member name is not. Mark anything
   unofficial with `@unofficial` in a doc comment, as the existing headers do.

7. **CFront mangling omits return types.** You can never confirm a return type
   from a symbol name — only from the body's register use. Three return types in
   this one class were wrong for exactly this reason. If you propose one, say
   how you know.

8. Environment: this is Windows. `dtk` fails on relative paths with forward
   slashes — use an absolute path to `bin/dtk-windows-x86_64.exe`. PowerShell
   5.1 parses 8-hex-digit literals as negative `Int32`, so do address arithmetic
   in Python. Files must be **LF, no UTF-8 BOM**. Write `GEMINI_RESPONSE.md` as
   plain ASCII or clean UTF-8 — the previous peer's responses arrived with
   mangled dashes and `?` substitutions, which makes them harder to act on.

---

## Where the evidence lives

| Source | What it gives you |
|---|---|
| `bin/dtk/wiimj2d_symbols.txt` | every symbol: mangled name, section, address, size. Your primary tool. |
| `bin/dtk/dtk_splits_wiimj2d.txt` | official per-source-file section ranges for already-split TUs. Hard bracketing — better than any inference. |
| `bin/dtkspl/obj/auto_*_*.o` | the original binary, split into objects by address. Disassemble with `bin\dtk-windows-x86_64.exe elf disasm <obj> <out.txt>`. |
| `source/dol/bases/*.cpp` | already-matching code. **An offset observed in banked, byte-exact code is a fact, not an inference** — it is the strongest evidence available to you. |
| `tools/auto_decomp/harness.py` | `compile_draft(src, obj)` encodes the exact compiler flags. Call it rather than reproducing the command line; the seven `BTE` include paths are mandatory or anything including `d_audio.hpp` fails. |

A useful trick from this week: **consecutive `@NNNNN` pool IDs attribute an
anonymous object to a translation unit.** It is the cheapest attribution
evidence in the project and it settled an ownership question that elimination
had got wrong.

---

## What a good response looks like

```
## 1. PauseManager_c
- Real name: <name>, recovered from <mangled symbol>, length prefix <n>
- Proposed header (full text, from scratch/)
- Confidence: high/medium/low, and what would raise it
- Offset-perturbing for existing TUs? YES/NO, and how I know

## 2. dScStage_c::getGameDisplay()
...
```

For each item: the evidence, the proposal, and an explicit statement of whether
applying it could change code in already-matching TUs. If you could not settle
one, say so plainly and say what would settle it — that is a useful deliverable
and it is much cheaper than a confident wrong answer.

If you finish early, say so in your response and ask for more rather than
picking your own next target; the tree is busy and collisions are expensive.
