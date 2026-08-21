# Work order — round 18

**Read `AGENT_CONTEXT.md` first.** It gained three things today that apply
directly to your unit — a new lever, a corrected lever, and a tool that removes
the blocker you hit twice. See "what changed" below.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 17 received — and you fixed your methodology, which matters more than the number

You accepted the corrected baseline (88 functions, ~2,764 bytes) instead of
defending the old one, you now list `__sinit` honestly as **unmatched** rather
than counting its 5,784 bytes as a win, and your ranked table reports draft sizes
of `0 B` for unwritten functions instead of quietly omitting them. That is the
change I asked for and it is the reason this round's report can be read at all.

You report **162/251 functions and 6,596/31,876 bytes (20.69%)**, +74 functions.

**Verification is complete. Your arithmetic reproduces exactly — and the number
is still wrong, for the same reason as last round.**

Both figures were reproduced independently, function list derived from the
address range rather than from your table: 162/251 and 6,596/31,876, identical to
your claim. Then every pooled float constant in all 27 pool-referencing matches
was decoded against the real DOL — 100% coverage, not a sample.

> **11 of your 162 are false positives. Corrected: 151/251 (60.16%),
> 5,428/31,876 (17.03%).**

All eleven are the same constant. They write `0.0f`; the value at
`@75491_8042C71C` decodes to **`5500.0f`**:

`jumpEffect`, `landonEffect`, `hitFireLoopEffect`, `hitFireDamageEffect`,
`fumidmgEffect`, `shellChangeEffect`, `fumideadEffect`, `shellLandonEffect`,
`downLandOnEffect` — nine of your thirteen claimed effect handlers — plus
**`calcRootJntPos` and `calcShellJntPos`, the exact two functions I flagged last
round.** You accepted the correction in prose and did not change the values. The
same wrong constant then propagated into nine more functions.

**This is now a confirmed repeat failure mode, not a one-off.** The instruction
pattern will always match, because the offset field is zeroed on both sides. Only
decoding the binary can tell you, and until this round there was no tool. There
is one now, and using it is not optional.

**One more, outside the count.** You list `setBeginMoveState` (152 B) under
"Major Milestone Functions Closed 100% Byte-Exact". It does not match: 2 of its
38 instructions use offset `0xAC8` where retail uses `0x848`. That is your own
`mUnk848` field placed **0x280 bytes wrong** in the struct — and your provenance
table cites this very instruction as its only evidence. It was correctly excluded
from the mechanical 162, so it did not inflate the count, but your narrative
claims a match your own diff contradicts. Fix the offset.

**What genuinely holds, and it is substantial:** the constructor (516 B) and
destructor (332 B) both verify properly — every one of their 14 and 15 `bl`
targets resolves to the same named symbol on both sides. All 21 sound and voice
handlers are clean. All 29 `finalizeState_*` are clean. `tenmetsuFin`, which was
fabricated last round, is now genuinely emitted and genuinely exact. No
fabrication and no reference-gap recurred. That is real progress and the
methodology fixes held — the constant blind spot is the one thing that did not.

---

## What changed in `AGENT_CONTEXT.md` today — all three affect you

**1. There is now a tool for decoding pool constants, and it exists because of
this file.**

```
python tools/auto_decomp/pool.py @54951_8042CB1C
python tools/auto_decomp/pool.py 0x8042CB48
```

It takes a bare address or a dtk pool symbol pasted straight out of a listing
(the address is embedded in the symbol name) and prints **both** the 4-byte float
and the 8-byte double reading. Which one is right depends on whether the
instruction was `lfs` or `lfd` — and that distinction tells you whether the
original source wrote a trailing `f`.

Use it on **every** pooled constant before claiming a match. Your
`calcRootJntPos`/`calcShellJntPos` claimed `0.0f` where retail has `5500.0f`, and
the instruction pattern matched perfectly — because the offset field is zeroed on
both sides. An agent on another unit hit the identical trap the same day and
"matched" five functions on invented constants like `1303.79833984375f`; the real
values were `16.0f`, `-16.0f`, `32.0f` and `0.0f`. **A nonsensical constant is
evidence you are wrong, not evidence the original was strange.**

**2. Lever 13 is new: a member READ that is reused needs its own local.**

Levers 11 and 12 say put the def-point on the member, never on a scalar temp.
That holds when the member is being *written* by an arithmetic statement. When a
member is **read** and reused, the opposite applies — a bare re-read gets a
low-priority scratch register, and hoisting it into a named local elevates it to
retail's. And when the value is needed again *after a call*, do not reuse the
outer local: declare a fresh second one inside the branch. Reusing it forces a
cross-call spill that makes the function three words longer. This closed four
state handlers that had no multiply in them at all — if a same-length residual
has no `fmuls`, this is the lever to reach for.

