# Work order for Gemini — round 15

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 15.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 14 verified: Task A is clean, Task B has one error that would cost a round

I re-derived every number in round 14 independently, from `original/wiimj2d.dol`
and `bin/dtk/wiimj2d_symbols.txt`, without reading your scripts.

**Task A survived in full.** All seven of your carve ranges land on real function
boundaries. Your function counts and anonymous-symbol counts are exact — 123/13
for `d_bc.cpp`, 88/15 for `d_bg.cpp`, 22/0 for `d_bg_actor_mng.cpp`, 125/42 and
34/11 for the two `d_bg_unit` spans, 72/9 for `d_capture_mng.cpp`, 5/0 for
`d_beans_kuribo_mng`. Your `.ctors` addresses are exact and contiguous, correctly
bracketed by `__sinit_\d_base_actor_cpp` and `__sinit_\d_cc_cpp`. I walked
`d_bg_actor_mng.cpp`'s `.data`, `.rodata`, `.sbss` and `.sdata2` object by object
and every edge you gave is right, including the correct call that it owns no
`.bss` — the four anonymous `.bss` objects in that gap belong to `d_bg_unit`.

Two cosmetic slips, neither load-bearing: `__vt__17dBgActorManager_c` is `0xC`,
not `0x10`, and your byte-named percentages use range size as denominator while
mine used summed function size, which is why `d_bc.cpp` reads 82.9 against my
82.8. Neither changes a ranking.

**Qwen is authoring `d_bg_actor_mng.cpp` this round on the strength of that
carve.** Do not touch it.

### The Task B error

You reported `dEnBoss_c`'s vtable as 226 slots, and that is right — `0x390`,
confirmed. You reported Kokoopa at 375 slots (`0x5E4`) and the difference as 149,
both right. Your 149-slot decomposition is right: I decoded every slot and the
state triples land exactly where you said — `Jump_St` at 226-228, `Jump` at
229-231, `DieFumi_St` at 283-285, the five demo states at 358-372, `awakeSE` and
`ikakuSE` at 373-374. Your 26 named boss virtuals at slots 200-225 are also
exactly right, name for name and slot for slot.

**But those 26 are not all of `dEnBoss_c`'s new slots.** You wrote that slots
23-199 are inherited from `dEn_c`. They are not:

```
__vt__5dEn_c  = .data:0x80311EE0  size 0x280  -> (0x280-8)/4 = 158 slots
__vt__8dActor_c = .data:0x8030A26C size 0xD4  -> 51 slots, not 23
```

`dEn_c` ends at slot 157. `dEnBoss_c` introduces **68** new slots, 158 through
225 — the 26 you found plus 42 you missed:

```
158-175  six state triples: DemoWait, DieFire, DieSlide, DieShell, DieStar, DieQuake
176-186  setBattleReady, createModel, createBossLife, createInit, tenmetsuReady,
         tenmetsuProc, tenmetsuFin, getTenmetsuTime_Fire, getTenmetsuTime_Shell,
         getTenmetsuTime_Press, deadAllKill
187-199  setFumiDamage, setFumiDead, setFireDamage, setFireDead, setHipatkDamage,
         setHipatkDead, setSlideDamage, setSlideDead, setStarDamage, setStarDead,
         setQuakeDamage, setQuakeDead, setShellDamage
200-225  the 26 you already have, beginning setShellDead
```

A header written to your round-14 spec would declare 26 virtuals where 68 are
needed and every slot from 158 on would land 42 places early — the same cascade
you yourself named as the diagnostic for a missing declaration, arriving from the
other direction.

Everything else in Task B held. `sizeof(dEnBoss_c) == 0x600` is **confirmed**:
`__ct__18dEnTorideKokoopa_cFv` at `0x800A88A0` does `stw r29, 0x600(r27)` as its
first derived-member write. Your member layout is confirmed instruction by
instruction from `__ct__9dEnBoss_cFv` at `0x800983C0` — the allocator sub-object
at `0x524`, the zeroed word at `0x540`, the sound object at `0x544`, the halfword
at `0x5F0` and the two words at `0x5F4`/`0x5F8`. Your `.text` extent is right: the
range ends exactly where `__ct__20dEnBossKoopaJrBase_cFv` begins at `0x8009AD30`.

---

## Task A: produce the complete `dEnBoss_c` declaration order

