# Work order for Gemini — round 16

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 16.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 15 verified in full, and the method is why

I re-derived every vtable figure independently from `bin/dtk/wiimj2d_symbols.txt`
without reading your scripts:

```
__vt__8dActor_c            0x0D4  ->  51 slots
__vt__5dEn_c               0x280  -> 158 slots
__vt__9dEnBoss_c           0x390  -> 226 slots   ->  68 new over dEn_c
__vt__18dEnTorideKokoopa_c 0x5E4  -> 375 slots   -> 149 new over dEnBoss_c
```

Exact, slot for slot, including the 68/21/41 decomposition.

**The part that raises this above a good analysis: you did not stop at
argument.** You compiled the proposed header with the real CodeWarrior and
matched all 226 slots against retail one-for-one, then placed a derived test
class and confirmed its first new slot lands at 226. For a declaration-order
claim that is the only adequate standard, because being 42 slots out is
completely invisible in prose — which is exactly the round-14 error, and you
closed it yourself.

**Task B's confirmed negative is worth as much as the positive.**
`include/game/bases/d_enemy.hpp` declares exactly 158 virtual slots with zero
divergence from retail. That header sits under every enemy unit in the project;
a defect there would have been latent everywhere. Establishing that it is clean
removes a whole category of doubt.

**Your own work has unblocked the largest unit on the board. It is yours.**

---

## Your task: first pass on `d_enemy_toride_kokoopa.cpp`

**33,552 bytes — larger than the last six landed units on this project
combined.** I verified the shape myself:

```
.text   VA 0x800A8710 - 0x800B0A20   (offset 0xA1F90-0xAA2A0)   0x8310 = 33,552 bytes
__sinit_\d_enemy_toride_kokoopa_cpp = .text:0x800AED40, size 0x1698
251 functions in range.  ZERO anonymous -- every one carries a real mangled name.
sizeof(dEnBoss_c) = 0x600, confirmed by your own round-15 measurement
```

**Twenty-eight states, named outright in the symbol map:**

```
Attack  AttackBegin  AttackEnd  AttackReady  AttackSearch
BigJump  BigJump_St  Jump  Jump_St  LandOn  ShellAtk  ShellAtk_St
DemoAwake  DemoAwake_Wait  DemoIkaku  DemoIkaku_Wait  DemoWait  DemoEscape_St
DieFire  DieFumi_St  DieShell
FireHit  FumiHit  QuakeHit  ShellHit  ShellOut  SlideHit  StarHit
```

### This round is a FIRST PASS, not the whole unit. Scope it deliberately.

251 functions is too many for one round and I would rather have a solid
foundation than a thin sweep. In priority order:

1. **The class layout for `dEnTorideKokoopa_c`.** This is the shared prerequisite
   every function depends on; an agent authoring against a wrong layout burns its
   whole round. Verify offsets with **compiled `offsetof` assertions plus a
   negative control proving the check discriminates** — that is the standard that
   worked on the last unit, and reading offsets off a disassembly and asserting
   them is not it.

2. **Declare all 28 states and MEASURE how many functions that alone emits.**
   Report that number separately — it is the figure I most want. On the previous
   unit, declaring 25 states took it from 0 to 67 matching functions with only a
   constructor and five stubs authored. Do the framework before hand-authoring
   anything.

3. **The 41 `dEnBoss_c` methods Kokoopa overrides**, which you already
   enumerated — these have real bodies to write. Author as many as the round
   allows, largest-first or easiest-first as you judge.

4. **Report what remains**, so the next round starts from a map.

### A warning specific to this unit, from the last one

**Report BYTE-WEIGHTED progress, not just a function count.** On the previous
unit the state framework emitted 67 of 182 functions — 36.8% by count but only
**6.9% by bytes**, because framework-emitted functions are many and small. A
count overstated it more than five to one. Give both figures.

### A layout trap that cost a round on the last unit

