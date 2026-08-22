# Work order — round 21

**Read `AGENT_CONTEXT.md` first.** Several sections are new, and one of them —
"Do NOT shadow `include/game/sLib/*`" — was written because of a residual you
reported in round 20 and explains it outright.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 20 — the two things I asked for, you did

**`setBeginMoveState` is fixed.** Fourth time of asking, and it is done. Thank
you.

**`LOST: none`, reported as a set.** That is the first round with a clean
regression profile, and you reported it in the form I asked for rather than as a
net count. Both of last round's defects were addressed and you ran
`poolcheck.py` unprompted — 78 constants, 0 mismatched.

**The `__sinit` derivation is real work.** You traced the `+0x90` to 36 slots at
indices 374..409, gave the vtable span (`0x80314360`..`0x803149F0` = `0x690`
against a draft `0x600`), and showed how the 28 `sFStateID_c` instances that
follow it inherit the shift. That is the shape of an answer I can act on.

Independent verification is running on the numbers. One thing it is checking that
you should look at too: **your round-19 baseline changed.** Round 19 reported
11,136 bytes (34.94%) and that figure was independently confirmed; round 20 gives
the round-19 baseline as 9,712 bytes (30.47%). The function count is identical in
both tellings — only the byte figure moved, by −1,424. Please account for it.

Also: `executeState_ShellAtk_St` is listed in the GAINED set with "only 2 diffs".
**A function with 2 diffs is not matched.** Confirm whether it is inside your 180
or not; if it is, the count is 179.

---

## THE BIG ONE: your slot residuals are self-inflicted. Delete your sLib shadows.

You diagnosed `executeState_FumiHit`'s single remaining diff as
`sStateStateMgr_c::executeState` landing at slot `0x20` where retail has `0x1C`,
"due to `sStateMgrIf_c` interface inheritance in the shadow headers".

**The shadow headers are the bug.** Your tree carries its own copies of

    include/game/sLib/s_StateStateMgr.hpp
    include/game/sLib/s_StateMgr.hpp
    include/game/sLib/s_StateID.hpp
    include/game/sLib/s_StateInterfaces.hpp

and all four differ substantially from the real ones (132, 59, 41 and 73 diff
lines respectively). **The real `s_StateStateMgr.hpp` was corrected against
retail and verified alone** — it is what took another unit from 76.0% to 91.6%,
and its virtual declaration order IS retail's slot order:

    initializeState, executeState, finalizeState, changeToSubState, returnState,
    getOldStateID, refreshState, isSubState, changeState, getState,
    getNewStateID, getStateID

Your copy declares them in a different order **and adds two virtuals the real one
does not have** (`isState`, `getMainStateID`). Every slot after those two is
shifted. That is your `0x1C`/`0x20`.

**Round 21, item 1: delete all four shadow copies, compile against the real
`include/game/sLib/` headers, and re-measure the whole unit.** I expect this to
close `executeState_FumiHit` outright and to move the `__sinit` delta — your 36
"interface vtable slots" at 374..409 are exactly the shape of surplus interface
vtables, which is the same defect class the real header removed.

Report the full before/after: matched count, byte count, GAINED and LOST by name,
and specifically what happened to the `+0x90`. If the delta changes but does not
vanish, the new number is the finding — report it either way.

If a real sLib header genuinely will not compile in your context, say which and
what the error is, rather than shadowing it silently. Shadowing a shared header
discards work already verified against the binary, and neither of us can see it
happening from a per-function diff.

---

## Round 21 — the rest

Work in `scratch/gemini_round21/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build would destroy that.

### 2. The near-matches, after the header change

Re-measure these before working them; several may close for free:

- `executeState_FumiHit` (1 diff) — expected to close with the real header.
- `executeState_AttackSearch` (2 diffs) — the `searchBaseByID` ternary. Try
  hoisting it into a named local of the callee's parameter type so the argument
  register is fixed before the call, rather than the null arm being materialised
  into the return register.
- `executeState_ShellAtk_St` (2 diffs) — you describe these as "local bounce
  table label relocation naming". If that is a naming artefact rather than a real
  difference, prove it: show the two symbols resolve to the same address and the
  same bytes. If it is real, it is 2 instructions.
- `initializeState_Jump` / `initializeState_BigJump` (7 diffs each) — the
  `isNonDamage`/`isOneDamage` branch polarity. Note these two are mirrors, and
  `AGENT_CONTEXT.md` records that **a mirror does not necessarily take the
  mirrored fix**. Measure both.

### 3. Then the unwritten ones, biggest first

`shellAtkEffect` (376 B), `shellWallEffect` (316 B), `setFireDamage` (272 B),
`setShellDamage` (264 B), `setFumiDamage` (236 B), `setStarDamage` (236 B).

The four `set*Damage` functions are a family and should be taken as a group — the
death-dispatch family fell together for you in round 18 and this is the same
shape.

---

## Reporting

- The sLib header result first: before/after, and what happened to `+0x90`.
- The byte-baseline reconciliation.
- GAINED and LOST by name.
- Ranked unmatched list, before and after.
- Per function: **draft size first**, then target size, then status. A `0 B`
  draft is unwritten, not mismatched.
- `poolcheck.py` output.

Two standing cautions. **A function with any diffs is not matched** — do not
list one in a GAINED set with a diff count attached. And **a high score does not
mean landable**: I had a unit at 98.7% that broke all five binaries, because the
scoring tools never run the linker and cannot see an undefined symbol, a weak
symbol we place that retail takes from elsewhere, or a wrong section order.
