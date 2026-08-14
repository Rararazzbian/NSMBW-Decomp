# Work order for Codex — round 12

**`AGENT_CONTEXT.md` is the standing briefing.** It gained four entries since
round 11; read them, two are about mistakes made in round 11. This file is only
round 12.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is
yours and I do not touch it.

---

## Round 11 produced no response file, and the artifacts cannot be read

`CODEX_RESPONSE.md` is still round 10's, from yesterday. What arrived for round
11 was `scratch/codex_round11/`, containing `assembled_static.cpp`,
`assembled_nonstatic.cpp`, `assembled_decl_nonstatic.cpp`, two `.o`s, two
disassemblies, and one shadow header. No report.

I measured the artifacts rather than assume. **All three `.cpp` files are
byte-identical** (same MD5), so the variable was in the shadow header, which is
the right way to run that experiment. But the two disassemblies differ in
exactly 70 lines, and **every one of them is a filename difference**:

```
< lwz r0, "scCoinMax__30@unnamed@assembled_static_cpp@"@sda21(r0)
> lwz r0, "scCoinMax__33@unnamed@assembled_nonstatic_cpp@"@sda21(r0)
```

Codegen difference: **zero**. And only one shadow header survives, so I cannot
tell which of two opposite conclusions is true — that the header change was
never actually applied between the two compiles, or that it was applied and is
codegen-neutral. The round is unusable, and that is a reporting failure rather
than a research failure.

**The decisive test, for next time.** A member function that is genuinely
`static` has no `this` parameter, so its arguments start in `r3` instead of
`r4`, and every call site in the TU changes with it. If you flip static-ness on
a function this TU defines or calls and see *no* register movement, your
variable did not vary — stop and fix the harness before drawing a conclusion.
The one case where zero change is honest is a declaration-only edit to a
function this TU never calls, and then the correct report is "this TU cannot
answer the question", not silence.

**Also: your draft's filename is part of the object code.** Anonymous-namespace
symbols mangle as `name__NN@unnamed@<filename>_cpp@` with `NN` the length of
that string. Compiling as `assembled_static.cpp` guarantees those lines diff
against the target forever, no matter how correct the source is. **Name the
draft `d_a_player_manager.cpp` from the first compile.** This is now in
`AGENT_CONTEXT.md`.

## Stop working the three near-misses

`createCourseInit`, `incCoin` and `checkCorrectCreateInfo` are parked. Not
because the work was bad — the `checkCorrectCreateInfo` constant measurement was
a genuine result and it is recorded — but because four rounds on register-level
near-misses have produced no byte-exact function, and a fifth is not the way to
change that. Your `incCoin` result (right instruction count, wrong register
allocation) is the same wall three other agents hit on a different unit this
week. It is a real wall. Park it and take ground elsewhere.

Two things from your round-10 report are worth keeping and I have kept them: the
`getFileP` inlining threshold is confirmed directional, and the constants must
stay plain non-`const`.

---

## Task: author the untouched bulk of `d_a_player_manager.cpp`

`0x8005E9A0`–`0x800613B0`, 10,768 B span, 10,300 B of code, **68 functions**.
This is the highest-value unit left in the DOL — worth more progress than the
next two queued units combined — and it is now fully unblocked:

- `sizeof(daPyDemoMng_c) == 0x98` is proven three independent ways and landed.
- **The `0x90` `.text` overflow was a phantom and is now formally settled.**
  Gemini audited all 143 landed units this round: 64% of them compile to objects
  larger than their slice claims, because unreferenced weak symbols are
  deadstripped and duplicate weak definitions are deduplicated to their home TU.
  For this unit specifically, `0x80` deadstrips, `0x64` deduplicates, and
  `getCourseIn__10dScStage_cFv` (`0x8`) is the surviving definition and belongs
  in the slice. **Net overflow: zero.** Do not spend a line on it again.

`include/game/bases/d_a_player_manager.hpp` already exists with real signatures,
because the banked `d_a_player.cpp` and `d_a_player_base.cpp` call into it.

**Your job is the functions nobody has attempted yet** — not the three parked
ones. Work the method that took `d_nand_thread.cpp` from a stub header to 14 of
19 functions byte-exact in a single session:

1. Start from the target disassembly, not from a guess about what the function
   does. Extract by ADDRESS and assert `instruction_count * 4` against
   `bin/dtk/wiimj2d_symbols.txt` before you write any C++.
2. One function at a time. Do not move on from a function until it either
   matches byte-for-byte or you can characterise the residual precisely.
3. Compile and diff only through `tools/auto_decomp/harness.py`.
4. Batch the small ones. Roughly a third of a unit this size is accessors and
   forwarders that match on the first or second attempt; clear those first so
   the residual is the real work.

Report per function: address, target instruction count, yours, MATCH or the
exact residual diff. A table of thirty MATCHes and eight characterised residuals
is a far better round than three deeply-analysed near-misses.

### Two levers proven on `d_nand_thread.cpp` this week, both likely to apply

- **The bool-materialisation lever.** MWCC emits a three-instruction normalise
  (`neg`/`or`/`srwi.`) or a `cntlzw`/`srwi.` pair when an **opaque non-`bool`**
  value is stored into a real `bool` — an external call's return, for instance.
  Writing `if (!OSTryLockMutex(x))` gives a plain `cmpwi`; writing
  `bool locked = OSTryLockMutex(x); if (locked)` gives the target's sequence.
  Bool-storage alone does nothing and opacity alone does nothing. This closed
  four functions.
- **Return types are invisible to the mangling and are worth suspecting.**
  Nine signature corrections came out of that unit and only three were provable
  from symbol names. `bool` versus `void` versus `s32` changes codegen; the
  witnesses are the caller's use of the result and the function's own epilogue
  shape (`li r3,1` / `li r3,0` converging on one epilogue is `return true` /
  `return false`, not falling off the end).

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`. I am the only
  integrator; two builds in one checkout clobber each other.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose,
  with the shadow-test evidence.
- Do not touch `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, or any `GEMINI_*.md`.
  Gemini is surveying `d_basesNP` and holds `d_a_en_coin_main.cpp`; my own
  sub-agents hold `d_nand_thread.cpp` and everything under
  `wip/nand_thread/scratch/`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
