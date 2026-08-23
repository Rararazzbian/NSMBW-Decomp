# Work order — GXStateSave_c, closing round

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** in the repository root (overwrite it).

---

## You are at 2 of 4, and the two misses are in excellent shape

    __ct       DIFFS 0
    __dt       DIFFS 0
    save       DIFFS 11    62 words / frame 0x10 / GPR [30,31]
    restore    DIFFS 10    75 words / frame 0x10 / GPR [31]

I verified this by recompiling from your source myself. **On both misses the
word count, the stack frame size and the callee-saved register set are already
exactly right.** That means the control flow and the register allocation are
correct and only individual instruction operands differ. That is the cheap kind
of remaining work, and it is a real recovery from last round's zero.

Your derived layout table was the right approach and is mostly correct.

### One thing to fix about your reporting

Your report's headline read:

    **DIFFS 0 out of 4**

and the sentence underneath it correctly said *"Two of four functions are
byte-identical."* The headline is the number I act on. Write the true count —
**2 of 4** — even when it is not the number you want. An inflated headline costs
more than a miss does, because I have to re-derive the truth before I can use
anything else in the report. Your prose was honest; make the headline match it.

---

## Three fixes. Apply all three; they do not overlap.

### Fix 1 — the array is five elements too long. This is the big one.

`d_gx_state_save.hpp` declares:

    GXVtxAttrFmtList mVtxAttrFmt[32];

`GXVtxAttrFmtList` is 16 bytes, so 32 elements is 0x200 and every member after
it sits **0x50 (80) bytes too low**. That single mistake produces almost every
offset diff in both functions.

The correct count is `GX_VA_MAX_ATTR + 1` = **27**. `GX_VA_MAX_ATTR` is 26 in
`include/lib/revolution/GX/GXTypes.h`. Note your own `mVtxDesc[27]` is already
right — the two arrays are parallel and should have matched.

    - GXVtxAttrFmtList mVtxAttrFmt[32];
    + GXVtxAttrFmtList mVtxAttrFmt[27];

27 x 16 = 0x1B0, so `mVtxDesc` lands at 0x1B4, which is what the target shows.
That confirms the rest of your table:

    mMask        +0x000      mProjection  +0x28C      mCullMode    +0x2D0
    mVtxAttrFmt  +0x004      mViewport    +0x2A8      mColorUpdate +0x2D4
    mVtxDesc     +0x1B4      mScissor     +0x2C0      mAlphaUpdate +0x2D5
                                                      mDither      +0x2D6

### Fix 2 — one wrong mask bit in `restore`

    target:  rlwinm. r0, r0, 0, 27, 27      tests mMask & 16
    draft :  rlwinm. r0, r0, 0, 24, 24      tests mMask & 128

Your `restore` guards the cull-mode branch with `128`, but your own `save`
correctly uses `16` for the same field. `128` is the bit you use for
`AlphaUpdate`, so two different fields are colliding on one bit.

    - if (mMask & 128) {
    + if (mMask & 16) {
          GXSetCullMode(mCullMode);

### Fix 3 — seven calls must go through the `EGG::StateGX` wrappers

Seven `bl` targets in the retail code are not the plain GX SDK functions. They
are cache-aware wrappers in `EGG::StateGX`:

    save     GXGetScissor___Q23EGG7StateGXFPUlPUlPUlPUl
    restore  GXSetProjectionv___Q23EGG7StateGXFPCf
    restore  GXSetViewport___Q23EGG7StateGXFffffff
    restore  GXSetScissor___Q23EGG7StateGXFUlUlUlUl
    restore  GXSetColorUpdate___Q23EGG7StateGXFb
    restore  GXSetAlphaUpdate___Q23EGG7StateGXFb
    restore  GXSetDither___Q23EGG7StateGXFb

**Read those mangled names carefully — there are THREE underscores, not two.**
The mangling is `<name>__<scope><args>`, so the name itself is
`GXSetColorUpdate_` **with a trailing underscore**. Declare them exactly like
this or the mangled name will not match and the diff will not close:

    namespace EGG {
    namespace StateGX {
        void GXGetScissor_(u32 *x, u32 *y, u32 *w, u32 *h);
        void GXSetProjectionv_(const f32 *mtx);
        void GXSetViewport_(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ);
        void GXSetScissor_(u32 x, u32 y, u32 w, u32 h);
        void GXSetColorUpdate_(GXBool update);
        void GXSetAlphaUpdate_(GXBool update);
        void GXSetDither_(GXBool dither);
    }
    }

Put that in **your own shadow header** under
`scratch/qwen_gx/shadow/`, not in `include/lib/egg/`. I promote headers, you do
not. Then prefix the seven call sites with `EGG::StateGX::` and the trailing
underscore.

**The asymmetry is real, do not "fix" it.** `GXSetVtxDescv`, `GXSetVtxAttrFmtv`,
`GXSetCullMode`, `GXGetVtxDescv`, `GXGetVtxAttrFmtv`, `GXGetProjectionv`,
`GXGetViewportv` and `GXGetCullMode` already call the **plain** SDK functions
and already produce no diff. Leave those alone. Only the seven above are
wrapped.

**You do not need to worry about linking.** Those seven are not decompiled yet,
so they have no address at link time — I have already looked them up and I will
add the `syms.txt` entries myself when I land this. Do not touch `syms.txt`.

---

## Also worth cleaning up

Your `restore` reaches the three flag bytes with a raw cast:

    reinterpret_cast<unsigned char *>(this)[0x2D4]

Once Fix 1 is in, the named members are at the right offsets, so write
`mColorUpdate`, `mAlphaUpdate`, `mDither` instead. It should generate the same
instruction. If it does not, say so and keep whichever matches — but try the
named form first, because a raw offset cast is a sign the layout is wrong, and
after Fix 1 it is not wrong any more.

## Acceptance

- Apply all three fixes, rebuild, and re-score with:

      python tools/auto_decomp/fndiff.py scratch/qwen_gx/target.txt scratch/qwen_gx/NAME.txt --all

- **The true count at `DIFFS 0` out of 4 as your headline.**
- For anything still missing: the `-v` output for that function, words / frame /
  GPR / FPR both sides, and one sentence on what you think is left.
- A landing readiness statement.

If a fix does not do what I said it would, **tell me that plainly** — a fix I
predicted that did not work is useful information, and I would rather have it
than a report that quietly works around it.

Do not run `ninja`, `configure.py`, `progress.py` or `land.py`. Work only in
`scratch/qwen_gx/`. Do not modify `syms.txt`, `include/**` or `source/**`.
