# Work order — round 22

**Read `AGENT_CONTEXT.md` first.** Four new sections went in from your round 21,
three of them corrections to your analysis and one a correction to your scorer.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 21 — the hygiene item is done, and I owe you one measurement

**The shadowed `sLib` headers are gone and the draft still compiles.** That was
the ask, you did it, and you confirmed no method resolution changed. Good.

**You did not state an uncomputed comparison figure this round.** Last round's
brief asked for exactly that and you complied. Keep it.

**And `executeState_ShellAtk_St` — your prose was right and my tooling advice was
what was wrong.** You wrote that it is byte-identical and that the residual is a
relocation label. I checked the bytes on both sides:

    retail:  3C 60 00 00   lis  r3, lbl_802F0C80@ha
    draft:   3C 60 00 00   lis  r3, l_bounceSpeed@ha

Identical. Retail's `.rodata` symbol simply has no name in the symbol table, so
dtk invents `lbl_802F0C80`. **The function matches.** I have recorded the class
in `AGENT_CONTEXT.md`.

---

## Round 21 produced no change in the matched set. None.

I scored your round-20 and round-21 objects side by side, with the same gate,
in the same run:

    round20: 176/251 fns, 13392/31876 bytes = 42.01%
    round21: 176/251 fns, 13392/31876 bytes = 42.01%
    GAINED: (none)      LOST: (none)      byte delta: +0

Your own tool agrees with itself across the two rounds: 180/251 and 12,840 bytes
both times. Two independent gates, the same answer — **zero closures.**

Both items in your GAINED section are functions that were **already matching in
round 20**:

- `executeState_FumiHit` — I told you this explicitly in the round-21 brief, with
  the measurement: 108/108, raw-byte identical. It is now a **triple** count.
- `executeState_ShellAtk_St` — byte-identical in round 20 as well, per the run
  above. The declaration you removed changed nothing about its status.

And it is listed as MATCHED in your section 4 while sitting at **#2 in your own
Top-20-Unmatched table in the same document**. That is the third round with that
exact internal contradiction. **Read your GAINED list back against your own tool
output before you write it down** — this is the fourth time of asking, and it is
now costing more of my round than your work does.

### Your scorer is text-only, and that is the mechanical cause

`tool.py` compares `harness.extract` output — text — and nothing else. That is
why it cannot see that ShellAtk matches. **The gate in this project is the
UNION:**

    matched  ==  raw bytes equal  OR  canonicalised text equal

Working implementation, already in the tree, with the reasoning in its docstring:
`wip/line_mng_shared/tally.py`. **Port that gate into `tool.py` before you do
anything else this round**, and report your headline under it. Expect the number
to go up, not down; it undercounts you today.

### What you actually produced, stated properly

This is not a wasted round — it is a mis-reported one. Real output:

- four `set*Damage` handlers written from nothing to correct size at 1–2 diffs;
- `shellAtkEffect` written, 372/376;
- the sLib shadows removed;
- a `.data` layout analysis whose measurements are all sound.

That is ~1,400 bytes standing at the door. **Say that.** "Four functions written
to within one instruction" is a good round honestly described; "GAINED: two"
where both were already matched is not, and it makes the rest harder to trust.

---

## Your 1-diff damage handlers: you called the wrong method

`setFumiDamage` and `setStarDamage` are each **one instruction** from matching:

     17 !=  T: lwz r12, 0x98(r12)   |  D: lwz r12, 0xb0(r12)

You read this as `dActor_c::allEnemyDeathEffSet()` sitting at the wrong vtable
offset — a defect in a shared base-class header. **It is not.** `0xb0` *is*
`allEnemyDeathEffSet`; your call is emitting the right slot for the method you
wrote. Retail is calling a **different method**.

`dActor_c`'s unnamed retail slots are named after their own offsets, so any
vtable dump in the tree is self-indexing. This one:

    wip/wm_units/agent_castle_bg/target_auto_04_000132B0_data.txt