An offset was recorded as padding because the constructor never touched it. It
was a real field, written only by two state-transition methods. **"The
constructor never touches it" is an argument from silence.** Before calling any
offset padding, grep the whole unit's disassembly for that offset — one command,
and it would have caught it immediately.

---

## Rules of evidence that have each cost a round here

- **Return types are ABSENT from CFront mangling; parameters are encoded.** Well
  over two dozen wrong declarations found. Read what the CALLER does with the
  return register right after the `bl` — read, or clobber.
- **A declaration is only tested by a CALL SITE.** An uncalled function's
  declaration is unverified however byte-exact its body is. Fourteen functions on
  the last unit had wrong return types purely because nothing had called them.
- **An argument-count mismatch at a call site is a STORAGE-CLASS tell** — one
  register where two are expected means no implicit `this`. Found four times
  today. **The trap: a static member function whose body never uses `this`
  compiles BYTE-IDENTICALLY to the non-static one**, so it is invisible in the
  function and shows only at its callers.
- **The `.fn <name>, global` tag answers LINKAGE, not the static-member
  question.** `static` at file scope means internal linkage and the tag sees it;
  `static` on a member means no implicit `this` and the tag is silent. Do not
  conflate them — I checked, and every function on the last unit was tagged
  `global` including two proven static members.
- **Check LENGTH before counting differences.** A length mismatch is CONTENT in
  both directions; register allocation cannot change an instruction count. Four
  agents filed a length mismatch as "register allocation" today.
- **But a matching length is not proof either.** Four functions were length-exact
  *by cancellation* — a spurious instruction masking a real gap — and the correct
  fix made the length column look worse. **Only BYTE equality settles anything.**
- **`bl _savegpr_N`/`_restgpr_N` versus inlined register stores is a WHOLE-TU
  decision.** A function compiled in isolation can differ structurally from
  identical source in the full TU — 27 words on one function measured. The size
  rule holds only within a fixed compilation context. **But a regime mismatch
  does NOT predict a defect** — I tested that across every parked unit and six
  units with a mismatch had already landed byte-perfect.
- **`harness.canonicalise` reports FALSE MISMATCHES** when the target's
  disassembly quotes a symbol name and a standalone `.o` does not. If a function
  is length-exact and the comparator still says differ, compare raw instruction
  BYTES.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- **Never edit anything under `source/` or `include/`, nor `syms.txt`, nor any
  `slices/*.json`.** Header changes are proposals in your response; I apply them
  and verify five binaries before anything lands, so a failure is never
  ambiguous.
- Work only in `scratch/gemini_round16/`. Do not touch `wip/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `peer_archive/`, or `QWEN_*.md`. **`wip/` has live agent
  work in it**, and Qwen is closing two functions in `d_bg_actor_mng.cpp` this
  round.
- Name your draft `d_enemy_toride_kokoopa.cpp` — anonymous-namespace symbols
  mangle the source filename into them.
- Mark anything unproven `@unofficial`. A `u8 pad[N]` for a region you cannot
  explain is a good answer; an invented member name is not.

## Deliverable

`GEMINI_RESPONSE.md`, containing:

1. **How many functions the 28 state declarations alone emitted**, before any
   hand-authoring. Report this on its own line — it is the headline.
2. **Both figures for overall progress: N/251 by count AND percentage BY BYTES.**
3. The proposed `dEnTorideKokoopa_c` layout, with the method by which each offset
   was verified, and explicitly which offsets you PROVED versus inferred.
4. A per-function table, **length column first**, for everything you authored.
5. Your source and header proposal in fenced blocks.
6. What remains, mapped, so the next round does not start cold.
7. Anything you could not settle, plainly, with what would settle it.

I re-measure everything independently. An honest DIFF row is worth more than a
claimed MATCH, and if something I assert above is wrong, say so with the
measurement — agents corrected me six times today and each correction was worth
more than the round's tally.

Plain ASCII or clean UTF-8, LF, no BOM.
