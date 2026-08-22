# Work order — round 23

**Read `AGENT_CONTEXT.md` first.** Three new sections went in from your round 22,
one of them a retraction of something I told you last round.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 22 is a real round, and your headline is substantially honest

This is the first time in five rounds I can write that sentence, so it goes
first. I scored your round-21 and round-22 objects side by side, same gate, same
run:

    round21: 180/251 fns, 12840/31876 bytes
    round22: 212/251 fns, 19620/31876 bytes

**And two of the four functions my gate calls a miss are my gate's fault, not
yours.** Both are symbol-naming artifacts of my canonicaliser, and in both cases
your prose was right:

- **`executeState_ShellAtk_St` (612 B) — 2 diffs, and they are `SYM2` against
  `l_bounceSpeed`.** This is the identical unnamed-`.rodata` case I already ruled
  a match in the round-22 brief. My canonicaliser anonymises retail's unnamed
  label but keeps your named symbol, so it manufactures a diff. It matches.
- **`__sinit` (5,784 B) — 4 diffs out of 1,446 lines**, and all four are one
  consistent `SYM` renumbering around `__vt__18dEnTorideKokoopa_c`. Structurally
  identical, instruction for instruction.

Count both and you are at **214 fns / 26,016 bytes** against your claimed 216 /
26,668 — within two functions and 652 bytes. Your `__sinit` closure is real.

**General rule, now in `AGENT_CONTEXT.md`:** a diff where one side is `SYM<n>`
and the other is a named local symbol is a **naming artifact, not a code
difference**. Flag those and move on; do not spend a round chasing them.

---

## You did not report LOST. Fifth round, and this time it cost you 12 functions

Your report has a GAINED section and no LOST section. The real delta is:

    GAINED: 44 functions, 7,768 bytes
    LOST:   12 functions,   988 bytes
    NET:    +32 functions, +6,780 bytes

The twelve you regressed:

    516  __ct__18dEnTorideKokoopa_cFv
    400  executeState_ShellOut__18dEnTorideKokoopa_cFv
     16  finalizeState_QuakeHit__18dEnTorideKokoopa_cFv
      8  getTorideFunfareTime / getShellChangeEffectOffsetY / getJumpGravity
      8  checkGetUp / getDownTime
      4  speedUp / finalizeState_DieFumi_St / ikakuSE / awakeSE

I am not going to relitigate the reporting point — you have heard it four times.
What matters is that **all twelve trace to the header and vtable edits in your
section 4**, and I can show you the shape. There are exactly three classes:

**1. You wrote bodies for functions that are empty in retail.**

    awakeSE   target: blr                    (1 instruction, 4 bytes)
              draft:  lwz r12, 0x544(r3) ... (8 instructions)

You declared `virtual void awakeSE();` at slot `0x5DC` and `ikakuSE` at `0x5E0`,
gave them bodies, and then **listed both as 32-byte matches in your GAINED item
35**. Retail's are 4 bytes and do nothing. That is the same error class as the
`removeCc` one from last round, running in the other direction: you inferred a
method's content from its slot rather than reading it.

**2. You emptied a function that has a body in retail.**

    finalizeState_QuakeHit   target: lwz r12, 0x60(r3) ... (4 instructions)
                             draft:  blr

**3. Two functions no longer exist in your object at all** — `getDownTime` and
`speedUp` produce no symbol. You renamed or dropped them.

And `executeState_ShellOut` is **one diff**, a branch displacement
(`beq 0x18` against `beq 0x2C`) — the code it jumps over changed size. That one
is nearly free.

`__ct__` is the priority: 516 bytes, and your draft is **528 bytes at 78 diffs**.
A constructor that grew by three instructions after you changed member offsets
and added `mUnk6A8` / `mpParamDemo` / `mAtkCnt` is telling you one of those
member edits is wrong. It is the cheapest possible check on your whole section 4.

---

## The pad is load-bearing, unlandable, and my `dDeathInfo_c` advice was wrong

I measured your pad both ways:

    with    u8 g_padData[128] = { 1 };   __sinit: 4 diffs   (i.e. matching)
    without                              __sinit: 200 diffs

So **the 128 bytes are real and they are doing the work.** Your layout analysis
was sound and I am not asking you to undo it. Two things follow.

**First, I owe you a retraction.** I told you the occupant was four
`dDeathInfo_c` — runtime-constructed objects that would contribute entries to
`__sinit`. **Your own result refutes that, and refutes it cleanly.** Your
`__sinit` matches retail instruction-for-instruction with an *inert* 128-byte
array in place. If the occupant were constructed at runtime, `__sinit` would
carry extra calls and the counts could not match. So the occupant contributes
**nothing** to `__sinit`: it is not a constructed object. Drop that line of
attack — it was mine and it was wrong. The rule it came from still holds (the
candidate must be zero before `__sinit` runs); the candidate did not.

Note also that your five `static const sDeathInfoData` objects are `const`, so
they land in `.rodata` and they hold relocations. Whatever they are doing for
you, they are not this 128 bytes.

**Second, `g_padData` cannot land.** It is a fabricated symbol, and `= { 1 }`
writes `0x01` into byte 0. I asserted last round that the region is all zeros. I
have not re-verified that myself and I am not going to assert it twice —
**you have the region located and I do not, so measure it and tell me.**

---

## Round 23 — order of work

Work in `scratch/gemini_round23/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green, all five binaries byte-exact, and a concurrent build destroys that.

1. **The twelve LOST, biggest first.** `__ct__` (516 B) and
   `executeState_ShellOut` (400 B) are 916 of the 988 bytes and both look cheap.
   Revert `awakeSE`/`ikakuSE` to empty bodies, restore `finalizeState_QuakeHit`,
   and find out where `getDownTime` and `speedUp` went. **Report what broke each
   one** — this is a diagnosis of your own section 4 header edits, and whichever
   edit did it is probably wrong for other functions too.

2. **Dump retail's 128 bytes and print them.** Byte 0 first: is it `0x00` or
   `0x01`? You have never reported the actual bytes of this region in five
   rounds of reasoning about it. Print all 128, hex, in your response. That one
   measurement decides what can replace `g_padData`, and no amount of further
   arithmetic will.

3. **Then propose a landable occupant** consistent with what you measured — zero
   or near-zero static image in `.data`, and **no constructor**, per the
   retraction above.

4. Remaining unmatched, biggest first, once 1–3 are done.

---

## Reporting

- Headline with round 22 recomputed under the same gate, so the delta is real.
- **A LOST section is mandatory this round, even if it is empty.** Read both
  lists back against your own tool output before you write them. If a function
  appears in your unmatched table it may not also appear in GAINED.
- Do not count symbol-naming diffs as misses (see the rule above), and say so
  explicitly where you apply it.
- Per function: draft size first, then target size, then status.
- `poolcheck.py` output.
- The 128 bytes, in hex.
