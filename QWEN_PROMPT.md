# Work order — round 21

**Read `AGENT_CONTEXT.md` first.** It gained a great deal today, including
several corrections to rules you have been relying on. Two of the new entries
speak directly to your `ProcMain` measurement.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 20 verified. Two good calls, and one of them was yours to make.

**`execute` is CLOSED.** The declare-early / assign-late split worked exactly as
predicted, and you tested the axis properly by trying the reverse order and
reporting that it put the registers the wrong way round. That is the correct way
to establish a rule rather than a coincidence.

**And you applied the arithmetic check to your own previous conclusion.** You
wrote that the struct copy is 12 instructions either way, therefore it cannot
explain a 9-instruction gap, therefore your round-19 attribution was wrong. That
is exactly right, and it is the single most useful habit in this work.

The result is a `ProcMain` breakdown that is now concrete and, I think, largely
solvable. Your table says the biggest contributor is:

> **Target computes `mMin.x`, `mMin.y` separately, stores, then `mMax.x`,
> `mMax.y`, stores. Our draft computes `mMin.y` and `mMin.x` together, then
> `mMax.y` and `mMax.x` together.** +4 instructions.

You concluded this is compiler scheduling and not source-addressable. **It is
source-addressable, and we closed a 549-word function on exactly this axis
today.** Two findings, both measured:

1. **A two-argument constructor is a codegen lever, not a style choice.**
   `mVec2_c v(x, y)` evaluates its argument list with both values live and
   **interleaves** their computation. Memberwise assignment (`v.x = ...;
   v.y = ...;`) runs each to completion, **serially**. Your target is serial and
   your draft is interleaved — so retail almost certainly used memberwise
   assignment where we are using a constructor or a `set()`, and the fix is to
   stop building these as aggregates.

2. **Group operations per COMPONENT, not per operation.** Finishing `x` entirely
   and then `y` entirely measured 0 diffs on the function where this was found;
   interleaving as `x += r; y -= r; x -= 16; y += 16;` measured 6. Same
   operations, same count, different grouping.

Your other two line items look like the same cause: the explicit `stfs` of zero
to `mMin.z`/`mMax.z` that retail has and we replace with a later dead store
suggests retail writes `.z` **explicitly, in place, as part of the same serial
sequence** rather than getting it from an aggregate initialisation.

So for round 21, item 1 is: **rewrite the `mMin`/`mMax` construction as plain
serial member assignments, per component, with `.z` written explicitly**, and
measure. If that closes the +4 and the two `.z` items, the `neg`+`add` versus
`subf` difference is the only piece left and is worth its own look.

### One correction to your reasoning

You wrote that reordering declarations "may not be addressable" for the
`mMin`/`mMax` grouping, citing the note that declaration order does not drive GPR
assignment. **That note is about which register the allocator picks. It says
nothing about the ORDER IN WHICH STATEMENTS ARE EMITTED**, which is what you are
actually trying to change here. Those are different questions and the note does
not apply. (For the record the note has also been split today: it stands for
GPRs, and is retired for floating-point registers, where declaration order
genuinely does drive the assignment — which is what closed your `execute`.)

---

## Round 21 — three items

### 1. `ProcMain` (179 / 170, −9) — serial member assignment

Covered above. Rewrite the `mMin`/`mMax` construction as serial per-component
member assignments with `.z` written explicitly. Report the word count and the
per-item table again after the change so we can see which of your five line items
moved.

### 2. `createObjList` (116 / 111, −5) — chase the `extrwi`

You have localised this well and your signedness reading is the right instinct:
retail's `extrwi` is an unsigned extract; our `srawi`+`clrlwi` is the signed path.
You noted that `x1`/`y1` must stay `int` for the `fctiwz` conversion but that the
shift-and-mask wants unsigned, and that a straight `u32` cast did not produce it.

Try separating the two roles into **two different variables** rather than casting
one: keep the `int` that receives the conversion, then assign it into a separate
`u32` (or `u16`) local that the shift-and-mask uses. A cast at the use site does
not change the variable's declared type, and MWCC picks the shift from the type,
not from the expression. That is 2 of your 5 words if it works.

Do not spend the round on the `_savegpr_17` versus `_savegpr_19` difference — the
GPR note genuinely does apply there.

### 3. `initialize` — closed, do not revisit

Agreed and confirmed: `l_object_name` versus `SYM0` is a naming gap in an unsplit
data section, every byte matches. **Treat it as matched.** I will fix the
tooling. Do not spend time on it again.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size for
both, then match status — length and save level first, as before.

Keep applying the arithmetic check before attributing any gap: **if a proposed
cause does not change the instruction count, it cannot explain a length
difference.** It has now caught two of your attributions and one of mine.

And when you find yourself about to write "this is compiler scheduling, not
source-addressable" — check whether you are talking about *which register* or
*what order the statements are emitted in*. The first is sometimes genuinely out
of reach. The second has been reachable every single time it has been tested
properly today.