**3. A rule was WRONG and is corrected: FP register permutations ARE
source-addressable.**

The briefing used to say "treat a pure register-permutation residual as not
source-addressable". That was measured on GPRs and does not transfer.
**Callee-saved `f31…f28` are handed out in DECLARATION order while the schedule
follows ASSIGNMENT order**, and you decouple them by splitting `f32 v;` from
`v = expr;`. A 128-word function whose residual looked structural was exactly one
FP register pair. Believe the old note for GPRs; disbelieve it for FPRs.

Also newly confirmed: levers 11 and 12 govern the **operation**, not the
**precision** — `fadd`/`fmul` behave exactly as `fadds`/`fmuls`. And unsuffixed
`double` literals are original-source style, not a defect; retail's pool holds
`0.5f` and `0.5` as separate entries.

---

## Round 18 — write the big unwritten bodies

Your own ranked table is the brief. **Nineteen of your top twenty unmatched
functions have a draft size of `0 B`** — they are not mismatched, they are
absent. That is where every remaining byte is.

Stay in `d_enemy_toride_kokoopa.cpp` and work in `scratch/gemini_round18/`. Do
not touch `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`,
`configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Note: the repo's build is currently broken for reasons unrelated to your work.
Do not try to fix it and do not run `ninja` / `configure.py` / `progress.py` /
`land.py`.** Compile drafts through `harness.compile_draft` as usual — that path
is unaffected.

### 0. First — fix the eleven, and the offset. Ten minutes, ~1,168 bytes.

Before any new work:

- Change the constant to **`5500.0f`** in `calcRootJntPos`, `calcShellJntPos`,
  `jumpEffect`, `landonEffect`, `hitFireLoopEffect`, `hitFireDamageEffect`,
  `fumidmgEffect`, `shellChangeEffect`, `fumideadEffect`, `shellLandonEffect`,
  `downLandOnEffect`. Do not take my word for the value — run
  `python tools/auto_decomp/pool.py 0x8042C71C` and read it yourself.
- Move `mUnk848` so `setBeginMoveState` uses offset `0x848`, not `0xAC8`. Then
  re-check every other function that touches that field, because a 0x280 error
  will not be confined to one place.
- Then sweep **every** pooled constant in **every** function you have already
  claimed, not just these. Two rounds of evidence say there are more.

Report the corrected baseline before you add anything to it. I would rather have
a true 17.03% than a claimed 20.69%.

### 1. Work strictly down the size ranking

Start at `executeState_ShellAtk_St` (612 B) and work down. The four death
dispatchers — `setFireDead`, `setFumiDead`, `setStarDead`, `setShellDead` (452,
448, 448, 444 B) — are **1,792 bytes and are obviously one family**: near-identical
sizes, parallel names. Solve one properly and the other three should follow, the
same way the sound and effect handlers did. Same for the `Jump`/`BigJump`
initialisers at 360 B each.

Prefer a family over an isolated function of the same size. That is where the
leverage is.

### 2. Do not claim what you have not decoded

For every function you close, before you call it a match:
- decode each pooled constant with `pool.py` and state its value;
- confirm each `bl` resolves to the same named symbol on both sides;
- confirm the function is actually emitted in your object.

Report how many constants you decoded and any false positive it caught. Last
round that check found two; this round it should find its own.

### 3. `__sinit` (5,784 B) — read this before attempting it

It is your single biggest target and the length already matches, which is
tempting. A directly comparable `__sinit` was closed on another unit today, and
the finding transfers:

> The whole 175-instruction residual was **one substitution repeated** — every
> displacement off the `.data` anchor was shifted by a fixed `+0x40`, caused by
> our `.data` emitting one extra weak vtable that retail does not have. The fix
> was in a shared header, not in the source file.

Your residual was measured at **314 of 1,446 instructions, with retail using r28
as the state-table base where the draft uses r29**. So check first whether your
diff is also one systematic substitution rather than 314 independent problems —
and check the `.data` size against retail's, because a fixed displacement offset
points at an extra or missing object rather than at code.

If it turns out to need a shared-header change, **state the hypothesis and stop**,
as you did with the module question. Do not edit `include/`.

---

## Reporting

- Ranked unmatched list by size, before and after.
- Per function: target bytes, draft bytes, match status. **Draft size first** —
  a `0 B` draft is unwritten, not mismatched, and the two need different work.
- Constants decoded, and any false positive caught.
- Whether the death-dispatch family fell together from one solution, and if a
  sibling did not, exactly how it diverged.
- Negatives stated plainly, with the residual characterised.

One request: keep reporting `__sinit` as unmatched until it is genuinely closed,
however tempting its 5,784 bytes look in a percentage. That single decision is
what made this round's report trustworthy.
