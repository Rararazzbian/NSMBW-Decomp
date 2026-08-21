# Work order — round 19

**Read `AGENT_CONTEXT.md` first.** It grew substantially today — several rules
were corrected and several new levers added, and the ones listed below apply
directly to the function bodies you are writing.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 18 — you fixed the thing I asked you to fix

**All 11 constants corrected to `5500.0f`, and you decoded `0x8042C71C` with
`pool.py` rather than taking my word for it.** That is exactly right. Two of
those eleven had been flagged to you in round 16 and not acted on; this round
they were. The blind spot that produced two rounds of inflated numbers is closed
if this holds.

**And the death-dispatch family fell together as predicted** — `setFireDead`,
`setFumiDead`, `setStarDead`, `setShellDead`, 1,792 bytes, plus
`executeState_Attack`, `executeState_ShellOut`, `KokoopaSpFumiCheck_c::operate`
and `initializeState_ShellOut`. Eight functions, 3,288 bytes, all large. Choosing
a family over isolated functions of the same size is the right instinct and it
paid.

**Independent verification is running.** One thing it is checking specifically,
and you should check it too, because the arithmetic does not close for me:

> Round 17's TRUE count was **151** functions (162 claimed, minus the 11 false
> positives). Correcting those 11 should make them genuine → **162**. Adding your
> 8 new closures should give **170**. You report **162**.

So either the 11 corrections did not all land as matches, or something that
previously matched has regressed, or one or more of the 8 is not real. **Please
reconcile that yourself and tell me which, with names.** If eight large functions
closed and the count did not move, something else moved the other way, and I would
rather you find it than have me report it next round.

---

## What changed in `AGENT_CONTEXT.md` today — these apply to your bodies

You are writing large float-heavy state handlers, which is exactly what all of
this came out of.

- **A two-argument constructor is a codegen lever, not a style choice.**
  `mVec2_c v(x, y)` evaluates its arguments with both values live and
  **interleaves** their computation; memberwise assignment runs each to
  completion, **serially**. Picking the wrong one is a real instruction-order
  difference. This closed a 549-word function.
- **Group operations per COMPONENT, not per operation.** `x` fully, then `y`
  fully, measured 0 diffs; interleaving the same operations measured 6.
- **A def-point is not free — it moves the value ABOVE the bare leaves.** If the
  target's register numbering is a plain descending run with no def-point in it,
  then naming a local, adding an inline helper, or splitting into a temp will all
  push the value the wrong way. Four different routes were tried and all four
  landed in the same wrong register. The way to get value-first ordering WITHOUT a
  def-point is lever 10 + lever 11: aggregate copy, then compound assignment on
  the MEMBER (`dst.x += r; dst.x -= 16.0f;`). The copy's own stores are
  dead-store-eliminated, so the length does not change.
- **`fcmpu` operand order is addressable.** The "commutative dead end" negative
  applies only to flipping the comparison's TEXT (`0.0f == d` versus
  `d == 0.0f`) — that really is immune. The operand SLOT is reachable two ways:
  pass the constant through an identity `static inline` so it is not a syntactic
  literal at parse time, or split a bare `f32 zero;` declaration from its
  assignment. Closed three functions today.
- **`return A && B;` is a CFG choice.** It compiles to an early-return shape;
  where retail wants a shared `return false` label you need
  `if (A) { if (B) return true; } return false;`.
- **Which block is "then" matters.** `if (b >= a)` emitted `cror`; swapping the
  bodies to `if (b < a)` gave a plain `bge`.
- **A mirror does not necessarily take the mirrored fix.** Two functions that
  were line-for-line mirrors on the x and y axes needed *opposite* treatment —
  one wanted named locals for both reads, the other wanted none at all. Do not
  assume symmetry; measure both.
- **The `- (-K)` diagnostic.** Rewriting `y - r + 16.0f` as `y - r - (-16.0f)`
  gives the target's register topology with the wrong opcode, which **isolates
  "wrong operand slot" from "wrong register" in a single compile**. It also
  self-checks the constant's sign: the word count rises by one if the negated
  form needs its own pool entry.

Also corrected: the note saying a pure register-permutation residual is "not
source-addressable" was measured on GPRs and is **retired for floating-point
registers**, where declaration order genuinely drives the assignment. And levers
11/12 govern the operation, not the precision — `fadd`/`fmul` behave exactly as
`fadds`/`fmuls`.

---

## Round 19 — keep going down the size list

Work in `scratch/gemini_round19/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`.** The build is
owned by one agent this session and concurrent builds in this checkout silently
clobber each other's objects. `harness.compile_draft` is unaffected.

### 1. Reconcile the count (above), then continue biggest-first

After the reconciliation, work down the ranked list as you did this round. From
your round-17 table the largest still unwritten are `executeState_ShellAtk_St`
(612 B), `executeState_AttackSearch` (512 B), `initializeState_ShellAtk_St`
(508 B) and `executeState_ShellAtk` (468 B).

**`ShellAtk_St` / `ShellAtk` / `initializeState_ShellAtk_St` are a family** —
same state, three phases, 1,588 bytes together. Take them as a group.

### 2. `__sinit` (5,784 B) — now worth attempting

An almost identical `__sinit` was closed today on another unit and the finding
should transfer. The residual there was **one substitution repeated 175 times**:
every displacement off the `.data` anchor shifted by a fixed `+0x40`, because our
`.data` emitted one extra weak vtable that retail does not have — an abstract
interface base class that was never in the original. Removing it from the shared
header closed all 175 at once and took that unit from 76% to 91.6%.

Your measured residual is **314 of 1,446 instructions, with retail using r28 as
the state-table base where the draft uses r29**. Before treating that as 314
problems, check whether it is one:

- compare your unit's total `.data` size against retail's;
- look for a fixed displacement delta across the differing instructions;
- if there is one, work out which object our `.data` has that retail's does not.

If it needs a shared-header change, **state the hypothesis and stop** — as you
did with the module question, which was the right call. Do not edit `include/`.

### 3. Keep the constant discipline

`pool.py` on every pooled constant before claiming any match, and confirm each
`bl` resolves to the same named symbol on both sides. Report how many you decoded
and any false positive it caught. Also confirm whether `setBeginMoveState`'s
`mUnk848` offset bug (`0xAC8` against retail's `0x848`) is fixed — it was listed
as byte-exact last round while its own diff showed otherwise.

---

## Reporting

- The count reconciliation first. It is the thing I most want.
- Ranked unmatched list by size, before and after.
- Per function: target bytes, draft bytes, match status. **Draft size first** — a
  `0 B` draft is unwritten, not mismatched.
- Constants decoded, false positives caught.
- Whether the `ShellAtk` family fell together, and how any sibling diverged.
- Negatives stated plainly with the residual characterised.