This is round 14's Task B finished properly, and it is the deliverable that
actually unblocks the 33,552-byte Kokoopa unit.

**Deliver a proposed `include/game/bases/d_enemy_boss.hpp`** in which the virtual
declaration order reproduces the real vtable exactly. Three things have to be
right and each is separately checkable:

1. **The 68 new slots, 158-225, in order.** I have given you the ordering above;
   confirm it against the binary yourself rather than trusting me, and give each
   one a signature. Parameters come free from the mangling. Return types do not —
   the `is*Invalid` group at 203-208 is `const` and near-certainly `bool`, and the
   `getTenmetsuTime_*` group at 183-185 returns something you must read off the
   caller.

2. **Which of `dEn_c`'s 158 slots `dEnBoss_c` overrides.** I measured **21**
   overridden and 137 inherited unchanged. Name all 21. An override that is
   declared but not actually overridden is invisible in the slot count and
   silently wrong.

3. **Which of the 226 inherited slots Kokoopa overrides.** I measured **41**.
   Name them. This is what tells the Kokoopa author which methods have bodies to
   write versus which are inherited, and it is not derivable from the 149.

State the method by which you confirmed each of the three counts.

## Task B: audit the `dEn_c` declaration we already have

`dEn_c` is the base under all of this and `source/dol/bases/d_enemy.cpp` is
landed, so `include/game/bases/d_enemy.hpp` exists and declares some number of
virtuals. **Count them and compare against the real 158.**

If the declared count is not 158, say so with the measurement and locate where
the divergence begins — that is a defect sitting under every enemy unit in the
project, not just Kokoopa, and finding it is worth more than either task above.

If it is 158, say that plainly. A confirmed negative is a real result here.

---

## Things that have each cost a round here

**Return types are ABSENT from CFront mangling.** Parameters are encoded; return
types are not. Well over two dozen wrong declarations have been found on this
project so far, and the count is still climbing. The method that
finds them: read what the CALLER does with the return register immediately after
the `bl` — does it READ r3 or CLOBBER it? An observed clobber outranks any
analogy with a sibling.

**An argument-count mismatch at a call site is a STORAGE-CLASS tell.** One
register set where two are expected means no implicit `this` — the function is
`static`.

**Check SIZE before counting differences.** A length mismatch is CONTENT in both
directions; a positional or register residual cannot change an instruction count.

**A vtable region derived from what you can name is a LOWER BOUND.** That is the
round-14 error in one sentence: you enumerated the slots you could identify and
reported the count of those, where the count comes from the vtable's own size.
Take the size first, divide, and make the names account for the whole span.

**`harness.canonicalise` reports FALSE MISMATCHES** when the target's
disassembly quotes a symbol name and a standalone `.o` does not. If a function is
length-exact and the comparator still says differ, compare raw instruction BYTES.
I hit this myself verifying Qwen this round.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- **READ-ONLY on the tree.** Never edit anything under `source/` or `include/`,
  nor `syms.txt`, nor any `slices/*.json`. Header changes are proposals in your
  response; I apply and verify them.
- Work only in `scratch/gemini_round15/`. Do not touch `wip/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `peer_archive/`, or `QWEN_*.md`. **Qwen is authoring
  `d_bg_actor_mng.cpp` this round** — if your work reaches into it, report the
  finding rather than acting on it.
- Mark anything unproven `@unofficial`, and state which edges you PROVED versus
  inferred, separately and per edge.
- **Report a negative result rather than manufacturing a positive one.**

## Deliverable

`GEMINI_RESPONSE.md`, containing:

1. **Task A**: the proposed `d_enemy_boss.hpp` in a fenced block, with the 68 new
   virtuals in order and signatures argued from evidence; the 21 `dEn_c`
   overrides named; the 41 Kokoopa overrides named. Say how you confirmed each
   count.
2. **Task B**: the declared virtual count in the existing `dEn_c` header against
   the real 158, and if they differ, where.
3. For both: what you PROVED versus what you INFERRED, kept separate.
4. Anything you could not settle, plainly, with what would settle it.

I check every number independently — that is how round 14's Task B error was
found, and it is also how round 14's Task A was confirmed clean enough to hand
straight to another agent. If something I assert above is wrong, say so with the
measurement.

Plain ASCII or clean UTF-8, LF, no BOM.
