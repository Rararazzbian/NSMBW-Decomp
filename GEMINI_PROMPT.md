# Work order — round 21

**Read `AGENT_CONTEXT.md` first.** Several sections are new since your last read.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 20 — the two things I asked for, you did

**`setBeginMoveState` is fixed.** Fourth time of asking, and it is done. Verified
independently: 38 instructions, byte-identical, `stw r0, 0x848(r31)` on both
sides. Thank you.

**Both round-19 defects are genuinely fixed.** `mUnkACC(1.0f)` in the
constructor, which now matches; and `KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c`
has a real body again and matches. A full declared-versus-defined sweep found 56
undefined methods on the main class, but **none of them was previously
matching** — they are legitimately unwritten, not new silent deletions.

**Your headline reproduces exactly: 180/251 and 12,840/31,876 = 40.28%.** And
`poolcheck.py` reproduces exactly too — 78 constants, 0 mismatched, 0 unresolved.
Running it unprompted was the right call.

---

## The reporting is not reliable, and this round it misdirected both of us

The measurements your scripts produce are sound and reproduce every time. The
prose written around them is not being checked against them — and two of the four
errors below are contradicted by a different section of your own document.

### 1. The round-19 baseline you state is not traceable to anything

You give it as **9,712 bytes (30.47%)**. The correct figure was handed to you in
the previous prompt, in bold: **11,136 mechanical, 10,620 true**. A search of your
entire workspace for `9712` / `30.47` finds **zero hits** — not in your scripts,
not in any output — and your own `tool.py` has no code path that can produce a
round-19-only byte figure at all.

**Consequence: your headline "+3,128 bytes, +9.81%" is inflated.** Real progress
is **+1,704 bytes** mechanically, or **+2,220** true-to-true. That is still a good
round. Please report the real number.

**Do not state a comparison figure you have not computed this round.** If you do
not have last round's number to hand, recompute it or say you cannot.

### 2. Four of your seven GAINED names are wrong — and two real gains are missing

The count of 7 is right by coincidence: two wrong entries cancel two omissions.

| you listed | actually |
|---|---|
| `initializeState_ShellAtk_St` | already matched in round 19 — double-counted, same pattern as last round |
| `executeState_ShellAtk_St` | **not matched** — 2 real diffs. It appears in your GAINED table *and* at #2 in your own Top-20-Unmatched list, in the same report |
| `executeState_DieFumi_St`, `initializeState_FumiHit`, `initializeState_DieFumi_St`, `postExecute`, `setBeginMoveState` | correct |
| *(not listed)* | `__dt__20KokoopaSpFumiCheck_cFv` — a real gain |
| *(not listed)* | `executeState_FumiHit` — a real gain, byte-identical |

The `executeState_ShellAtk_St` diffs are genuine, not naming noise:

    lis  r3, SYM2@ha        |  lis  r3, "@LOCAL@...l_bounceSpeed"@ha
    addi r3, r3, SYM2@l     |  addi r3, r3, "@LOCAL@...l_bounceSpeed"@l

Different relocation targets. **A function with any diffs is not matched** — do
not put one in a GAINED set with a diff count attached.

### 3. `executeState_FumiHit` already matches — your section 4 says otherwise

You describe it as "down to 1 diff", caused by `sStateStateMgr_c::executeState`
sitting at slot `0x20` instead of `0x1C`. Measured: **108/108 instructions,
raw-byte identical.** No residual, and no slot problem in it.

### 4. `setBeginMoveState` is fixed — but it is not 16 bytes

You report it as "16 B / 4 instructions". Its real size is **0x98 = 152 bytes, 38
instructions**, as it has been every round. The fix is real; the description is of
some other function.

---

## Your `__sinit` slot map is wrong — and so was my reply to it

Last round I told you the `+0x90` finding was "the correct shape" and looked like
surplus interface vtables. **I checked it myself and it is not supported on any
checkable point. My endorsement was wrong.** Here is the measurement.

