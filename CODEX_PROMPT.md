# Work order for Codex — round 11

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 11.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is yours.

---

## STOP WORK ON THE TWO EGG DESTRUCTORS. They were never a problem.

This is the most important thing in this file and it invalidates part of rounds
8, 9 and 10. **The mistake was mine, not yours**, and you should know exactly
what it was because it changes what "done" means for this unit.

`wip/player_manager/TRIAL_LINK.md` reports `.text` as `0x90` over its claim and
attributes `0x80` of that to `__dt__Q23EGG8Vector2fFv` and
`__dt__Q23EGG8Vector3fFv`. That table is headed **"compiled object vs claim"** —
it measured the size of the `.o`, not the size of what survives the link. I then
wrote it up as though it were a link-time overflow, and you reasonably built
three rounds of work on top of it.

Here is the refutation, from the build that verifies all five binaries **right
now**. `bin/compiled/wiimj2d/dol/bases/d_multi_manager.o` — a landed,
byte-exact, banked unit — emits thirteen functions:

```
__ct__11dMultiMng_cFv, global
__dt__11dMultiMng_cFv, global
initStage__11dMultiMng_cFv, global
__dt__7mVec2_cFv, weak
__dt__Q23EGG8Vector2fFv, weak          <-- the "problem" symbol
setClapSE__11dMultiMng_cFv, global
setRest__11dMultiMng_cFii, global
addScore__11dMultiMng_cFii, global
incCoin__11dMultiMng_cFi, global
incEnemyDown__11dMultiMng_cFi, global
__dt__Q23EGG8Vector3fFv, weak          <-- the other "problem" symbol
setBattleCoin__11dMultiMng_cFii, global
setCollectionCoin__11dMultiMng_cFv, global
```

Its slice claims `.text 0xc8170-0xc8580`, exactly `0x410`, which is the ten real
functions. The object is `0xC0` bigger than that. **It links, and the DOL is
byte-identical to retail.**

So: **an unreferenced weak symbol emitted into an object does not have to fit
inside that object's slice `.text` claim.** The linker does not place it. Every
banked unit in this project that touches an `mVec2_c` or `mVec3_c` local has been
carrying these two symbols the whole time, invisibly and harmlessly.

Three consequences, and please carry all three:

1. **`d_a_player_manager.cpp` does not have a `0x90` `.text` overflow.** It has
   an object that is `0x90` bigger than its claim, `0x80` of which will not be
   placed. Whether the remaining `0x10` is real is now an open question that
   only a link can answer, and the link is mine to run.
2. **The eggVector.h change was never needed**, which retrospectively explains
   why removing it broke five binaries: it was removing something the whole
   project legitimately relies on.
3. **The unit's acceptance condition is unchanged and is the only thing that
   matters: all 65 functions byte-exact.** Section arithmetic is not the gate.

Your round-8 flag — "Offset-perturbing: YES, full build verification needed" —
was the correct call at the correct confidence, and it is the reason this cost
one build rather than a day. That judgement is what I want more of.

## Round 10 verdict: no matches, two results I am keeping

**`checkCorrectCreateInfo` is the model answer.** I gave you a hypothesis and you
came back with three measurements that refute it — `const int` folds to 99,
plain `int` gives the target's hoisted `@sda21` loads at 103, `volatile`
overshoots to 106 — and the conclusion that our draft was already right and my
suggestion would have broken it. **My hypothesis was backwards and you proved it
rather than accommodating it.** That is worth more to me than a match.

**`getFileP` coupling is confirmed.** At 347 instructions it emits out-of-line
as a real `bl`; at 345 it was still inlined. That is a genuine structural fact
about this unit and it was worth the round on its own.

`incCoin` reaching 130/130 is real progress even without byte-exactness.

---

## Round 11: find the ONE cause behind the register-allocation class

Stop treating the near-misses as independent problems. **Twenty-one near-misses
in one TU, of which at least seven are "same instructions, different register
numbers", is not what independent bugs look like.** In the four actor units that
landed byte-exact before this one, register allocation matched on first compile
almost everywhere. Something structural is different about `daPyMng_c`, and I
think it is one thing, not seven.

### The leading hypothesis, and why it is invisible to everything we have checked

`ASSEMBLY.md` describes `addRest` as "74/74 instructions, **uniform +1 register
shift** in the clamp section". `getYoshi` and `startMissBGM` are both described
as `r4` vs `r12`. A *uniform* register shift across a whole function is the
classic signature of an **argument-position difference** — the compiler thinking
a function has one more or one fewer incoming argument than the target did.

For a member function, the argument in position zero is `this`. And:

**CFront does not encode static-ness in the mangled name.** A static member
function and a non-static one with the same parameters mangle identically. So
`bin/dtk/wiimj2d_symbols.txt` **cannot** tell you which of `daPyMng_c`'s
functions were `static` in the original. This is the exact same blind spot as
return types, which has already cost this project six wrong declarations in this
very class.

`d_a_player_manager.hpp` declares the class as all-static. If even one function
was actually a non-static member — or the reverse — every argument in it shifts
by one register and the body is otherwise identical. That would produce exactly
the observed signature.

**Test it. Do not assume it.** Take the two or three cleanest "register
allocation only" cases — `addRest` and `getCoinAll` are the tightest — and
compile each both ways in a whole-`assembled.cpp` shadow copy. If flipping
static-ness closes one, you have found the cause and the remaining six are
probably a sweep.

### If that hypothesis dies, here are the other shapes of "one cause"

Report the negative and move down this list. Do not chase all of them; pick by
what the disassembly actually suggests.

- **A wrong member type in the header.** An `int` where the target has `u8`/`s8`
  changes load/store widths and can cascade into allocation. The class is
  all-static, so every one of these is a named symbol with a **size** in the
  symbol map — cross-check every static member's declared type against its
  recorded size. That is a mechanical check and it is cheap.
- **A wrong parameter type** that does not change the mangled name. `int` vs
  `long` mangles differently (`Fi` vs `Fl`) so that one is checkable, but
  `bool` vs `char`, or `const T&` vs `T&`, may not be.
- **Argument count or order** on a helper that many of the 21 call.

### Method, unchanged and non-negotiable

Compile the **whole of `assembled.cpp`** with your edit in it and diff the
function out of that object. Shadow-copy `assembled.cpp` into your own scratch
directory; `wip/` is read-only to you. Use `tools/auto_decomp/harness.py`'s
`compile_draft` / `extract` / `diff_fn`, extract **by address**, and assert
`instruction_count * 4` against the symbol map.

### What a good round 11 looks like

Either "the cause is X, here is one function that closes because of it, and here
are the others it should sweep", or "it is not X, here is the measurement that
kills it, and here is what the disassembly points at instead". **A well-killed
hypothesis is a complete result.** Do not come back with three more
individually-characterised near-misses; I already have twenty-one of those.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (read it freely, shadow-copy what you need, write
  nothing), `HANDOFF.md`, `AGENT_CONTEXT.md`, `CODEX_PROMPT.md`, or any
  `GEMINI_*.md`. Gemini is on `d_a_en_coin_main.cpp` and on a project-wide
  weak-symbol audit; stay out of both. `wip/nand_thread/` is my agents'.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
