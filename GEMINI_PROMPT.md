# Work order — round 34

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 33: two good calls and one verified fact

**The twins are at 5 diffs** in the object, down from 6, folded and confirmed by
my own scoring. Good.

**I checked your death-info literal against `original/wiimj2d.dol` byte by
byte.** At `0x802F0C40`:

    00000000 40400000 c0800000 be400000 80357af4 ffffffff ffffffff 00ff0000
    -> 0.0f, 3.0f, -4.0f, -0.1875f, ptr 0x80357AF4, -1, -1, 0, 0xFF

which is your literal exactly. And `0x80357AF4` really is
`dEnBoss_c::StateID_DieStar`: `setStarDead`'s own pool entry is `@70590` at
`0x802F0C20`, it contains the same pointer, and `setStarDead` matches
byte-exact. So `setQuakeDead` genuinely uses the star death state, and the two
entries differ only in that trailing `0xFF`. Reconstructing a 32-byte struct
from the binary and getting every field right is exactly the right method.

You also restored `nullptr`, which put the object back to 88 words / 80 diffs.
That is the correct trade and I asked for it: an honest 80 beats a fake 1.

---

## The entire function now matches except one allocator decision

I ran a proper alignment-aware edit script instead of a positional diff.
**78 of 85 instructions are identical.** Here is every difference, complete:

    stwu r1, -0x30      vs  -0x40                 frame
    (nothing)           vs  stw r29, 0x34(r1)     save
    li   r0, 0x0        vs  li   r29, 0x0         the store constant
    sth  r0, 0x792      vs  sth  r29, 0x792
    sth  r0, 0x790      vs  sth  r29, 0x790
    li   r3, 0x0        vs  (nothing)             the null arm
    cmpwi r3, 0x0       vs  mr r29, r3 / cmpwi r29, 0x0
    (nothing)           vs  mr r3, r29
    (nothing)           vs  lwz r29, 0x34(r1)     restore
    addi r1, r1, 0x30   vs  0x40

That is it. Head matches, tail matches — 32 consecutive instructions of the
death-info copy are identical. **Every remaining difference is downstream of one
choice: MWCC put the zero for the two `sth` stores into callee-saved `r29`
instead of volatile `r0`, and then reused it as the conditional's merge
register.**

So there is nothing structurally wrong with the function any more. The source is
right. What is left is finding the source nudge that makes MWCC use `r0` for a
constant that dies before the call, exactly as it does in `setShellDead`.

Your own structural observation from last round is the best lead and I want you
to push on it rather than restate it: in `setShellDead` the score call is
conditional and `r29` is occupied by `killedBy`; in `setQuakeDead` the call is
straight-line and `r29` is free. **But retail's `setQuakeDead` saves only
`[30,31]`, so `r29` is free in retail too and it still uses `r0`.** That is the
contradiction to resolve. Something about our function makes the allocator think
the constant is worth promoting, and retail's does not.

Things not yet tried:

- **`register` storage class** on `base`, or on a local holding the zero. MWCC
  honours it more than modern compilers do.
- **A named local for the zero used only by the stores**, declared in a nested
  scope `{ }` that closes before the call, so its live range is syntactically
  bounded.
- **Widening or narrowing the store type.** Check the retail struct: are
  `mUnk790` and `mUnk792` really two independent `s16`s, or one field the header
  has split? Two `sth` from one source expression is a different tree from two
  `sth` from two statements.
- **The `mAnmMatClr` block at the top.** In `setShellDead` that check is at the
  *end*, after the ternary; in ours it is at the *start*. The retail listing has
  `bl setFrame` early, so early is right — but confirm it, because it is the
  other ordering difference between the two functions.

I have my own negatives to save you time. **Ruled out, do not re-run:** chained
and reversed zero stores, `mUnk790 = mUnk792`, an `s16` zero local, `(u16)`
casts, `(fBase_c *)0`, `static_cast`, declaration-in-condition, `if/else` with
an empty then-branch, and a `static inline` helper in three forms. All give the
same 88/`0x40`/`r29`. One shape does move the merge: a null-initialised local
plus a plain `if` reaches **86 words, frame `0x30`, GPR `[30,31]`** with the
merge in `r0` — saved at `scratch/claude_kokoopa/q5_null_then_if.cpp`. It is one
word over because it emits `li r0,0` plus two `mr`s instead of `li r3,0`. That
is the closest honest shape anyone has, and it is one instruction from the
target in a different direction from your `>> 31`. **Read its listing.**

---

## Completeness

Both items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 34 — order of work

### 1. `setQuakeDead`: get the store constant into `r0`

That is the whole task. Every other difference follows from it. Work from the
four untried directions above and from `q5_null_then_if.cpp`.

For each variant report **which register holds the store constant**, which holds
the merge, and whether `li r3, 0` is present. Those three facts are the score
this round — not the diff count.

**Acceptance:** ≥5 variants, none from the ruled-out list, with those three
columns plus words and frame.

### 2. The twins: the last inversion

Still 5 diffs, still the same single question — retail allocates `speed.x`
before the two registers the int-to-float conversion consumes, and the draft
allocates it after. You have confirmed `l_EnMuki` is `extern const s8` and that
`mpParamJump` members are read directly with no pool entry, so both of those
levers are spent and correctly spent.

What remains is the *provenance* of `speed`. It is an `mVec2_c` assigned in one
of two branches. Untried: making it a `const` reference or pointer to the chosen
`mpParamJump` member instead of a copy, so there is no stack `mVec2_c` at all
and the loads come straight off the parameter struct. The target loads from
`0x10(r1)` and `0x14(r1)`, so a stack slot does exist — but a pointer-selected
source may still change the allocation order.

**Acceptance:** ≥4 variants, both twins scored each time, with the observed
f2/f3/f4 allocation for each.

### 3. Fold and verify

Fold the best of each, rebuild, `fndiff.py --all`, `poolcheck.py`.

---

## Constraints

Continue in `scratch/gemini_round24/`. You may read `scratch/claude_kokoopa/`
but do not write there. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`, `CODEX_HANDOFF.md`,
or `HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or
`land.py`**, and do not attempt the landing.

---

## Reporting

- **Lead with your best number.**
- `setQuakeDead` table: variant, store-constant register, merge register,
  `li r3,0` present, words, frame.
- Twins table: variant, both counts, f2/f3/f4 allocation.
- `fndiff.py --all` after folding, pasted whole.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- `poolcheck.py` output.
- `NOT REACHED`, if anything.
