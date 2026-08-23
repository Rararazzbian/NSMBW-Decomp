# Work order — round 32

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 31: the volume test passed. This is your best round.

Verified before reading your report:

- **T0 gate hit exactly** — `125 / 0x50 / FPR [29,30,31]` from a fresh compile of
  the true baseline. The harness works and you proved it before proceeding.
- **13 objects, all compiled in `scratch/round31/`**, timestamps fresh. No
  copied `.o` or `.txt`. The round-29 problem is fixed.
- **All eight tasks attempted, none silently skipped**, with four genuinely new
  sources authored (`t2_baseline`, `t2_local_vec`, `t5_baseline`, `t5_stores`).
- **`target_math_dokan` rebuilt fresh and confirmed at 88 words**, as required.
- Honest throughout: no variant matched, and you said so.

Eight tasks is a workload you can carry. Two things went wrong under that load.

### You met most acceptance criteria by re-measuring old sources

Of your 13 objects, **only four came from source you wrote this round**
(`t2_baseline`, `t2_local_vec`, `t5_baseline`, `t5_stores` — and the two
variants between them change 7 and 4 lines respectively). The other nine are
round-28 files copied in and recompiled.

Recompiling was right — I asked for fresh objects and you produced them. But a
recompiled old variant is not a new experiment, and counting it as task coverage
overstates the round:

    T1  calc            required >=3 variants  ->  2, both recycled     MISSED
    T3  fn_80080E40     required >=2 variants  ->  1, recycled          MISSED

**A variant is new source, written this round, testing a stated idea.** Rebuilding
last round's file measures what you already knew. From now on the table has a
"new this round?" column and recycled rows do not count toward a task's minimum.

### Target-side precision degraded

Three of your target measurements are wrong, and one of them cost you the best
function on the board.

---

## The three errors, smallest first

**1. `calc` target FPR saves.** You reported `[29,30]`; the target saves
`[30,31]`. The count is right, the identity is not.

**2. T8's `32 / 7 / 0` is round 27's number, not round 31's.** Your
`diff_ctr.py` has `BASE = scratch/round27/d_bg_ctr` hardcoded, and there is no
`draft_disasm.txt` in `round31/` at all — the script never looked at your work.
The *conclusion* is true (nothing matched, so nothing changed), but the
measurement behind it is four rounds stale. When you copy a script, repoint its
`BASE`.

**3. `revisePos` — and this one matters.** You reported *"72 words / frame 0x30
versus target 72 words / frame 0x70"*, called the frames different, and closed
the function as a bounded negative.

**The target frame is `0x30`.** I extracted it: `revisePos` target is
`72 words, frame 0x30, no FPR saves`. Your draft is `72 words, frame 0x30`.

    words:  72 == 72     frame: 0x30 == 0x30     FPR saves: none == none

**Everything structural already agrees.** This is the closest any function in
this unit has been to matching, the diff count *is* meaningful here because the
frames agree, and you retired it on a number that was wrong. **Reopen it.**

Note the shape of this mistake: two of the three errors are target-side facts
you had the data to extract and instead stated from memory or from the prompt.
Several table rows even say *"target frame not stated in prompt"* — you have
`target.txt`; extract it. **Never take a target-side figure from a brief when
you can measure it.** My briefs are not authoritative about the binary; the
binary is.

---

## Round 32 — the work list

Everything in `scratch/round32/d_bg_ctr/`. Carry `build.py` forward — it worked
— and **repoint `diff_ctr.py`'s `BASE` to round 32.**

### T0 — extract every target figure yourself, once

Before any variant work, produce a table straight from `target.txt` for all
seven remaining functions:

| Function | target words | target frame | target GPR saves | target FPR saves |

This is the reference for the whole round and it replaces every target figure in
my briefs. **Acceptance:** seven rows, all extracted, none quoted from me.

### T1 — `revisePos`: the reopened function, and the priority

72/72, frame `0x30` both, no FPR saves either side. Pure instruction selection
and ordering, and the diff count is meaningful. **Get the actual diff list** —
which instructions differ and where — and work from that rather than from
whole-function rewrites.

You have three measured negatives already (target-read-order ×2,
store-before-call). Those were all *structural* attempts on a function that is
already structurally correct. Read the diffs and fix what they say.

**Acceptance:** the diff list, and ≥3 variants driven by it.

### T2 — `calc` (+4)

Target `125 / 0x60 / FPR [30,31]`; baseline `125 / 0x50 / FPR [29,30,31]`. The
target has more stack and fewer saved FPRs, so it holds a local you do not.
Ruled out and not to be retried: store-before-call, trig ordering, declaration
order (139 words, worse). **Acceptance:** ≥2 variants.

### T3 — `fn_80080670` (−3)

Target `130 / 0xB0 / no FPR saves`; your baseline `127 / 0xB0 / FPR [30,31]`.
Frames agree, so **the diff count is meaningful here too — get the diff list.**
Your `t2_local_vec` attempt made it worse (129/0xC0); do not repeat it.
**Acceptance:** diff list + ≥2 variants.

### T4 — `addDokanMoveDiff` (−7)

`target_math_dokan` at 88 versus target 87, frames agree at `0x60`. One word,
and the diff count is meaningful. **Get the diff list.** **Acceptance:** diff
list + ≥2 variants from it.

### T5 — `fn_80080E40` (−4) and T6 — `fn_8007FFA0` (−8)

Both still content-short. `filter_dc_first` reached 122 vs 121 — one word, but
check whether the frames agree before trusting the diff count.
**Acceptance:** ≥2 variants each.

### T7 — `fn_80080900` (−48)

One serious pass, as before. **Acceptance:** one compiled attempt with frame and
`_savegpr` reported.

### T8 — canonical fold

Only if something matched. Build a real canonical `d_bg_ctr.cpp` in
`scratch/round32/`, disassemble it to `draft_disasm.txt` **in that directory**,
and run the repointed `diff_ctr.py` against it.

---

## Note the pattern in T1/T3/T4

Three functions now have **matching word count and matching frame**. For those,
the diff count is a real score and the individual differing instructions are the
work list. That is a much better position than a frame mismatch, and it is where
the round's value is. Prioritise them over `fn_80080900`.

---

## Constraints

Work only in `scratch/round32/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `GEMINI_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`.**

---

## Reporting

Same table as round 31, plus one column:

| Task | Variant file | **New this round?** | Words (T/D) | Frame (T/D) | GPR saves (T/D) | FPR saves (T/D) | Diffs |

Every row from an object compiled in `scratch/round32/`. **Only rows marked new
count toward a task's variant minimum.** Recycled rows are welcome as reference
points — mark them and do not count them.

A compile takes under half a second. Nothing in these minimums is expensive; the
cost is in deciding what to change, which is the part I am asking for.

- **T0's target table first**, extracted by you.
- Diff lists for T1, T3, T4.
- **GAINED / LOST by name**, matched-status changes only.
- Tasks not reached: list them as not reached.
- `poolcheck.py` on any canonical object.
- **Drop `Offset-perturbing`.** Fifth time. It has never once carried
  information; delete the field.
