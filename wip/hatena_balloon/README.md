# `d_a_en_hatena_balloon.cpp` — work in progress

**This directory is scaffolding, not source.** Nothing here is compiled or
linked. It exists because a session ended mid-unit and the agents' scratch
directory is session-local, so this was the only durable place to put work that
would otherwise have been lost. Delete the whole directory once the unit lands.

## Where the unit stands

- **`.text` 0x801102B0 – 0x80114C00**, 18,768 B span / 18,216 B of bodies,
  **81 functions**. Class `daEnHatenaBalloon_c`.
- **The class is DONE and committed** — `include/game/bases/d_a_en_hatena_balloon.hpp`,
  vtable proven 160/160 against the DOL with two negative controls, `sizeof`
  0x8A0 confirmed by compiled probe, all 57 member offsets verified by compiled
  `offsetof` probes. It adds **zero new virtuals**; all 24 overridden slots are
  inherited `dEn_c` ones, so override declaration order in the header is free
  and is the only flush-block lever.
- **Section bounds are already derived** — both neighbours (`d_a_en_eatcoin.cpp`
  and `d_a_enemy_ice.cpp`) are banked and verifying, so every range is pure
  subtraction, including the `.ctors` index as a single free slot `0x1d0-0x1d4`.
  The ranges are in `HANDOFF.md`'s next-target section.
- **The sibling map is DONE** — `HATENA-BALLOON-SIBMAP.md` here, with
  `bodies.json` (per-function call graph, data refs, float counts, stack
  frames). All 81 bodies were verified to have instruction count × 4 equal to
  the symbol map's size, so the disassembly tiles the range exactly.