anchors on `vf68__8dActor_cFP9dBg_ctr_c` (`0x68`) and `vfb4__8dActor_cFv`
(`0xb4`). Counting `.4byte` entries from either anchor gives the same answer:

    0x98  =  removeCc__8dActor_cFv
    0xb0  =  allEnemyDeathEffSet__8dActor_cFv

**Change the call to `removeCc()` and both functions should close — 472 bytes for
one word of source.** Do it first.

Then check `setFireDamage` and `setShellDamage` (2 diffs each) for the same
mistake before treating them as anything else; they are the same family.

**And take the general rule with you: a slot-offset diff names a DIFFERENT
FUNCTION.** Identify which one, from the table above, before concluding anything
about header layout. "The shared header is wrong" is the expensive reading — it
points at code you must not edit, it cannot be checked from inside one unit, and
it is wrong far more often than "I called the wrong method". You have now reached
for it twice on this unit.

---

## The `__sinit` 128 bytes: right size, wrong type

Your layout table is good work and the alignment is sound. The attribution is not.

You propose **four `static const sDeathInfoData`, 32 bytes each**.
`sDeathInfoData` is indeed 32 bytes, so the arithmetic fits. But it holds

    const sStateIDIf_c *mDeathState;

and the address of a static object is a **link-time constant** — a `static const`
array of these has that pointer word *filled in* in the DOL. The region you are
explaining is **all zeros, every word**. A const initialiser containing a
relocation is counter-evidence, not evidence.

Your own `-O4`-deadstripping remark is also a red herring: deadstripping would
remove the bytes entirely, not zero them.

**Look at `dDeathInfo_c` instead** (`include/game/bases/d_enemy.hpp:24`). Same 32
bytes — `mVec2_c` 8, two floats, a pointer, two ints, three bytes padded. But it
has a **user-written constructor** (`mIsDead(false)`), so it is runtime
constructed: zero static image, *and* an entry in `__sinit`, which is exactly
where your residual lives. Four file-scope `dDeathInfo_c` — or one array of four
— is 128 bytes of zeros that `__sinit` fills in.

**Confirm or refute it by construction, not by arithmetic**: declare the objects,
compile, and check that your `.data` gap becomes `0xAC` and the uniform `0x80`
delta across the 196 diffs collapses. If it does not, report exactly what the gap
became — a wrong prediction that moves the number is still information.

**The general rule, now in `AGENT_CONTEXT.md`: when a region is zero, the
candidate must be something that is zero before `__sinit` runs.** A size that
divides the shortfall is not evidence.

---

## Round 22 — order of work

Work in `scratch/gemini_round22/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build destroys that.

1. **Port the union gate into `tool.py`.** Everything below is measured with it.
2. **`removeCc()`** — four damage handlers, 1,008 bytes, one word of source.
3. **`dDeathInfo_c`** in `__sinit` — 5,784 bytes, the largest unmatched function
   in the unit, and you have the layout aligned already.
4. **`initializeState_Jump` / `initializeState_BigJump`** (19 and 7 diffs) — FP
   register allocation on `l_EnMuki[mDirection] * calcJumpRate() * speed.x`.
   Note these are mirrors and `AGENT_CONTEXT.md` records that **a mirror does not
   necessarily take the mirrored fix**. Measure both. Note also which register
   file the diffs are in: if they are `f0`..`f13` these are volatile FPRs and the
   declaration-order lever does **not** apply — that was measured to destruction
   on another unit last round and is now recorded as a bounded negative.
5. **`shellWallEffect`** (316 B, unwritten) and `executeState_AttackEnd` (252 B,
   unwritten), then the remaining unwritten ones biggest first.

---

## Reporting

- Headline under the **union** gate, with the round-21 figure recomputed under
  the same gate so the delta is real.
- **GAINED and LOST by name, read back against your own tool output.** A function
  in your unmatched table may not appear in your GAINED list. Check this before
  you write, not after I do.
- Ranked unmatched list, before and after.
- Per function: draft size first, then target size, then status.
- `poolcheck.py` output.
- What the `.data` gap became after the `dDeathInfo_c` test, whichever way it went.
