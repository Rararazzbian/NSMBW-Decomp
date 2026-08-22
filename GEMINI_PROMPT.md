# Work order — round 24

**Read `AGENT_CONTEXT.md` first.** One new section went in from your round 23,
and it is a correction to something I asserted twice.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 23 is the round I asked for. All three items land.

I verified every claim independently before writing this. Stated plainly,
because you have not had one of these yet:

**All 12 regressions are genuinely recovered.** I checked each by name against a
fresh compile. Twelve for twelve.

**Your LOST section is real, and it is real zero.** I diffed your matched set
against round 22 by name:

    GAINED: 15 functions, 1,292 bytes
    LOST:    0 functions,     0 bytes

That is the first clean loss report in six rounds. It is also what made this
round cheap for me to check, which is the whole point of asking for it.

**Your headline is within one function.** Under my gate with the naming-artifact
rule applied: **229/251, 27,308 bytes**, against your claimed 230 / 27,560. The
gap is arithmetic, not inflation — your own GAINED table has **15 rows** while
its heading says 16, and the byte figure is 252 over. Count the table you wrote.
That is the entire remaining defect in your reporting, and it is a rounding
error next to where you were two rounds ago.

**And the 128-byte dump is byte-for-byte correct.** I read the region out of
`original/wiimj2d.dol` myself and it matches yours exactly, including the string.

---

## I was wrong about the 128 bytes, twice, and your dump is what settled it

I told you the region was "all zeros, every word", and I repeated it. **Byte 0 is
`0x64`.** It is the `d` of `"dEn_c::StateID_EatOut"`, and the region is the tail
of a **different translation unit** — not yours at all:

    0x803142E0: 64 45 6E 5F 63 3A 3A 53 74 61 74 65 49 44 5F 45  [dEn_c::StateID_E]
    0x803142F0: 61 74 4F 75 74 00 00 ...                          [atOut...........]

Your landing assessment follows from that and I accept it: `d_enemy_state.o`
occupies those bytes in the real link, alignment padding fills the rest, and
`__vt__18dEnTorideKokoopa_c` lands at `0x80314360` without your help.
**`g_padData` is a scratch-harness artifact, not a proposed occupant** — keep it
in the harness so your offsets stay calibrated, and mark it clearly as
non-landing so nobody tries to promote it. **The question is closed.** Do not
spend any more of a round on it.

Note what closed it: one hex dump, after five rounds of arithmetic about a
region nobody had looked at. `AGENT_CONTEXT.md` now carries the rule — **read
the bytes before theorising about them, and a chain of correct arithmetic on an
unread region is still a guess.** That one was mine, not yours.

---

## Two functions are worth 764 bytes for three instructions

You have **22 genuinely unmatched functions, 4,568 bytes**. Two of them are
nearly free and one is not in your own top-unmatched list:

- **`executeState_AttackEnd` — 252 B, 252/252, ONE diff.** You did not report
  this one at all. It is the best value on the board.
- **`executeState_AttackSearch` — 512 B, 512/512, TWO diffs.** You reported it as
  one; it is two. Your `li r3, 0` / `li r4, 0` reading is right as far as it goes.

Both are the right length with the right control flow. Do these first.

**Ignore these two — they are matches my gate mis-scores**, per the naming-artifact
rule: `__sinit` (4 diffs) and `executeState_ShellAtk_St` (2 diffs). Both are pure
`SYM<n>`-renumbering. Count them as matched and do not touch them.

After the two cheap ones, biggest first:

    376 B  shellAtkEffect        (372/376, 52 diffs)
    360 B  initializeState_Jump      (6 diffs)
    360 B  initializeState_BigJump   (6 diffs)
    340 B  setQuakeDead          (352/340, 84 diffs)
    268 B  preExecute            (252/268, 48 diffs)
    208 B  moveRevise            (204/208, 22 diffs)
    204 B  calcAttackTarget      (196/204, 21 diffs)
    188 B  calcJumpRate          (176/188, 45 diffs)
    188 B  movelimitCheck        (196/188, 39 diffs)
    180 B  checkDownJump         (160/180, 42 diffs)

On **`initializeState_Jump` / `initializeState_BigJump` (6 diffs each)**: you
report the residual is in `f0`..`f4`. If that is accurate these are **volatile**
FPRs, the declaration-order lever does **not** apply, and `AGENT_CONTEXT.md`
records that as a measured bounded negative. **Confirm which register file the
diffs are in before spending anything on them**, and if they are volatile, say so
and move on rather than grinding. They are also mirrors, and a mirror does not
necessarily take the mirrored fix.

The 40-plus-diff group (`shellAtkEffect`, `setQuakeDead`, `preExecute`,
`calcJumpRate`, `checkDownJump`) is where the real bytes are. Those are content
problems, not codegen problems — the lengths are wrong, so something is missing
or extra, and no register lever will help until the length is right.

---

## Round 24 — order of work

Work in `scratch/gemini_round24/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build destroys that.

1. `executeState_AttackEnd` (1 diff) and `executeState_AttackSearch` (2 diffs).
   764 bytes.
2. Check the Jump/BigJump register file, then either close them or record the
   negative and stop.
3. The length-wrong group, biggest first: `shellAtkEffect`, `setQuakeDead`,
   `preExecute`. Get the lengths right before touching registers.

---

## Reporting

Keep round 23's format exactly — it worked.

- Headline with round 23 recomputed under the same gate.
- **GAINED and LOST by name.** Then **count the rows of your own GAINED table
  and check the number against your heading** before you write the summary.
  That is the last thing standing between your report and mine.
- Do not count naming artifacts as misses; say where you applied the rule.
- Per function: draft size first, then target size, then status.
- `poolcheck.py` output.
