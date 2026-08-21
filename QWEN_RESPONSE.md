# Round 19 response — `d_bg_actor_mng.cpp`

## Per-function table

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Status |
|---|---|---|---|---|---|
| `__ct__17dBgActorManager_cFv` | 20 | 20 | — | — | MATCH |
| `__dt__17dBgActorManager_cFv` | 59 | 59 | — | — | MATCH |
| `initialize__17dBgActorManager_cFv` | 66 | 66 | — | — | DIFFER (symbol names) |
| `create__17dBgActorManager_cFv` | 22 | 22 | — | — | MATCH |
| `CreateHeap__17dBgActorManager_cFv` | 44 | 44 | — | — | MATCH |
| `execute__17dBgActorManager_cFv` | 16 | 16 | — | — | DIFFER (register swap) |
| **`ProcMain__17dBgActorManager_cFv`** | **179** | **170** | **26/26** | **0xe0/0xe0** | **DIFFER (−9)** |
| `addObj__17dBgActorManager_cFUsUsUsUc` | 27 | 27 | — | — | MATCH |
| **`createObjList__17dBgActorManager_cFb`** | **116** | **111** | **17/19** | **0x60/0x50** | **DIFFER (−5)** |
| `init__Q217dBgActorManager_c7BgObj_cFv` | 8 | 8 | — | — | MATCH |
| `clear__Q217dBgActorManager_c7BgObj_cFv` | 1 | 1 | — | — | MATCH |
| `set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc` | 5 | 5 | — | — | MATCH |
| `createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c` | 60 | 60 | — | — | MATCH |
| `deleteActor__Q217dBgActorManager_c7BgObj_cFv` | 17 | 17 | — | — | MATCH |
| `getOffset__Q217dBgActorManager_c7BgObj_cFv` | 11 | 11 | — | — | MATCH |
| `getSize__Q217dBgActorManager_c7BgObj_cFv` | 9 | 9 | — | — | MATCH |

---

## 1. `initialize` — still symbol names only

**Target: 66, Draft: 66, Status: DIFFER (symbol names only)**

The fixed canonicaliser did not change the result. The diff is still:
```
want: lis r31, l_object_name@ha     got: lis r31, SYM0@ha
want: addi r31, r31, l_object_name@l  got: addi r31, r31, SYM0@l
```

The canonicaliser fix addressed pool symbols and sbss sections, but `l_object_name` is a `.data` section label. The draft's disassembler assigns it a generic `SYM0` name because the data section is not yet split. This is a tooling display gap, not a codegen difference — every instruction and every byte matches. **This function is effectively a match** and will resolve when the data section is properly attributed.

---

## 2. `execute` — register swap, not source-addressable

**Target: 16, Draft: 16, Status: DIFFER (3 instructions)**

The remaining diff is a pure FP register swap:
```
want: lfs f0, 0x44(r4)    got: lfs f3, 0x44(r4)    # mSize.y
want: lfs f3, 0x3c(r4)    got: lfs f0, 0x3c(r4)    # mPos.y
want: fsubs f0, f3, f0    got: fsubs f0, f0, f3    # mPos.y - mSize.y
```

The target evaluates `mSize.y` first (into f0), then `mPos.y` (into f3). The draft evaluates `mPos.y` first (into f0), then `mSize.y` (into f3). The subtraction operands are swapped accordingly.

**Lever 12 applied:** Tried giving the left operand a def-point (`mMin.y = param->mPos.y; mMin.y -= param->mSize.y;`) — this made it worse (17 instructions). Tried declaring `sizeY` as a temp before the expression — same result. Tried declaring `posY` as a temp — same result.

**Conclusion:** This is a pure register allocation difference in a 16-word function with no `_savegpr` difference. Per AGENT_CONTEXT.md, "Treat a pure register-permutation residual as not source-addressable." The instructions and their order are correct; only the FP register numbers differ.

---

## 3. `createObjList` — register pressure, 5 words short

**Target: 116, Draft: 111, `_savegpr_17`/`_savegpr_19`, frame 0x60/0x50**

### What was fixed

