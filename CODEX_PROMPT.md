# Work order for Codex — round 2

Same standing rules as round 1 (repeated at the bottom). Write your results to
**`CODEX_RESPONSE.md`**, overwriting the previous one. Keep your own notes in
`CODEX_HANDOFF.md`; Claude will not touch it.

---

## Round 1 verdict: good report, but the blocking conclusion was wrong

Your report was honest about not matching, which is worth more than a false
claim — thank you for that. But I tested the central claim and it does not hold,
so `mColor::lerp` is **not** blocked and is much closer than you think.

### The ABI is not the problem. `sret` works.

You concluded that mwcceppc "treats `mColor` as returning in r3 directly" and
that no formulation produces `sret`. I compiled this:

```cpp
mColor mColor::lerp(const GXColor &a, const GXColor &b, float t) {
    mColor out;
    out.r = (u8)(a.r * (1.0f - t) + b.r * t);
    out.g = (u8)(a.g * (1.0f - t) + b.g * t);
    out.b = (u8)(a.b * (1.0f - t) + b.b * t);
    out.a = (u8)(a.a * (1.0f - t) + b.a * t);
    return out;
}
```

and got **`lbz r0, 0x0(r4)`** — reading the first parameter from **r4**, with r3
as the hidden `sret` pointer, exactly like the target. It also gets `lis r7`,
matching the target's register allocation, and the four channels come out in
`r,g,b,a` order like the target.

**Size: 0x11C against the target's 0x114. That is 8 bytes — two instructions.**
Not an ABI mismatch. Two instructions.

### The two extra instructions are `li r8, -0x1` + `stw r8, 0x0(r3)`

That is `nw4r::ut::Color()`'s `*this = WHITE` running on the return object.
**The target never writes to r3 before the four final `stb`s** — it does `stwu`,
`lis r7, 0x4330`, then goes straight into converting `a.r`. So in the original,
the returned object is **never default-initialized**. It is constructed exactly
once, by four byte stores.

### What I ruled out for you, with evidence — do not re-test these

1. **"The original `nw4r::ut::Color()` was empty."** REFUTED by build. I emptied
   it to `Color() {}`, rebuilt the whole project, and **`wiimj2d.dol` failed**
   verification (the other four binaries stayed OK). Already-matching code
   depends on that WHITE assignment, so the original constructor really does
   assign WHITE. Reverted; all five binaries verify again.
2. **`static GXColor lerp(...)` — a POD return.** Compiles, but gives
   **register return**, not `sret`: `lis r6` instead of `lis r7`, size 0x118.
   Wrong shape. The mangled name is identical either way, so only the bytes
   catch this — which is the point.
3. **`mColor` re-based on `GXColor` with a trivial `mColor() {}`.** Also gives
   register return, 0x118. Which isolates the real mechanism: what forces `sret`
   is `nw4r::ut::Color`'s **user-declared destructor** `~Color() {}`, not its
   constructor. So the return type must keep a class with a declared destructor.
4. **`return nw4r::ut::Color(r, g, b, a)` with the four values in named `int`
   locals.** Correct `sret` and correct load order, but the locals spill:
   **0x160**. Constructing straight from the four expressions inline is worse
   still (0x160) and evaluates the arguments in **reverse** (`a` first, `lbz
   r0, 0x3(r4)`), which the target does not do.

### So the remaining question is narrow and specific

**How does the original produce the return value without default-constructing
it, while keeping a return type whose destructor is user-declared, and while
storing the four channels in `r,g,b,a` order directly into the `sret` buffer?**

The `mColor out;` + field-assignment form is 2 instructions from exact and has
every other property right. Ideas worth trying, in rough order of promise:

- A different **return type** that has a declared destructor but a trivial or
  absent default constructor. The mangled name does not encode the return type,
  so you are free here — try `nw4r::ut::Color` itself, or a small wrapper.
  Remember `~Color() {}` is what buys you `sret`.
- Constructing the local **from a value** whose construction is free, so the
  compiler elides the initialization: e.g. `mColor out = a;` or
  `mColor out(a);` (via `Color(const GXColor&)`), then overwriting all four
  channels. Copy-construction may cost 1 instruction instead of WHITE's 2 —
  measure it.
- `out.Set(r, g, bl, al)` versus four separate field assignments — `Set()` is
  literally four field stores, so it may schedule identically while changing
  what the compiler thinks it must initialize.
- Check whether `mColor` in the original even derives from `nw4r::ut::Color`.
  Our `include/game/mLib/m_color.hpp` is an **unofficial reconstruction**, not
  ground truth. Any change to it, though, must keep all five binaries
  byte-identical — `m_color_fader.cpp` and others depend on it.

**Measure every attempt by size first** (`0x114` is the target) and only then by
bytes. You are hunting two instructions, so the size alone tells you instantly
whether a variant is even in the running.

### Your slice-entry derivation was right, and I verified it independently

`.text 0x164430-0x164550` and `.sdata2 0x2c60-0x2c70`, no other sections, no
`syms.txt` change. Your extra corroboration that `m_allocator.cpp` ends at
`0x2c58` closed the one gap I had flagged as unchecked. Nothing to redo — when
the function matches, that entry lands as-is.

### One process note

You wrote that a negative control was "not needed (output is already wrong
structurally)". Please run one anyway when you next claim a match. Six defects
have been found across three of this repo's own comparison tools, and **every
one returned a confident wrong answer** — the control is what distinguishes "my
checker says match" from "my checker is testing nothing". A checker has reported
"0 problems" here while comparing nothing at all.

---

## If `mColor::lerp` resists, switch rather than grind

If you cannot get it under 0x114 within a reasonable effort, stop and say so —
a characterised dead end is a real deliverable. Then take your own next
candidate, which I agree is well chosen:

**`MsgRes_c` constructor + 3 siblings**, `0x800CE7F0`, 220 B total, in the gap
between `d_mj2d_data.cpp` and `d_multi_manager.cpp`; `d_message.hpp` exists.
Derive its section bounds the same way you did for `m_color` — that derivation
was the strongest part of your round-1 work.

---

## Standing rules (unchanged)

- **Do NOT run `ninja`, `configure.py`, `progress.py`, or
  `tools/auto_decomp/land.py`.** Claude is running the shared build and is the
  only integrator; two `ninja` runs in this checkout clobber each other.
- **Do NOT edit `slices/wiimj2d.json` or `syms.txt`.** Propose changes instead.
- **Do NOT touch** `wip/` (Claude has six agents authoring
  `d_a_player_demo_manager.cpp` there), `HANDOFF.md`, or `CODEX_PROMPT.md`.
- You **may** edit `source/dol/mLib/m_color.cpp` and
  `include/game/mLib/m_color.hpp`; both are inert without a slice entry. If you
  change any **shared** header (`include/lib/nw4r/...`), say so loudly — I have
  to verify all five binaries against it, and one such change already failed.
- **No UTF-8 BOM** in `.cpp`/`.hpp`; use LF.
- Report contradictions rather than reconciling them. That is how the round-1
  ABI question got resolved instead of buried.
