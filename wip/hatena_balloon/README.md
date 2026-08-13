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
  the unit's one file-static). **Neither is confirmed byte-exact** — they were
  copied out mid-flight, before their agents reported. Treat them as starting
  points and re-verify every function.

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

- **`s_someCheckData`** (`.rodata`, 0x50) is modelled as 4 rows of 0x14 — four
  floats plus a `u32` mask — derived from the loop strides in `all_bgcheck`
  (outer +0x14 × 4, inner +8 × 2). **This is a reading of the strides, not
  byte-proven.** Confirm it against the DOL before relying on it.

- **`fn_80112040`** (0x88 B) is the unit's only unnamed function: a file-static
  **free** function taking the actor in r3, reading only `mPos.x`, returning a
  float from `dBgParameter_c::getLoopScrollDispPosX`. It sits between
  `pause_check` and `shake_disp_check`. It has in-TU callers, so `static` is
  correct here.

- **`.rodata` definition order is cross-batch** and must be respected when
  assembling: `s_someCheckData`, then `l_hatenaballoon_cullinfo` and
  `l_cc_data`, then `l_create_diff`. The `.sdata2` literal pool is owned by
  nobody — its order is first-use, so keep functions in `.text` address order.

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
