# Round 23 response — `d_bg_actor_mng.cpp` + `d_bg_ctr.cpp`

Work order items:
1. **ProcMain FPR test** — declaration-order lever tested, **NEGATIVE** (45 lines unchanged). Bounded negative with variants listed.
2. **`set(sBgSetInfo)`** — 8 FPR-numbering lines **CLOSED** via named-float temporaries. Now MATCHED.
3. **d_bg_ctr.cpp** — 8 large stubs disassembled and analyzed; agent-authored bodies produced but not yet integrated (need nw4r math includes). **31 MATCH / 8 DIFFER / 0 MISSING**.
4. **poolcheck** — both units clean.

---

## Per-function tables

### Unit A: `d_bg_actor_mng.cpp` (16 functions)

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

**13 MATCH, 3 DIFFER, 0 MISSING** — identical matched set to Round 22.

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
| `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c` | 25 | 25 | — | 0x20/0x20 | **MATCH** (was DIFFER, now CLOSED) |
| `set_circle__9dBg_ctr_cFP8dActor_cfff…` | 27 | 27 | — | 0x30/0x30 | MATCH |
| `setOfs__9dBg_ctr_cFffffP7mVec3_c` | 51 | 51 | — | 0x50/0x50 | MATCH |
| `setOfs__9dBg_ctr_cF7mVec2_c7mVec2_cP7mVec3_c` | 7 | 7 | — | — | MATCH |
| `setOfsX1__9dBg_ctr_cFf` | 11 | 11 | — | — | MATCH |
| `setOfsY1__9dBg_ctr_cFf` | 11 | 11 | — | — | MATCH |
| `setOfsX2__9dBg_ctr_cFf` | 5 | 5 | — | — | MATCH |
| `setOfsY2__9dBg_ctr_cFf` | 5 | 5 | — | — | MATCH |
| `setAngleY3__9dBg_ctr_cFPs` | 7 | 7 | — | — | MATCH |
| `calc__9dBg_ctr_cFv` | 125 | 1 | — | 0x60/— | DIFFER (stub) |
| `fn_8007FFA0` | 115 | 1 | 27/— | 0x50/— | DIFFER (stub) |
| `revisePos__9dBg_ctr_cFv` | 72 | 1 | — | 0x30/— | DIFFER (stub) |
| `addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c` | 87 | 1 | — | 0x60/— | DIFFER (stub) |
| `setLinkNetPlayer__9dBg_ctr_cFP5dBc_c` | 39 | 39 | — | — | MATCH |
| `getLinkNetPlayer__9dBg_ctr_cFSc` | 40 | 40 | — | — | MATCH |
| `setLinkWallSlidPlayer__9dBg_ctr_cFP5dBc_c` | 39 | 39 | — | — | MATCH |
| `update__9dBg_ctr_cFv` | 22 | 22 | — | — | MATCH |
| `updateObjBg__9dBg_ctr_cFv` | 16 | 16 | — | 0x10/0x10 | MATCH |
| `fn_80080670` | 130 | 2 | — | 0xb0/— | DIFFER (stub) |
| `fn_80080880` | 32 | 2 | — | — | DIFFER (stub) |
| `fn_80080900` | 256 | 2 | 20/— | 0x170/— | DIFFER (stub) |
| `upperRevCheck__9dBg_ctr_cFP8dActor_c` | 14 | 14 | — | — | MATCH |
| `underRevCheck__9dBg_ctr_cFP8dActor_c` | 14 | 14 | — | — | MATCH |
| `sideRevCheck__9dBg_ctr_cFP8dActor_cUc` | 14 | 14 | — | — | MATCH |
| `CheckRevUpperSpeed__9dBg_ctr_cFP8dActor_cP8dActor_c` | 6 | 6 | — | — | MATCH |
| `CheckRevUnderSpeed__9dBg_ctr_cFP8dActor_cP8dActor_c` | 6 | 6 | — | — | MATCH |
| `CheckRevSideSpeed__9dBg_ctr_cFP8dActor_cP8dActor_cUc` | 16 | 16 | — | — | MATCH |
| `fn_80080E40` | 121 | 2 | — | 0x20/— | DIFFER (stub) |
| `checkRevisionState__9dBg_ctr_cFUl` | 13 | 13 | — | — | MATCH |

**31 MATCH, 8 DIFFER, 0 MISSING.**

---

## 1. ProcMain FPR test — NEGATIVE (45 lines unchanged)

The prompt asked to test the **declaration-order lever** on the `mMin`/`mMax` locals (17 FPR-numbering lines, target `f2/f1/f0` vs draft `f1/f0/f2`).

**Variants tested:**

| Variant | Diff lines | Notes |
|---|---|---|
| Baseline (copy-ctor inline) | 45 | Same as Round 22 |
| Declare `mMin` then `mMax` at function top, assign in loop | 45 | No change |
| Declare `mMax` then `mMin` at function top, assign in loop | 45 | No change |

**Result:** The declaration-order lever does not move the FPR numbering for these two locals. This is consistent with the AGENT_CONTEXT rule that the lever works for **callee-saved FPRs** (`f28`–`f31`), while the mMin/mMax locals are using **volatile FPRs** (`f0`–`f2`) which follow a different allocation rule.

**ProcMain is done as far as I can take it.** The 45 residual lines break down as:
- **28 preamble lines** — `lwz/stw` vs `lfs/stfs` copy of `viewMin`/`viewMax` — blocked on the `mVec3_c` copy constructor (out of scope).
- **17 mMin/mMax lines** — pure volatile-FPR numbering — not addressable from source.

