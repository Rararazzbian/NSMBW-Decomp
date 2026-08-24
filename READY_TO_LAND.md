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

## PARKED: dol/bases/d_a_zoom_pipe_base.cpp (daZoomPipeBase_c)
Parked 2026-08-24 at Attempts 3 per the stall rule. Three sessions spent;
execute closed, init is one allocator mechanism away but nobody found the lever.

- Source: scratch/auto_zoompipe/d_a_zoom_pipe_base.cpp
- Header: scratch/auto_zoompipe/shadow/game/bases/d_a_zoom_pipe_base.hpp
  (declares daObjPipeBase_c {calcDownLength, execute}, extern "C" fn_80045A10,
  daZoomPipeBase_c layout: mParam@0x4, mTargetLength@0x5C8, mSpeed[2]@0x5CC,
  mStep@0x5D4, mCurrent@0x5D8, mIdx@0x5DC)
- Functions: 1 of 2 byte-exact.
  execute__16daZoomPipeBase_cFv 53/53 words DIFFS 0, hex-verified.
  init__16daZoomPipeBase_cFUi 44/44 words, frame 0x20 both sides,
  20 diffs (plus 3 cosmetic @sda21 pool-label name artifacts whose bytes match).
- Slice hypothesis (session 2, verified tiling):
  .text 0x80063F80-0x80064104 (init 176 B + execute 212 B);
  .sdata2 0xA78-0xA88 tiles exactly between spin_child_base and d_actor.
  No __sinit, no __vt__, no .data/.bss seen for this class.
- Externals to pin at landing: daObjPipeBase_c::calcDownLength,
  daObjPipeBase_c::execute, fn_80045A10 (unnamed helper, extern "C";
  param 3 is FLOAT -- retail narrows the int arg with fsubs before the call;
  args after it are (1, 0, 0)).

### The residual, fully decoded (session 4 analysis -- start HERE)
init converts three nibbles of mParam to float via stw-magic/lfd/fsubs/fmuls.
Source mapping CONFIRMED CORRECT against retail:
  mStep      (0x5D4) = ((w>>8)&0xF) * 0.5f + 0.5f   [extrwi r,4,20]
  mSpeed[1]  (0x5D0) = (w&0xF)      * 16.f + 16.f   [clrlwi 28]
  mSpeed[0]  (0x5CC) = ((w>>4)&0xF) * 16.f + 16.f   [extrwi r,4,24]
  mCurrent   (0x5D8) = mSpeed[0]
Retail allocates the two reusable int->double conversion slot pairs:
  pair A {r1+0x8,+0xC}: 1st conv = mStep nibble, REUSED 2nd conv = mSpeed[1]
  pair B {r1+0x10,+0x14}: 3rd conv = mSpeed[0], then REUSED for kind=(w>>16)&3
The draft inverts the middle two: mSpeed[1] takes fresh pair B, mSpeed[0]
reuses pair A. Every remaining diff follows from that one inversion --
retail keeps the mStep product live in f6 across the region and folds late;
the draft interleaves through volatile f0-f3. Retail also schedules the
mSpeed[1] conversion's live range OVERLAPPING mSpeed[0]'s (stfw w19/lfd w23
vs stfw w21/lfd w24), which forces distinct slots; the draft's schedule has
no overlap (lfd lands between the other's stfw), so the allocator pairs them
the opposite way.

### What would settle it
Find a source shape that makes the compiler schedule the mSpeed[1]
conversion's lfd AFTER mSpeed[0]'s stfw (restoring the overlap). Untried
candidates: bind (w&0xF) to a named local converted at a controlled point;
interleave an unrelated op between the two assignments; convert through an
explicit double local. ~100 swept variants over sessions 1-3 reordered
statements/declarations/compound-add forms without hitting it; see
scratch/auto_zoompipe/ variants and the WORK_QUEUE Attempts note.
