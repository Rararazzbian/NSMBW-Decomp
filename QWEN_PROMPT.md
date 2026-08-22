# Work order — round 25

**Read `AGENT_CONTEXT.md` first.** Two new sections came out of your round 24.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 24 — verified in full, and every number reproduced

I reproduced everything independently before writing this, and this is the
cleanest round anyone has turned in on this project. Stated plainly so it is on
the record:

**All eight bodies compile, and every figure in your table is exact.** I reran
your own `diff_ctr.py` against both round 23 and round 24 in the same session:

    round23:  MATCHED: 31  DIFFER: 8  MISSING: 0  (target 39)
    round24:  MATCHED: 31  DIFFER: 8  MISSING: 0  (target 39)

and the eight went from stubs to real bodies exactly as you reported:

| Function | target | r23 draft | r24 draft |
|---|---|---|---|
| `fn_80080900` | 256 | 2 | 208 |
| `fn_80080670` | 130 | 2 | 127 |
| `calc` | 125 | 1 | 138 |
| `fn_80080E40` | 121 | 2 | 124 |
| `fn_8007FFA0` | 115 | 1 | 114 |
| `addDokanMoveDiff` | 87 | 1 | 91 |
| `revisePos` | 72 | 1 | 72 |
| `fn_80080880` | 32 | 2 | 31 |

**You reported GAINED {} / LOST {} and that is correct** — I checked the matched
set by name, both units, and it is unchanged. You described a round that moved
906 words from un-compiled prose to measured bodies *without* claiming a single
function it did not close. That is exactly the reporting standard, and it is the
reason I can spend this round on your leads instead of on your arithmetic.

**Your nw4r geometry header is right where it matters.** Both segment symbols
are in the target disassembly verbatim:

    IntersectionSegment3Sphere__Q24nw4r4mathFPCQ34nw4r4math8SEGMENT3PCQ34nw4r4math6SPHEREPfPf
    DistSqSegment3ToSegment3__Q24nw4r4mathFPCQ34nw4r4math8SEGMENT3PCQ34nw4r4math8SEGMENT3PfPf

`Atan2Idx__Q24nw4r4mathFff` is also real, but note where it is: it is called from
**`fn_8007FFA0` and `addDokanMoveDiff`**, not from `fn_80080900`. `fn_80080900`
calls **`atan2s__2cMFff`** twice — and your draft already does that correctly, so
this is a note for your header's documentation, not a code defect.

---

## Your `calc` / `addDokanMoveDiff` fix is correct — and you must not write it

You proposed replacing `CosFIdx((f32)rot * (1.0f/256.0f))` with
`nw4r::math::CosIdx(rot)` / `SinIdx(rot)`. **Do it. It is right.** But you were
about to declare these yourself, and the tree already has them:

    include/lib/nw4r/math/math_triangular.h:41   inline f32 SinIdx(short idx) { return SinF(U16ToF32(idx)); }
    include/lib/nw4r/math/math_triangular.h:61   inline f32 CosIdx(short idx) { return CosF(U16ToF32(idx)); }

**Resolve the objection you would otherwise hit.** You may look at the target,
see `bl CosFIdx__Q24nw4r4mathFf`, and conclude that it therefore cannot be
`CosIdx`. That is a false dichotomy — `CosIdx` is **inline** and expands to

    CosIdx(idx) -> CosF(U16ToF32(idx)) -> CosFIdx(U16ToF32(idx) * (1.0f/256.0f))

so it emits `bl CosFIdx` too. Both spellings produce the same call. The call is
not the diff.

**The diff is the argument, and you already found it.** The `psq_l f1, 0x0(r3),
1, qr3` you spotted is `U16ToF32` inlining to

    include/lib/nw4r/math/math_arithmetic.h:113   inline f32 U16ToF32(u16 arg) { f32 ret; OSu16tof32(&arg, &ret); return ret; }

`OSu16tof32` is a GQR3 u16-dequantising paired-single load: it does the 16-bit
load *and* the `1/256` scale in one instruction, straight from the object field.
Your manual `(f32)rot * (1.0f/256.0f)` compiles to a load, a convert and an
`fmuls` instead. **That sequence is where your +13 and +4 words are.**

Here is the target's `calc` doing it, for confirmation:

    lha   r31, 0x0(r3)
    psq_l f1, 0x0(r3), 1, qr3
    bl    CosFIdx__Q24nw4r4mathFf
    psq_l f1, 0x0(r3), 1, qr3
    bl    SinFIdx__Q24nw4r4mathFf

Note it reads the same field twice with `psq_l` rather than keeping the converted
float alive — do not "optimise" that into one load.

These two are **mirrors**, and `AGENT_CONTEXT.md` records that a mirror does not
necessarily take the mirrored fix. Measure both separately.

---

## Round 25 — order of work

Work in `scratch/round25/`. Do not touch `wip/**`, `source/**`, `include/**`,
`slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or
`HANDOFF.md`. **Do not run `ninja`, `configure.py`, `progress.py` or `land.py`**
— the tree is green, all five binaries byte-exact, and a concurrent build
destroys that.

1. **`calc` (+13) and `addDokanMoveDiff` (+4)** — the `CosIdx`/`SinIdx` swap
   above. Highest confidence, cheapest, and it needs no new declarations. Do it
   first.

2. **`fn_80080880` (−1)** — your own next step is good: try
   `if (f3 + mScratch[lbl_802EFBD0[idx]].x < f2) return false; return true;`
   against the compound `return ... >= f2;`. One compile, and the target's
   `fcmpo`/`mfcr`/`extrwi`/`cntlzw`/`srwi` shape is a strong tell for the
   negated-if form.

3. **`revisePos` (72/72)** — same instruction count, so this is pure ordering and
   nothing structural is wrong. Your diagnosis (target reads `m_9C`, then `m_98`,
   then `m_94`; draft reads declaration order) is testable in one compile. Note
   this is the same lever that closed `set(sBgSetInfo)` in round 23: **named
   temporaries in the target's READ order.** Reach for it exactly that way.

4. **`fn_80080900` (−48, frame −0xb0)** — the biggest single gap left in the unit
   and worth the rest of the round. Read the prologue as a requirement, not an
   artifact:

       target:  stwu r1, -0x170(r1) ;  bl _savegpr_20   (r20..r31, 12 GPRs)
       draft:   stwu r1, -0xc0(r1)  ;  bl _savegpr_24   (r24..r31,  8 GPRs)

   The target needs **four more callee-saved GPRs and 0xb0 more stack**. That is
   more simultaneously-live locals, not different codegen for the same source —
   you are missing state, not mis-spelling it. Your SEGMENT3/SPHERE-on-stack
   reading is the right thread: the four-corner array and the `edgeSeg` struct
   are the candidates for the missing stack. Pull on that before touching
   register allocation.

5. `fn_80080670` (−3), `fn_8007FFA0` (−1), `fn_80080E40` (+3) — all three are
   register-file diffs (FPR saves where the target has GPR saves, or assignment
   order). Take them last; they are the least likely to move on a source edit.

`d_bg_actor_mng.cpp` needs no work this round unless something falls out for
free — 13/3/0 is where it should stay.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size, then
**diff count**.

**GAINED and LOST by name** against round 24, both units. You have done this
correctly twice running; keep it.

`poolcheck.py` before you report.

On the `psq_l` result specifically: report whether the `CosIdx` swap changed the
word count, **whichever way it went**. A refuted prediction that moves the number
is still information, and this one is load-bearing for two other units.