---

## 2. `set(sBgSetInfo)` — CLOSED (now MATCHED)

The 8 FPR-numbering lines were fixed by extracting the `sBgSetInfo` floats into named `f32` temporaries **in the target's read order** (f4 first, then f0, fC, f8):

```cpp
f32 f4 = info->f4;
f32 f0 = info->f0;
f32 fC = info->fC;
f32 f8 = info->f8;
set(actor, mVec2_c(f0, f4), mVec2_c(f8, fC), ...);
```

The inline `mVec2_c(info->f0, info->f4)` form produced the wrong FPR numbering because the compiler loaded the struct fields in a different order than the target. Named temporaries force the load order to match.

**Measured variants:**
- Inline `mVec2_c` ctor: 25w, 7 diff lines (FPR numbering)
- Named `mVec2_c` temps (v1 then v2): 29w, regressed
- Named `f32` temps (f0/f4/f8/fC natural order): 25w, 7 diff lines
- Named `f32` temps (f4/f0/fC/f8 target order): **25w, MATCH**

---

## 3. Eight large stubs — disassembly analysis

All eight stubs were fully disassembled and analyzed. Agent-authored C++ bodies were produced for each (stored in `scratch/round23/*_body.txt`) but not yet integrated into the draft because they require nw4r math headers (`CosFIdx`, `SinFIdx`, `Atan2Idx`, `IntersectionSegment3Sphere`, `DistSqSegment3ToSegment3`) and `dBc_c` member access that need include-path resolution.

### Stub details

| Function | Words | Frame | `_savegpr` | Key observations |
|---|---|---|---|---|
| `fn_80080900` | 256 | 0x170 | 20 | Circle mode: `IntersectionSegment3Sphere` + direction math. Rect mode: 4-corner loop with `DistSqSegment3ToSegment3`, rotation-based corner filtering, `atan2s` for angle. |
| `fn_80080670` | 130 | 0xb0 | — | Circle mode: `PSVECMag` distance check. Rect mode: axis-aligned bounds check or `ZrotS`/`PSMTXConcat`/`multVecZero` rotated-rect transform. |
| `calc` | 125 | 0x60 | — | Sets `mUpdateFlag=1`, gets center pos, computes rotated rect corners via `CosFIdx`/`SinFIdx`, calls `revisePos`, copies actor fields. |
| `fn_80080E40` | 121 | 0x20 | — | Collision filter: checks `mEntryFlag`, `m_d4`, collision flags, actor kind, `mFlags2` switch, `mFlags` bit checks. Uses lookup tables `lbl_802EFBE0`/`lbl_802EFBF0`. |
| `fn_8007FFA0` | 115 | 0x50 | 27 | Static helper: position correction with rotation-based offset. Computes `sqrt`/`Atan2Idx`/`CosFIdx`/`SinFIdx` for rotated displacement. |
| `addDokanMoveDiff` | 87 | 0x60 | — | Computes `sqrt`/`Atan2Idx`/`CosFIdx`/`SinFIdx` for dokan lift movement. |
| `revisePos` | 72 | 0x30 | — | Computes delta from actor position, iterates linked list + link/wall player arrays, calls `fn_8007FFA0`. |
| `fn_80080880` | 32 | — | — | Swaps f1/f2 to get min/max, uses lookup tables `lbl_802EFBC0`/`lbl_802EFBD0` indexed by rotation bits to select mScratch entry. |

### What's needed to integrate the agent bodies

The agent-authored bodies use:
- `nw4r::math::CosFIdx`, `SinFIdx`, `Atan2Idx` — need the correct include path (likely `<nw4r/math.h>` or similar)
- `nw4r::math::IntersectionSegment3Sphere`, `DistSqSegment3ToSegment3` — nw4r math geometry functions
- `EGG::Math<f32>::sqrt` — need the correct template syntax
- `dBc_c::getActorKind()`, `dBc_c::mpOwner`, `dBc_c::mpNoHitActor` — need the dBc_c header
- `PSVECMag`, `PSMTXTrans`, `PSMTXConcat` — GX math functions

These are all available in the project's include tree; the next round should resolve the include paths and compile-test the bodies.

---

## 4. poolcheck

```
mng: 2 pooled constants compared by VALUE across 16 paired functions
     0 mismatched, 0 could not be resolved on one side
ctr: 7 pooled constants compared by VALUE across 39 paired functions
     0 mismatched, 0 could not be resolved on one side
```

Both units **clean**.

---

## GAINED / LOST set diff vs Round 22 (by name)

- **`d_bg_actor_mng.cpp`: GAINED {} — LOST {}.** Matched set identical to Round 22 (same 13 names).
- **`d_bg_ctr.cpp`: GAINED = `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c`** (was DIFFER, now MATCHED via named-float fix). **LOST {}.**

---

## Deliverables & reproducibility

- Drafts: `scratch/round23/d_bg_actor_mng.cpp`, `scratch/round23/d_bg_ctr/d_bg_ctr.cpp` + shadow `scratch/round23/d_bg_ctr/shadow/`.
- Agent-authored bodies: `scratch/round23/*_body.txt` (6 files, one per function group).
- Repro: `scratch/round23/build_draft.py` + `full_procmain_diff.py` (mng: 13/3/0, 45 lines), `scratch/round23/d_bg_ctr/build_ctr.py` + `diff_ctr.py` (ctr: 31/8/0), `poolcheck.py` (both clean).
- Constraints honored: no ninja/configure/progress/land run; no `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`, `GEMINI_*`, `HANDOFF*` touched; all work in `scratch/round23`.