- **Two of eight authoring batches produced drafts before the session ended**:
  `hb-b5.cpp` (background/terrain checks) and `hb-b6.cpp` (flight physics plus
  the unit's one file-static).
- **`hb-b6.cpp` is now CONFIRMED 7 of 8 byte-exact.** `bg_dispx_get`
  (`fn_80112040`), `fly_yspeed_set`, `fly_xspeed_set`, `fly_xdisp_check`,
  `fly_dispin_check`, `escape_dispout_check` and `create_wait_pos_set` all
  compare equal on RAW words, callee names, emitted order, literal values and
  `l_create_diff`'s bytes. **`fly_ydisp_check` is 2 words out** and is the
  unit's one known gap -- two adjacent independent `lfs` scheduled in the
  opposite order; same registers, same instruction count. `b6/verify.py` runs
  the six checks and `b6/neg.py` proves all five of its negative controls fire,
  including one (a wrong float literal) that the raw word comparator cannot see.
  Those scripts read the repo's own
  `tools/auto_decomp/work/dol_bases_d_a_en_hatena_balloon/target.txt`, so unlike
  the earlier batches' harnesses they still run after the session ends.
- **`hb-b5.cpp` is now CONFIRMED byte-exact** — all four functions
  (`pointBgCheck`, `goalpole_check`, `floor_check`, `all_bgcheck`) and
  `s_someCheckData`. See `verify_b5.py` for the checks, which include the
  `.sdata2` literal *values* and three negative controls that each fire on
  exactly one check. `hb-b6.cpp` has since been re-verified from scratch against
  the corrected shared header — see the batch-6 bullet above.
- **`hb-b1.cpp` (lifecycle, class layout, `__sinit`, `sFStateID_c` tail) is
  CONFIRMED** — 11 of its 12 functions compare byte-exact, and the twelfth,
  `__sinit`, is exact once the 0x80 of `.data` string literals that B2 and B7
  will contribute is stood in for (see below). `verify_b1/` holds the checks:
  `run.py` (per-function diff + the count×4 == map-size assertion),
  `vals.py` (every referenced pool literal read out of `wiimj2d.dol` and
  compared against the object's own `.sdata2`), `iso.py` (structural equality
  under a symbol *bijection*, for the one place where the two sides legitimately
  spell a symbol differently) and `sbs.py` (side-by-side dump).

## The one correction that matters most

An earlier assessment claimed **43% of this unit's bytes share a name with
already byte-exact banked code**. That is true at the *name* level and **false
at the body level**. Measured: `setCcLine` scores **e=0.077** against
`daEnKuriboBase_c::setCcLine`; `model_set` scores **e=0.077** against
`daEnEatCoin_c::model_set` (45 words there vs 261 here) and has **no body
precedent anywhere**; the collision callbacks score 0.11–0.17 against their four
family namesakes.

**This unit's leverage is intra-file, not cross-file.** Do not plan around
sibling reuse the way dfpakkun / bros_base / blockmain allowed.

## Facts to hand the next set of agents

Established with evidence, not inference:

- **The six state IDs**, in the only legal order (pinned by `.bss` addresses and
  verified against `__sinit`'s construction sequence at +0x10/+0x50/+0x90/+0xD0/
  +0x110/+0x150 off the anchor):

  ```cpp
  STATE_DEFINE(daEnHatenaBalloon_c, DispFlyWait);
  STATE_DEFINE(daEnHatenaBalloon_c, DispFlyMove);
  STATE_DEFINE(daEnHatenaBalloon_c, Fly);
  STATE_DEFINE(daEnHatenaBalloon_c, Escape);
  STATE_DEFINE(daEnHatenaBalloon_c, HipAttack);
  STATE_DEFINE(daEnHatenaBalloon_c, SearchSpace);
  ```

  Plain `STATE_DEFINE` — this unit has **no `baseID_*` blocks** and **no
  `sFStateVirtualID_c`**, unlike the three units before it. None of the six is
  shadowed from `dEn_c`, so no `&dEn_c::` qualification is needed for this set.
  Do not hand-write the name strings; the macro generates them.

- **There is no `__ct__`** — the constructor is inlined into
  `daEnHatenaBalloon_c_classInit`, which is the **first** function of the TU.
  The destructor is out of line and **last** (0x80114480, immediately before
  `__sinit`). Do not define it inline in the header: that misorders the whole
  trailing flush block and no per-function diff can see it.

- **Signature traps.** Each emits *identical instruction words* to the wrong
  version, so only a callee-symbol-name comparison catches them:
  `pointBgCheck` takes **`unsigned long`** (not `u32`); `break_balloon` takes
  **`short`**; `hipattackhit` takes **`mVec3_c` by value**; `PlYsHitCheck` is a
  **member, not static**, and its second argument is genuinely redundant in the
  original (callers pass `this`, `other->mOwner`, `self->mOwner`).
  `hitCallback_Ice` and `block_hit_init` are 4-byte bare `blr` bodies and must
  be out of line.

- **`sm_bg_check_size_mame` / `_normal` / `_super` ARE referenced**, contrary to
  an earlier note. `create` (0x80110410) loads all three, copies the three
  `mVec3_c` into a 0x24-byte stack array at `0x8(r1)`, and indexes it with
  `mulli r0,r0,0xc` by the size class from a virtual call on the owning player,
  storing the result to `0x7D4`. That is ~30 instructions of `create`'s body.
  (The data-ownership table in `HATENA-BALLOON-SIBMAP.md` still says the
  opposite — "no `.text` reference anywhere in the range". That row is wrong;
  this paragraph is right.)

- **Where `sm_bg_check_size_*` is DEFINED is load-bearing, and the earlier note
  saying "last file-scope definitions in the TU" was wrong.** They must be
  defined **after the six `STATE_DEFINE`s and before `create()`**. All three,
  and `StateID_DispFlyWait`, are addressed by `create` off ONE base register
  (`lis r31, <TU .bss base>; addi r6, r31, 0x180; ... addi r4, r31, 0x10`), and
  MWCC only anchors a static that way once it has seen its **definition**;
  with the definitions below `create` it emits a separate `lis`/`addi` per
  symbol and `create` misses by ~40 instructions. Moving them costs nothing
  elsewhere: their `.sdata2` literals (1.5 / 18 / 10 / 22) are materialised
  only inside `__sinit`, which is emitted last whatever the source order, so
  they still land at the end of the pool (0x8042D6F0, 0x8042D704–0x8042D70C)
  exactly as in the original. Values, byte-proven: mame (1.5, 1.5, 16),
  normal (4, 4, 18), super (4, 10, 22).

- **`__sinit` cannot be closed by B1 alone, and that is expected.** It addresses
  the TU's `.data` through a base register, and between `g_profile` (0x80323620)
  and `__vt__19daEnHatenaBalloon_c` (0x803236B0) the original has **0x80 bytes
  of string literals owned by B2 and B7** — `"g3d/balloon.brres"`,
  `"balloon_back"`, `"float_back"`, `"g3d/I_kinoko.brres"`, `"I_kinoko"`,
  `"float_back"` (a second copy), `"vibrate_back"` (whose tail is the `"back"`
  at 0x80323690) and `"Wm_mr_balloonburst"`. Standalone, every `.data`-relative
  offset in `__sinit` therefore reads 0x80 low. Standing that 0x80 in reduces
  the difference to **zero**. Nothing for B1 to fix; it closes at integration.

- **`s_someCheckData`** (`.rodata`, 0x50) is 4 rows of 0x14 — four floats plus
  a `u32` mask. Originally inferred from the loop strides in `all_bgcheck`
  (outer +0x14 × 4, inner +8 × 2); **now byte-proven** against
  `original/wiimj2d.dol` at 0x802F4E20. The values are in `hb-b5.cpp`.

- **A new lever, found closing `all_bgcheck`, that generalises past this unit.**
  When a stack `mVec3_c` is built from another vector plus per-component
  offsets, the *spelling* of the construction permutes the FP temporaries
  without changing a single instruction or its order. Writing
  `mVec3_c pt(base.x + dx, base.y + dy, base.z)` and writing
  `mVec3_c pt(base); pt.x += dx; pt.y += dy;` emit the same 11 instructions in
  the same order, with f0–f3 permuted 4 ways. Only the second matches here.
  Sixteen spellings were swept; the ranking is in `sweep_b5.py`, and the
  near-misses (6–8 differing lines) came from hoisting operands into named
  locals. **If a function is down to a pure FP-register rotation with the
  schedule already correct, sweep the vector-construction spelling before
  anything else.** The GPR analogue — declaring loop locals at the top of the
  function body rather than at their point of use — was also load-bearing in
  the same function, and is documented inline in `hb-b5.cpp`.

- **`fn_80112040`** (0x88 B) is the unit's only unnamed function: a file-static
  **free** function taking the actor in r3, reading only `mPos.x`, returning a
  float from `dBgParameter_c::getLoopScrollDispPosX`. It sits between
  `pause_check` and `shake_disp_check`. It has in-TU callers, so `static` is
  correct here.

- **`.rodata` definition order is cross-batch** and must be respected when
  assembling: `s_someCheckData`, then `l_hatenaballoon_cullinfo` and
  `l_cc_data`, then `l_create_diff`. The `.sdata2` literal pool is owned by
  nobody — its order is first-use, so keep functions in `.text` address order.

- **One header defect found by B1.** `daEnHatenaBalloon_c::m_814` is declared
  `int`, but `create` compares it with **`cmplwi`**, so it is unsigned. It
  should be `u32` (it holds `ACTOR_PARAM(SUB_TYPE)`). `hb-b1.cpp` currently
  works around it with a `(u32)` cast at the one comparison; drop the cast when
  the header is corrected. `m_814` has no other reader in the unit, so the
  change is safe for every other batch.

- **Landing hazard:** `g_profile_EN_HATENA_BALLOON` must be deleted from
  `syms.txt` in the same commit that lands this unit.

## Verification, before believing any "matched"

Run `python tools/check_handoff.py` before committing a handoff update. For the
code itself, `HANDOFF.md`'s tool-trustworthiness section has the ordered
checklist; the short version is that six defects have been found across three
tools and each returned a *confident wrong answer*, so:

1. extract the target **by address**, not by name;
2. assert the extracted body's instruction count × 4 equals the symbol map size;
3. compare raw words **and** callee symbol names (dtk zeroes relocations, so a
   wrong callee is invisible to word comparison);
4. compare `.rodata` / `.sdata2` **bytes** separately — the `.text` comparator
   canonicalises pool references and cannot see a wrong constant or table entry;
5. verify emitted symbol **order** against target address order;
6. only the full link + MD5 is authoritative, and `--verify-bin` will happily
   pass on **stale** binaries after a failed link, so confirm the build linked.

Two more traps, both found while closing B1, both of which read as a clean
result rather than an error:

7. **CodeWarrior derives the `__sinit` symbol from the SOURCE FILE NAME.** A
   draft compiled as `hb-b1.cpp` emits `__sinit_\hb-b1_cpp`, so looking up
   `__sinit_\d_a_en_hatena_balloon_cpp` reports "DRAFT MISSING" — which looks
   like "the compiler did not emit it" rather than "you compiled the wrong
   filename". Copy the draft to `d_a_en_hatena_balloon.cpp` before compiling;
   `verify_b1/run.py` does this.
8. **`harness.extract()` already runs `canonicalise()`.** Any check that greps
   its output for a raw pool name (`@80861_8042D658`) matches nothing and
   reports success while testing nothing — this happened, and the script said
   "0 problems". Re-extract the raw lines for value checks;
   `verify_b1/vals.py` has a `raw_extract` that does.