| your claim | measured |
|---|---|
| retail vtable spans `0x80314360`..`0x803149F0` = `0x690` | `__vt__18dEnTorideKokoopa_c` is **`0x5E4`**, ending `0x80314944` (`bin/dtk/wiimj2d_symbols.txt:19123`) |
| draft vtable is `0x600` | your own object reports **`0x5E4`** for the same symbol — **the two vtables are the same size** |
| slots 374..409 are 36 interface slots / "retail internal thunks" | those bytes in `original/wiimj2d.dol` are **`0xAC` of pure zeros** — every word, dumped and checked |
| the delta is `+0x90` (36 slots) | the uniform delta across all 196 offset diffs is **`0x80`** (128 bytes, 32 words) — retail `addi r5, r28, 0x690` against draft `addi r5, r28, 0x610` |

**What is actually true.** Retail has `0xAC` of zero bytes between the end of the
vtable and the first `sFStateID_c` (`@76840`, at `0x803149F0`); your draft has
`0x2C`. The shortfall is `0x80` of **zero-initialised `.data` sitting after the
vtable object**, not inside it. This is **not missing virtual methods.** Adding 36
base-class virtuals would have been the wrong fix, and my reply would have cost
you a round.

**Round 21, item 1: find the missing file-scope objects.** 128 bytes of
zero-image `.data` between the vtable and the state-ID array means retail declares
file-scope objects your draft does not. A runtime-constructed static has a zero
static image, so it is invisible in the DOL bytes and has to be found
structurally:

- list every `.data` object your compiled object emits, in order, with sizes;
- list what retail has over the same span from `bin/dtk/wiimj2d_symbols.txt`;
- align the two and find where the 128 bytes go missing.

Your own `setQuakeDead` note mentions a `static const sDeathInfoData
l_death_data` you have not written — that class of object is exactly what to look
for. **Report what you find. Do not add virtuals to any base class.**

This is worth doing carefully: `__sinit` is 5,784 bytes, the single largest
unmatched function in the unit, and everything else about your analysis of it —
196 uniform diffs, one repeated cause, 4 explainable as tooling noise — holds up.
Only the attribution was wrong.

---

## A real but smaller point: stop shadowing `include/game/sLib/*`

Separately, and **no longer offered as the cause of any specific residual**,
because the residual I attributed to it does not exist: your tree carries its own
copies of

    s_StateStateMgr.hpp   s_StateMgr.hpp   s_StateID.hpp   s_StateInterfaces.hpp

differing from the real ones by 132, 59, 41 and 73 lines. The real
`s_StateStateMgr.hpp` was corrected against retail and verified alone; your copy
reorders its virtuals and adds two the real one lacks (`isState`,
`getMainStateID`).

Since `executeState_FumiHit` already matches, this is latent rather than active.
Treat it as hygiene: **switch to the real headers, re-measure, report any function
whose status changes.** If one genuinely will not compile in your context, name it
and give the error rather than shadowing it silently — shadowing a shared header
discards work already verified against the binary, and neither of us can see it
happening from a per-function diff.

---

## Round 21 — the rest

Work in `scratch/gemini_round21/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build would destroy that.

### 2. The genuine near-matches

- `executeState_ShellAtk_St` (2 diffs) — the `l_bounceSpeed` relocation above.
  Your draft references a differently-scoped symbol than retail's; retail's is a
  file-scope local, yours resolves through a different name. Worth 612 bytes.
- `executeState_AttackSearch` (2 diffs) — the `searchBaseByID` ternary. Try
  hoisting it into a named local of the callee's parameter type so the argument
  register is fixed before the call, rather than the null arm being materialised
  into the return register.
- `initializeState_Jump` / `initializeState_BigJump` (7 diffs each) — the
  `isNonDamage`/`isOneDamage` branch polarity. These two are mirrors, and
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

- The `.data` object alignment for `__sinit`: what is missing, and how derived.
- The byte-baseline corrected, computed this round.
- GAINED and LOST by name — **read the names back against your own matched-set
  output before writing them down.**
- Ranked unmatched list, before and after.
- Per function: **draft size first**, then target size, then status. A `0 B` draft
  is unwritten, not mismatched.
- `poolcheck.py` output.

One standing caution, since it applies to `__sinit` directly: **a high score does
not mean landable.** I had a unit at 98.7% that broke all five binaries on
landing, because the scoring tools never run the linker and cannot see an
undefined symbol, a weak symbol we place that retail takes from elsewhere, or a
wrong section order. Your missing 128 bytes of `.data` is exactly that class of
problem.
