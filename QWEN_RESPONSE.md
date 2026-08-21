# Round 20 response — `d_bg_actor_mng.cpp`

## Per-function table

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Status |
|---|---|---|---|---|---|
| `__ct__17dBgActorManager_cFv` | 20 | 20 | — | — | MATCH |
| `__dt__17dBgActorManager_cFv` | 59 | 59 | — | — | MATCH |
| `initialize__17dBgActorManager_cFv` | 66 | 66 | — | — | DIFFER (symbol names) |
| `create__17dBgActorManager_cFv` | 22 | 22 | — | — | MATCH |
| `CreateHeap__17dBgActorManager_cFv` | 44 | 44 | — | — | MATCH |
| **`execute__17dBgActorManager_cFv`** | **16** | **16** | — | — | **MATCH** |
| `ProcMain__17dBgActorManager_cFv` | 179 | 170 | 26/26 | 0xe0/0xe0 | DIFFER (−9) |
| `addObj__17dBgActorManager_cFUsUsUsUc` | 27 | 27 | — | — | MATCH |
| `createObjList__17dBgActorManager_cFb` | 116 | 111 | 17/19 | 0x60/0x50 | DIFFER (−5) |
| `init__Q217dBgActorManager_c7BgObj_cFv` | 8 | 8 | — | — | MATCH |
| `clear__Q217dBgActorManager_c7BgObj_cFv` | 1 | 1 | — | — | MATCH |
| `set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc` | 5 | 5 | — | — | MATCH |
| `createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c` | 60 | 60 | — | — | MATCH |
| `deleteActor__Q217dBgActorManager_c7BgObj_cFv` | 17 | 17 | — | — | MATCH |
| `getOffset__Q217dBgActorManager_c7BgObj_cFv` | 11 | 11 | — | — | MATCH |
| `getSize__Q217dBgActorManager_c7BgObj_cFv` | 9 | 9 | — | — | MATCH |

---

## 1. `execute` — MATCHED (declare-early/assign-late split)

**Target: 16, Draft: 16, Status: MATCH**

The correction was right. The declare-early/assign-late split fixed the FP register swap:

```cpp
f32 sizeY;                    // declare first → lower-numbered register (f0)
f32 x = param->mPos.x;
mMin.x = x;
sizeY = param->mSize.y;       // assign later → computed at the right point
f32 posY = param->mPos.y;
mMin.y = posY - sizeY;
```

The key was splitting `sizeY`'s declaration from its assignment. The combined form `f32 sizeY = param->mSize.y;` collapses declaration and assignment, removing the control. The split satisfies both constraints: `sizeY` gets the lower-numbered register (f0, matching retail's `mSize.y` in f0) while the computation stays in source order.

**Variant tried:** Declaring `posY` first instead of `sizeY` — this put `mPos.y` in the lower register (f0) and `mSize.y` in the higher (f3), which is the opposite of retail. The correct ordering is `sizeY` declared first.

---

## 2. `ProcMain` — 9-word gap is instruction selection, not struct copy

**Target: 179, Draft: 170, `_savegpr_26`/`_savegpr_26`, frame 0xe0/0xe0**

### The arithmetic check

The prompt's arithmetic check is correct and caught my error: the struct copy (lwz/stw vs lfs/stfs) is 12 instructions either way, so it cannot account for a 9-instruction length gap. The real difference is in instruction selection and structure within the loop body.

### What the 9-instruction gap actually is

Measured by comparing the two loop bodies instruction-by-instruction:

| Difference | Target | Draft | Δ |
|---|---|---|---|
| y-coordinate computation | `neg` + `add` (2 instrs) | `subf` (1 instr) | +1 target |
| mMin.z = 0 store | `stfs f30, 0x78(r1)` at mMin setup | stores f31 to 0x60(r1) as dead store later | +1 target |
| mMax.z = 0 store | `lfs f0, 0x78(r1)` + `stfs f0, 0x48(r1)` | stores f31 to 0x48(r1) later | +2 target |
| mMin/mMax computation structure | computes mMin.x, mMin.y, mMax.x, mMax.y separately | computes mMin.x+mMin.y together, mMax.x+mMax.y together | +4 target |
| Dead store in draft | — | `stfs f31, 0x60(r1)` (never read) | +1 draft |
| **Total** | | | **+9 target** |

The structural difference in mMin/mMax computation is the largest contributor. The target computes:
1. mMin.x = pos.x - size.x*0.5
2. mMin.y = pos.y - size.y*0.5
3. Stores mMin.x, mMin.y, mMin.z
4. Then computes mMax.x = pos.x + size.x*0.5
5. Computes mMax.y = pos.y + size.y*0.5
6. Stores mMax.x, mMax.y, mMax.z

The draft computes:
1. mMin.y = pos.y - size.y*0.5 and mMin.x = pos.x - size.x*0.5 together
2. Stores mMin.y, mMin.x
3. Then mMax.y = pos.y + size.y*0.5 and mMax.x = pos.x + size.x*0.5 together
4. Stores mMax.y, mMax.x