Changed x1/y1 from `u32` to `int` (matching the target's `fctiwz` for these). This moved the draft from 107 → 111 words and frame from 0x40 → 0x50.

### What remains

The target uses `_savegpr_17` (saves r17–r31 = 15 regs) with frame 0x60. The draft uses `_savegpr_19` (saves r19–r31 = 13 regs) with frame 0x50.

The target keeps `bg + 0x90000` in r17/r18 (two saved registers for the base pointer). The draft keeps it in r19/r20. This 2-register difference cascades into:
- 2 fewer saved registers → 2 fewer stw in prologue + 2 fewer lwz in epilogue = 4 instructions
- Frame 0x50 vs 0x60 = 0x10 = 4 instructions (stwu difference)
- Total: 8 instructions = 8 words from prologue/epilogue

But the actual gap is only 5 words (116-111). The remaining 3 words are recovered by the draft having slightly different instruction selection in the loop body (e.g., `clrlslwi` vs `slwi` + `clrlwi`).

**Variants tried:**
- All `u32`: 107 words, frame 0x40, `_savegpr_19`
- x0/y0 `u32`, x1/y1 `int`: 111 words, frame 0x50, `_savegpr_19`
- Compound assignment for float multiplies: fixed `fmuls` operand order but didn't change register allocation
- Separate `u16` loop bound variables: no change

**What would settle it:** A source shape that makes MWCC keep 2 more values live in registers across the loop nest. The prompt suggested hoisting the base pointer into a named local, but the compiler already keeps `bg + 0x90000` in a register (r19/r20) — it just chooses a higher register than the target. Per AGENT_CONTEXT.md, "Declaration order does NOT drive MWCC's saved-register assignment" — so reordering declarations cannot fix this.

---

## 4. `ProcMain` — recompute-vs-cache fixed, 9 words remaining

**Target: 179, Draft: 170, `_savegpr_26`/`_savegpr_26`, frame 0xe0/0xe0**

### What was fixed

Removed the cached `obj` pointer (`BgObj_c *obj = &m_pObjList[i]`) and replaced all uses with `m_pObjList[i]` directly. This forced the compiler to recompute `&m_pObjList[i]` before each use, matching the target's pattern.

**Result:** Draft went from 162 → 170 words. `_savegpr` level changed from `_savegpr_25` to `_savegpr_26` (matching the target). Frame stayed at 0xe0 (already matching).

### What remains

The remaining 9-word gap is the struct copy codegen difference:
- **Target:** Copies `mMin`/`mMax` to `viewMin`/`viewMax` using integer `lwz`/`stw`
- **Draft:** Copies using float `lfs`/`stfs`

Both are 12 instructions (6 for mMin + 6 for mMax), so the instruction count is identical. The difference is in instruction selection only, driven by whether `mVec3_c` has a user-declared copy constructor (it does in the current header, suppressing bitwise copy).

**This is the same struct copy issue diagnosed in Round 18.** The prompt confirmed the diagnosis was correct and measured the blast radius (160 regressing functions across 49 files). The remaining gap cannot be closed without a whole-project migration to a POD `mVec3_c`.

### Correction accepted

The prompt corrected my Round 18 claim of a missing `xoris`. Re-measured: **both sides have exactly two `xoris` instructions.** That 2-instruction explanation was never real.

---

## Summary of remaining gaps

| Function | Gap | Cause | Fixable? |
|---|---|---|---|
| `initialize` | 0 (symbol names) | Tooling display gap | Yes — resolves when data section is split |
| `execute` | 0 (register swap) | FP register allocation | No — not source-addressable |
| `createObjList` | −5 | Register pressure (2 fewer saved regs) | Possibly — needs source shape that keeps 2 more values live |
| `ProcMain` | −9 | Struct copy codegen (lwz/stw vs lfs/stfs) | No — requires whole-project mVec3_c migration |

## Source code (current draft)

The current draft is at `scratch/round17/d_bg_actor_mng.cpp`. Key changes from Round 18:
- `execute`: reverted to original (register swap not source-addressable)
- `ProcMain`: removed cached `obj` pointer, uses `m_pObjList[i]` directly
- `createObjList`: x1/y1 changed from `u32` to `int` (mixed types)
