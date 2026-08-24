# Ready to land

Units that an agent has finished and byte-verified, waiting for me to run the
landing gate. Append new sections at the bottom. Do not edit sections above
yours.

Format is specified in `STANDING_ORDERS.md` section 8.

---

## PARKED: dol/bases/d_wm_bgm_sync.cpp
- Source: scratch/qwen_bgm/d_wm_bgm_sync.cpp
- Header: scratch/qwen_bgm/shadow/game/bases/d_wm_bgm_sync.hpp
- Functions: 5 of 5 at DIFFS 0, all byte-verified by hex compare
- Status: code is complete and byte-exact; the SLICE is unsolved. Four landing
  attempts, all rejected at binary verification, never at compile or link.
- See AGENT_CONTEXT.md "PARKED: dWmBgmSync_c" for the six section ranges already
  established, the method that proved each, and the open question about the
  vtable at 0x803217C8.
- Next step recorded there: diff the produced DOL against retail to find the
  disagreeing address, instead of inferring ranges. One measurement replaces
  four blind attempts.

## PARKED: dol/bases/d_random.cpp (dRandom_c::calcMachineRandom)
- Source: scratch/auto_random/d_random.cpp (best 61w shape) and
  scratch/auto_random/sw27/k1_bodyscope.cpp (best 54-diff shape)
- Header: scratch/auto_random/shadow/game/bases/d_random.hpp
  (+ shadow/lib/egg/core/eggController.h adding getDpdRawPos/getDpdDistance
  declarations -- additive, non-virtual, layout-neutral; needed for landing)
- Functions: 0 of 1 byte-exact. Best scores: 61w vs 62w target at 58 positional
  diffs (cursor addressing); 63w at 54 diffs (k1/k2, early rp/raw declarations).
- Slice hypothesis: .text 0x800D9850-0x800D9948 only. No .data/.bss/.ctors seen
  in this TU (no __sinit, no statics) but NOT yet proven by object inspection.

### What the target does (fully decoded, high confidence)
CRC32 seed over an 0x80-byte stack buffer at r1+0x30: u64 OSGetTime() at +0x00,
SCGetOwnerNickName nickname (char[0x18]) at +0x08, then four 0x18-byte records
at +0x20 built from mPad::g_core[i]: three floats read from cc+0x24/0x28/0x2c
(copied through TWO mVec3_c locals at r1+0x18 and r1+0x24; record fields are
reloaded from the SECOND copy), EGG::Vector2f raw pos (sret slot r1+0x8) plus a
dead-but-present mVec2_c local copy at r1+0x10, and getDpdDistance. Loop is
do-while (++i <= 3), counter compared as cmpwi rX, 3. Frame 0xd0,
_savegpr_27/_restgpr_27 (five callee-saved GPRs: base, two byte-IVs, counter,
rebuilt record address).

### The blocker (one sentence per axis)
Retail lowers BOTH array walks as pinned base + running BYTE IV: g_core via
lis/addi base + `lwzx r4, r29, r30` (r30 += 4), records via per-iteration
`addi r27, r1, 0x50; add r27, r27, r31` (r31 += 0x18). Every source spelling
tried compiles to the cheaper CURSOR form instead (moving pointer + displacement
loads, cursor advanced by stride), or to frame-rebuild + `mulli` when bound
inside the loop. ~50 variants across sweeps 12-27: [i+4] vs [i], pointer vs
reference vs array-reference bindings, POD member-array struct, whole-seed
local struct (inflates via __construct_array unless all-float), loop forms
(for/while/do-while/post-increment), u32 counter, explicit parallel offset
variables (normalized BACK to cursors), by-value static-inline helper returning
mVec3_c (sret semantics did not move it), declaration-order permutations for
rp/raw/a/b slots (a/b sit at 0x18/0x24 in both; only rp/raw swap).

### Ruled out (do not retry)
Whole-seed struct with mVec3_c/mVec2_c members (adds ~12 words of
__construct_array); helper functions taking cc* vs i (identical output);
EGG::Vector2f early declaration (63w shapes k1/k2, closer on diffs but +1 word
and still fst24); explicit co/ro offset variables (compiler re-normalizes).

### What would settle it
The base+IV+frame-rebuild lowering appears NOWHERE else in any prepared target
(grep over tools/auto_decomp/work/*/target.txt: only this unit). Find one real
occurrence elsewhere in wiimj2d with known-or-guessable source, or obtain MWCC's
optimization report for both compilations. Absent that, treat the five-GPR shape
as driven by a pressure/source property not yet identified.