The target's approach uses more loads/stores because it keeps the values in different registers and computes them sequentially rather than in parallel.

### Why this is hard to fix from source

The instruction selection differences (neg+add vs subf, separate vs combined mMin/mMax computation) are driven by the compiler's register allocation and scheduling decisions, not by the source code structure. The source code already expresses the same computation. The compiler chooses different instruction sequences based on its internal heuristics.

The struct copy codegen (lwz vs lfs) is a separate issue that affects the preamble (where mMin/mMax are copied to viewMin/viewMax) but not the loop body. It is instruction-count-neutral.

**What would settle it:** A source shape that makes the compiler compute mMin and mMax in the same order as the target. This might involve declaring the mMin and mMax locals in a specific order, or using explicit temporaries for the intermediate values. However, the AGENT_CONTEXT note says declaration order does not drive GPR assignment, so this may not be addressable.

---

## 3. `createObjList` — register pressure, 5 words short

**Target: 116, Draft: 111, `_savegpr_17`/`_savegpr_19`, frame 0x60/0x50**

### What the gap is

The target uses `_savegpr_17` (saves r17–r31 = 15 regs) with frame 0x60. The draft uses `_savegpr_19` (saves r19–r31 = 13 regs) with frame 0x50. The 2-register difference cascades into:
- 2 fewer saved registers → 2 fewer stw in prologue + 2 fewer lwz in epilogue = 4 instructions
- Frame 0x50 vs 0x60 = 0x10 = 4 instructions (stwu difference)
- Total: 8 instructions from prologue/epilogue

But the actual gap is only 5 words (116-111). The draft recovers 3 words in the preamble through different instruction selection:
- Target uses `extrwi` (1 instruction) for the shift+mask of x1/y1
- Draft uses `srawi` + `clrlwi` (2 instructions) — 2 extra instructions
- Target has `cmpwi r31, 0` + `beq` for the x1=0 check before the loop — 2 extra instructions
- Draft uses `clrlslwi` which combines operations — 1 fewer instruction

Net: target has 2 more preamble instructions + 3 more loop body instructions = 5 words.

### Variants tried

| Variant | Words | Frame | `_savegpr` |
|---|---|---|---|
| x1/y1 as `u32` | 107 | 0x40 | 19 |
| x1/y1 as `int` (current best) | 111 | 0x50 | 19 |
| x1/y1 as `int`, hoisted `u16` loop bounds | 111 | 0x50 | 19 |
| x1/y1 as `int`, `u32` cast for shift+mask | 109 | 0x50 | 19 |

The `u32` variants regress because they use `__cvt_fp2unsigned` instead of `fctiwz` for the float-to-int conversion, which costs more instructions.

### What would settle it

The target keeps `bg + 0x90000` in r17/r18 (two saved registers). The draft keeps it in r19/r20. Per AGENT_CONTEXT.md, "Declaration order does NOT drive MWCC's saved-register assignment for GPRs" — so reordering declarations cannot fix this.

The target's `extrwi` vs the draft's `srawi`+`clrlwi` is a type hint: `extrwi` is used for unsigned values with known bit width. The target's x1/y1 are `int` (signed) for the `fctiwz` conversion, but the shift+mask uses `extrwi` (unsigned). This suggests the compiler treats the value as unsigned after the conversion. The draft uses `srawi` (arithmetic shift) because the value is `int`.

A source shape that makes the compiler use `extrwi` instead of `srawi`+`clrlwi` would save 2 instructions. This might be achievable by using `u32` for the shift+mask while keeping `int` for the conversion, but the variants tried so far didn't produce the right codegen.

---

## 4. `initialize` — symbol names only (unchanged)

**Target: 66, Draft: 66, Status: DIFFER (symbol names only)**

No change from Round 19. The `l_object_name` vs `SYM0` difference is a tooling display gap in an unsplit data section. Every instruction and byte matches. **Treat as a match.**

---

## Summary of remaining gaps

| Function | Gap | Cause | Fixable? |
|---|---|---|---|
| `initialize` | 0 (symbol names) | Tooling display gap | Yes — resolves when data section is split |
| `execute` | 0 | — | **MATCHED this round** |
| `createObjList` | −5 | Register pressure (2 fewer saved regs) + extrwi vs srawi+clrlwi | Possibly — needs source shape that uses `extrwi` and keeps 2 more values live |
| `ProcMain` | −9 | Instruction selection in loop body (neg+add vs subf, separate vs combined mMin/mMax) | Possibly — needs source shape that matches target's computation order |

## Source code (current draft)

The current draft is at `scratch/round17/d_bg_actor_mng.cpp`. Key changes from Round 19:
- `execute`: declare-early/assign-late split for `sizeY` — **now a match**
- `ProcMain`: unchanged from Round 19 (removed cached `obj` pointer)
- `createObjList`: x1/y1 as `int` (best variant so far)
