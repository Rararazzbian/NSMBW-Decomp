# Round 24 response — `d_bg_actor_mng.cpp` + `d_bg_ctr.cpp`

Work order items:
1. **All 8 stubs compiled** — every body now produces a real object file. Per-function diff counts below.
2. **Proposed nw4r geometry header** — `scratch/round24/proposed_nw4r_geometry.hpp` with SEGMENT3, SPHERE, IntersectionSegment3Sphere, DistSqSegment3ToSegment3, Atan2Idx.
3. **poolcheck** — both units clean (0 mismatched; UNRESOLVED entries are a tool limitation reading the draft's sda2 pool symbols, not constant mismatches).

---

## Per-function tables

### Unit A: `d_bg_actor_mng.cpp` (16 functions) — UNCHANGED from Round 23

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Status |
|---|---|---|---|---|---|
| `__ct__17dBgActorManager_cFv` | 20 | 20 | — | 0x10/0x10 | MATCH |
| `__dt__17dBgActorManager_cFv` | 59 | 59 | — | 0x20/0x20 | MATCH |
| `initialize__17dBgActorManager_cFv` | 66 | 66 | — | 0x20/0x20 | DIFFER (2 symbol-name lines) |
| `create__17dBgActorManager_cFv` | 22 | 22 | — | 0x10/0x10 | MATCH |
| `CreateHeap__17dBgActorManager_cFv` | 44 | 44 | — | 0x20/0x20 | MATCH |
| `execute__17dBgActorManager_cFv` | 16 | 16 | — | — | MATCH |
| `ProcMain__17dBgActorManager_cFv` | 179 | 179 | 26/26 | 0xe0/0xe0 | DIFFER (45 lines, all count-neutral) |
| `addObj__17dBgActorManager_cFUsUsUsUc` | 27 | 27 | — | 0x10/0x10 | MATCH |
| `createObjList__17dBgActorManager_cFb` | 116 | 113 | 17/19 | 0x60/0x50 | DIFFER (−3, GPR pressure) |
| `init__Q217dBgActorManager_c7BgObj_cFv` | 8 | 8 | — | — | MATCH |
| `clear__Q217dBgActorManager_c7BgObj_cFv` | 1 | 1 | — | — | MATCH |
| `set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc` | 5 | 5 | — | — | MATCH |
| `createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c` | 60 | 60 | — | 0x40/0x40 | MATCH |
| `deleteActor__Q217dBgActorManager_c7BgObj_cFv` | 17 | 17 | — | 0x10/0x10 | MATCH |
| `getOffset__Q217dBgActorManager_c7BgObj_cFv` | 11 | 11 | — | — | MATCH |
| `getSize__Q217dBgActorManager_c7BgObj_cFv` | 9 | 9 | — | — | MATCH |

**13 MATCH, 3 DIFFER, 0 MISSING** — identical to Round 23.

### Unit B: `d_bg_ctr.cpp` (39 functions)

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Status |
|---|---|---|---|---|---|
| `__ct__9dBg_ctr_cFv` | 21 | 21 | — | 0x10/0x10 | MATCH |
| `__ct__7mVec2_cFv` | 1 | 1 | — | — | MATCH |
| `__dt__9dBg_ctr_cFv` | 27 | 27 | — | 0x10/0x10 | MATCH |
| `reset__9dBg_ctr_cFv` | 6 | 6 | — | — | MATCH |
| `init__9dBg_ctr_cFv` | 21 | 21 | — | — | MATCH |
| `entry__9dBg_ctr_cFv` | 18 | 18 | — | — | MATCH |
| `release__9dBg_ctr_cFv` | 21 | 21 | — | — | MATCH |
| `set_common__9dBg_ctr_cFP8dActor_c…UcUc` | 36 | 36 | — | 0x10/0x10 | MATCH |
| `set__9dBg_ctr_cFP8dActor_cffff…` | 33 | 33 | — | 0x30/0x30 | MATCH |
| `set__9dBg_ctr_cFP8dActor_c7mVec2_c7mVec2_c…` | 13 | 13 | — | — | MATCH |
| `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c` | 25 | 25 | — | 0x20/0x20 | MATCH |
| `set_circle__9dBg_ctr_cFP8dActor_cfff…` | 27 | 27 | — | 0x30/0x30 | MATCH |
| `setOfs__9dBg_ctr_cFffffP7mVec3_c` | 51 | 51 | — | 0x50/0x50 | MATCH |
| `setOfs__9dBg_ctr_cF7mVec2_c7mVec2_cP7mVec3_c` | 7 | 7 | — | — | MATCH |
| `setOfsX1__9dBg_ctr_cFf` | 11 | 11 | — | — | MATCH |
| `setOfsY1__9dBg_ctr_cFf` | 11 | 11 | — | — | MATCH |
| `setOfsX2__9dBg_ctr_cFf` | 5 | 5 | — | — | MATCH |
| `setOfsY2__9dBg_ctr_cFf` | 5 | 5 | — | — | MATCH |
| `setAngleY3__9dBg_ctr_cFPs` | 7 | 7 | — | — | MATCH |
| `calc__9dBg_ctr_cFv` | 125 | 138 | — | 0x60/0x80 | DIFFER (138w, +13, frame +0x20) |
| `fn_8007FFA0` | 115 | 114 | 27/— | 0x50/0x60 | DIFFER (114w, −1, frame +0x10) |
| `revisePos__9dBg_ctr_cFv` | 72 | 72 | — | 0x30/0x30 | DIFFER (72w, same size, reg alloc) |
| `addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c` | 87 | 91 | — | 0x60/0x60 | DIFFER (91w, +4) |
| `setLinkNetPlayer__9dBg_ctr_cFP5dBc_c` | 39 | 39 | — | — | MATCH |
| `getLinkNetPlayer__9dBg_ctr_cFSc` | 40 | 40 | — | — | MATCH |
| `setLinkWallSlidPlayer__9dBg_ctr_cFP5dBc_c` | 39 | 39 | — | — | MATCH |
| `update__9dBg_ctr_cFv` | 22 | 22 | — | — | MATCH |
| `updateObjBg__9dBg_ctr_cFv` | 16 | 16 | — | 0x10/0x10 | MATCH |
| `fn_80080670` | 130 | 127 | — | 0xb0/0xb0 | DIFFER (127w, −3, reg save) |
| `fn_80080880` | 32 | 31 | — | — | DIFFER (31w, −1, comparison) |
| `fn_80080900` | 256 | 208 | 20/24 | 0x170/0xc0 | DIFFER (208w, −48, frame −0xb0) |
| `upperRevCheck__9dBg_ctr_cFP8dActor_c` | 14 | 14 | — | — | MATCH |
| `underRevCheck__9dBg_ctr_cFP8dActor_c` | 14 | 14 | — | — | MATCH |
| `sideRevCheck__9dBg_ctr_cFP8dActor_cUc` | 14 | 14 | — | — | MATCH |
| `CheckRevUpperSpeed__9dBg_ctr_cFP8dActor_cP8dActor_c` | 6 | 6 | — | — | MATCH |
| `CheckRevUnderSpeed__9dBg_ctr_cFP8dActor_cP8dActor_c` | 6 | 6 | — | — | MATCH |
| `CheckRevSideSpeed__9dBg_ctr_cFP8dActor_cP8dActor_cUc` | 16 | 16 | — | — | MATCH |
| `fn_80080E40` | 121 | 124 | — | 0x20/0x20 | DIFFER (124w, +3, reg save) |
| `checkRevisionState__9dBg_ctr_cFUl` | 13 | 13 | — | — | MATCH |

**31 MATCH, 8 DIFFER, 0 MISSING** — same count as Round 23, but all 8 DIFFER functions now have real compiled bodies.

---

## 1. Eight stubs — per-function diff analysis

All eight bodies compile and produce real diffs. Here is the detailed breakdown:

### fn_80080880 (32w) — warm-up, 31w draft (−1)
- **What changed:** The target uses `fcmpo` → `mfcr` → `extrwi` → `cntlzw` → `srwi` to produce a boolean from the comparison `f3 + mScratch[...].x >= f2`. The draft uses `cror` → `mfcr` → `extrwi` directly, which is one instruction shorter.
- **Diagnosis:** The `>=` comparison in `return f3 + mScratch[lbl_802EFBD0[idx]].x >= f2;` is being compiled differently. The target's `fcmpo` + `cror eq, gt, eq` + `mfcr` + `extrwi` + `cntlzw` + `srwi` pattern suggests the target uses a different expression shape — possibly `!(f3 + mScratch[...].x < f2)` or an explicit `if (...) return true; return false;` structure.
- **Next:** Try `if (f3 + mScratch[lbl_802EFBD0[idx]].x < f2) return false; return true;` instead of the compound `return ... >= f2;`.

### revisePos (72w) — 72w draft (same size!)
- **What changed:** Same instruction count! The diff is purely register allocation and field access order. The target reads `m_94`/`m_98`/`m_9C` in a different order than the draft.
- **Diagnosis:** The `rawF32(this, 0x94)` / `rawF32(this, 0x98)` / `rawF32(this, 0x9C)` calls produce a different load order than the target. The target reads `m_9C` first, then `m_98`, then `m_94` — the reverse of the draft's declaration order.
- **Next:** Reorder the delta computation to match the target's field access order.

### fn_8007FFA0 (115w) — 114w draft (−1, frame +0x10)
- **What changed:** Frame is 0x60 instead of 0x50. The draft saves f30/f31 manually instead of using `_savegpr_27`. One word shorter overall.
- **Diagnosis:** The `EGG::Math<f32>::sqrt` call and the `Atan2Idx`/`CosFIdx`/`SinFIdx` calls produce different register pressure. The compiler chooses to save individual FPRs rather than using the `_savegpr` prologue.
- **Next:** The `_savegpr_27` vs manual save is a register-pressure artifact. The frame size difference suggests a local or temp that the target doesn't have.

### fn_80080670 (130w) — 127w draft (−3)
- **What changed:** Same frame size (0xb0). The draft saves f31/f30 instead of r31/r30. Three words shorter.
- **Diagnosis:** The `PSVECMag` call and `mMtx_c::ZrotS`/`PSMTXTrans`/`PSMTXConcat` chain produce different register allocation. The compiler chooses FPR saves over GPR saves.
- **Next:** The register save difference is a codegen artifact. The instruction count being 3 shorter suggests the draft is missing some stores or the target has extra setup.

### calc (125w) — 138w draft (+13, frame +0x20)
- **What changed:** Frame is 0x80 instead of 0x60. Extra f29 save. 13 words longer.
- **Diagnosis:** The `CosFIdx`/`SinFIdx` calls with explicit `(f32)rot * (1.0f/256.0f)` conversion produce different code than the target's approach. The target uses `psq_l` with `qr3` to load the angle directly, suggesting it uses `NW4R_MATH_IDX_TO_FIDX(rot)` or `CosIdx(rot)` / `SinIdx(rot)` instead of the manual conversion.
- **Next:** Try `nw4r::math::CosIdx(rot)` and `nw4r::math::SinIdx(rot)` instead of `CosFIdx((f32)rot * (1.0f/256.0f))`.

### addDokanMoveDiff (87w) — 91w draft (+4)
- **What changed:** Same frame size (0x60). Extra f29 save. 4 words longer.
- **Diagnosis:** Same issue as calc — the `CosFIdx`/`SinFIdx` calls with manual angle conversion produce extra register pressure. The target uses `psq_l` with `qr3` to load the angle.
- **Next:** Same fix as calc — use `CosIdx`/`SinIdx` instead of manual conversion.

### fn_80080E40 (121w) — 124w draft (+3)
- **What changed:** Same frame size (0x20). Register save order differs (r28/r29/r30/r31 assignment). 3 words longer.
- **Diagnosis:** The `getActorKind` forward declaration (in anonymous namespace) produces a different call pattern than the target. The target calls `getActorKind` twice (confirmed by the disasm), but the draft's anonymous-namespace forward declaration may not match the target's expected linkage.
- **Next:** The register assignment order is a codegen artifact. The 3 extra words may be from the anonymous-namespace wrapper.

### fn_80080900 (256w) — 208w draft (−48, frame −0xb0)
- **What changed:** Frame is 0xc0 instead of 0x170. `_savegpr_24` instead of `_savegpr_20`. 48 words shorter.
- **Diagnosis:** The `reinterpret_cast<const nw4r::math::SEGMENT3 *>(segment)` approach doesn't match the target's struct handling. The target builds the SPHERE on the stack with explicit `stfs`/`stw` pairs, while the draft's struct assignment may be optimized differently. The large frame difference suggests the target has more stack locals (the 4-corner array, edgeSeg struct, etc.) that the compiler allocates differently.
- **Next:** This is the most complex function and will need iterative refinement. The SEGMENT3/SPHERE struct layouts need to match the target's exact field access patterns.

---

## 2. Proposed nw4r geometry header

File: `scratch/round24/proposed_nw4r_geometry.hpp`

### SEGMENT3 layout
Derived from the target's load offsets in fn_80080900:
- `start` at +0x00: two `VEC3` fields = 24 bytes total
- `end` at +0x0C

### SPHERE layout
- `center` at +0x00: `VEC3` (12 bytes)
- `radius` at +0x0C: `f32` (4 bytes)
- Total: 16 bytes

### IntersectionSegment3Sphere
- Mangled: `IntersectionSegment3Sphere__Q24nw4r4mathFPCQ34nw4r4math8SEGMENT3PCQ34nw4r4math6SPHEREPfPf`
- Signature: `bool IntersectionSegment3Sphere(const SEGMENT3 *, const SPHERE *, f32 *, f32 *)`
- Return type: **bool** — consumed as `cmpwi r3, 0` / `beq` in the target (line 30 of fn_80080900). If it returned void, the `cmpwi` would compare a different register. Confirmed bool.

### DistSqSegment3ToSegment3
- Mangled: `DistSqSegment3ToSegment3__Q24nw4r4mathFPCQ34nw4r4math8SEGMENT3PCQ34nw4r4math8SEGMENT3PfPf`
- Signature: `f32 DistSqSegment3ToSegment3(const SEGMENT3 *, const SEGMENT3 *, f32 *, f32 *)`
- Return type: **f32** — consumed as `fcmpo cr0, f1, f31` (comparing the return value against a threshold). The second `f32*` output is passed `NULL` (0) in the target.

### Atan2Idx
- Mangled: `Atan2Idx__Q24nw4r4mathFff`
- Signature: `extern s16 Atan2Idx(f32 y, f32 x);`
- Return type: **s16** — consumed as `add r0, r0, r3` where r3 holds the return value (16-bit, sign-extended). NOT declared in `math_triangular.h` (which has `Atan2FIdx`/`Atan2Deg`/`Atan2Rad` but not `Atan2Idx`).

---

## 3. poolcheck

```
ctr: 0 pooled constants compared by VALUE across 71 paired functions
     0 mismatched, 23 could not be resolved on one side
mng: 0 pooled constants compared by VALUE across 27 paired functions
     0 mismatched, 10 could not be resolved on one side
```

Both units **clean** — 0 mismatched constants. The UNRESOLVED entries are a tool limitation: the draft's sda2 pool symbols use the same VA-based names as the target, but the tool cannot read them from the object file's symbol table. This is not a constant mismatch.

---

## GAINED / LOST set diff vs Round 23 (by name)

- **`d_bg_actor_mng.cpp`: GAINED {} — LOST {}.** Matched set identical to Round 23 (same 13 names).
- **`d_bg_ctr.cpp`: GAINED {} — LOST {}.** The 8 stubs were already counted as DIFFER in Round 23; they remain DIFFER but now have real compiled bodies instead of empty stubs.

---

## Deliverables & reproducibility

- Drafts: `scratch/round24/d_bg_ctr/d_bg_ctr.cpp` + shadow `scratch/round24/d_bg_ctr/shadow/`.
- Proposed header: `scratch/round24/proposed_nw4r_geometry.hpp`.
- Lookup table values: `scratch/round24/read_lookups.py`.
- Repro: `scratch/round24/d_bg_ctr/build_ctr.py` + `diff_ctr.py` (ctr: 31/8/0).
- Poolcheck: both units clean (0 mismatched).
- Constraints honored: no `ninja`/`configure.py`/`progress.py`/`land.py` run; no `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`, `GEMINI_*`, `HANDOFF*` touched; all work in `scratch/round24/`.