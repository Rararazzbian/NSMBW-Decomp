# Work order — round 22

**Read `AGENT_CONTEXT.md` first.** Two new sections went in because of your round
21, one of them correcting a rule I had given you.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 21 — you disproved my lever and found the right one

**`ProcMain`'s length gap is CLOSED, 179/179.** And the way you got there is the
part worth keeping.

I told you the fix was to stop building `mMin`/`mMax` as aggregates and write
serial member assignments. **You tested that first and it moved nothing** — a
three-argument `mVec3_c` constructor already serialises, so the rule I gave you
(which was measured on the *two*-argument `mVec2_c` case) does not extend to it.
You then found the shape that does work: **copy the aggregate, then compound-
assign the members.** That is levers 10 and 11 composing, and it closed all five
of your round-20 line items at once — including both `.z` stores, which you never
had to address directly, because copying a `pos` whose `z` is zero writes them.

I have corrected `AGENT_CONTEXT.md` accordingly. The generalisation now reads:
**when the target stores a base value and then modifies it in place, the source
shape is a copy followed by compound assignment, not a construction from
computed arguments.**

**`createObjList` reached `extrwi`,** 111 → 113. Your variant table is the reason
I can trust it: you included the *disproof* — `(u16)` cast at the use site,
measured, unchanged — which is what turns "this worked" into "this is the rule".
That table is now in `AGENT_CONTEXT.md` as a confirmed lever.

Both remaining gaps are correctly attributed. The `_savegpr_17`/`_savegpr_19`
difference is genuinely the GPR note's territory and I am not asking you to
fight it.

---

## Round 22 — finish this unit, then take a new one

Work in `scratch/round17/` as before (or move to a cleanly named directory of
your own — see the warning below). Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `GEMINI_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`.** The tree is
currently GREEN — all five binaries byte-exact, for the first time in about ten
days — and a concurrent build in this checkout would destroy that.
`harness.compile_draft` is unaffected and is what you have been using.

**A warning about your working file.** `scratch/round17/d_bg_actor_mng.cpp` has
twice been swept into unrelated commits by a concurrent `git add -A` — mine, not
yours. Nothing was lost, but if you want a snapshot you can trust, copy your
draft to a fresh path at the start of the round and work there.

### 1. `ProcMain` — the `neg`+`add` versus `subf` residual

Your own suggestion is the right next move and I would like you to take it:
reshape the preamble so `y0` is negated before the conversion, giving the loop a
ready-made negative operand so `add` does the work. You noted the statement-order
axis is still open here, and it is.

One caution learned the hard way this week: **an instruction-selection difference
that is count-neutral cannot be closed by anything that changes the count.**
Before adopting a variant, check the word count is still 179. A fix that closes
the `subf` and costs a word has not helped.

The `lwz`/`stw` versus `lfs`/`stfs` struct copy is the other same-count residual.
Retail copying `viewMin`/`viewMax` through the *integer* path suggests the
original copies a POD struct wholesale (`memcpy`-like, or plain `=` on a type
with no float members declared), where we are assigning float fields one by one.
Worth one variant: copy the whole struct in a single assignment rather than
field-by-field.

### 2. `initialize` — leave it

66/66, symbol naming only. Confirmed closed twice now. Do not spend time on it.

### 3. Then start a new unit: `dol/bases/d_bg_ctr.cpp`

`d_bg_actor_mng` is essentially finished — two same-count residuals and a GPR
difference that is off-limits. Rather than grind those, take the neighbouring
unit, which is unclaimed and which you are already primed for by having read
`d_bg.hpp`.

    class dBg_ctr_c, .text 0x8007F7A0 .. roughly 0x80081070
    include/game/bases/d_bg_ctr.hpp already exists

Seed it with:

    python tools/auto_decomp/prepare.py --unit dol/bases/d_bg_ctr.cpp \
        --range 0x8007F7A0-0x80081070

**Check the end boundary yourself before trusting it** — `prepare.py`'s own
docstring warns that a TU does not end at its `__sinit`, and getting the end
wrong has been this project's most common single error. The next class after it
is `dBgGlobal_c` at `0x80081070`. Confirm the last function in your `target.txt`
belongs to `dBg_ctr_c` and the next one does not.

Start with the largest functions, and report the ranked list before you write
anything so we can both see the shape of the unit.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size, then
match status — length and save level first, as before.

**New requirement, and it applies from this round on.** Report your matched set
as a **set difference against last round: GAINED and LOST, both by name.** Not a
net count. The other peer reported eight closures in one round while silently
breaking eight unrelated functions, two of which had their bodies deleted
entirely — a per-function diff cannot see a deleted body, because there is
nothing left to compare. Your round-21 table is already whole-unit, so this is a
small addition for you, but I want it explicit.

**Run the new constant checker before you report, every round:**

    python tools/auto_decomp/poolcheck.py <draft.cpp> <shadow_include> <target.txt>

Your unit was verified clean on constants last round, so this is insurance rather
than a correction — but the reason it exists is worth knowing, because it changes
what a "match" means. **Both halves of the match gate are blind to a wrong
pooled constant.** Raw bytes are blind because an `lfs`/`lfd` offset field is
zeroed. Canonicalised text is blind because it renumbers pool symbols by order of
appearance, so a draft loading `0.0f` against a retail `1.0f` produces the *same*
canonical text when both are the first pool reference in the function. The value
only exists in the binaries. The checker reads it from both sides and compares.

It found a false positive in my own unit that had been counted as matched for
days: retail loaded `(double)(-0.1f)` — bytes `BFB99999A0000000` — where the
draft had the exact double `-0.1`, `BFB999999999999A`. Both are `lfd`, both
assemble identically. **When a float literal is compared against a `double`, keep
the `f` suffix**; widening a float is not the same constant as the double.

Keep doing the two things that made this round work: **measure the variant you
expect to fail**, and **check the arithmetic before attributing a gap**.
