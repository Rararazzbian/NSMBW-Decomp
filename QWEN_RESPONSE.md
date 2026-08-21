# Round 21 response — `d_bg_actor_mng.cpp`

## Per-function table

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Status |
|---|---|---|---|---|---|
| `__ct__17dBgActorManager_cFv` | 20 | 20 | — | — | MATCH |
| `__dt__17dBgActorManager_cFv` | 59 | 59 | — | — | MATCH |
| `initialize__17dBgActorManager_cFv` | 66 | 66 | — | — | DIFFER (symbol names) |
| `create__17dBgActorManager_cFv` | 22 | 22 | — | — | MATCH |
| `CreateHeap__17dBgActorManager_cFv` | 44 | 44 | — | — | MATCH |
| `execute__17dBgActorManager_cFv` | 16 | 16 | — | — | MATCH |
| **`ProcMain__17dBgActorManager_cFv`** | **179** | **179** | 26/26 | 0xe0/0xe0 | **DIFFER (−0, length closed)** |
| `addObj__17dBgActorManager_cFUsUsUsUc` | 27 | 27 | — | — | MATCH |
| **`createObjList__17dBgActorManager_cFb`** | **116** | **113** | 17/19 | 0x60/0x50 | DIFFER (−3) |
| `init__Q217dBgActorManager_c7BgObj_cFv` | 8 | 8 | — | — | MATCH |
| `clear__Q217dBgActorManager_c7BgObj_cFv` | 1 | 1 | — | — | MATCH |
| `set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc` | 5 | 5 | — | — | MATCH |
| `createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c` | 60 | 60 | — | — | MATCH |
| `deleteActor__Q217dBgActorManager_c7BgObj_cFv` | 17 | 17 | — | — | MATCH |
| `getOffset__Q217dBgActorManager_c7BgObj_cFv` | 11 | 11 | — | — | MATCH |
| `getSize__Q217dBgActorManager_c7BgObj_cFv` | 9 | 9 | — | — | MATCH |

---

## 1. `ProcMain` — length gap CLOSED (179 / 179, −0)

**Target: 179, Draft: 179, `_savegpr_26`/`_savegpr_26`, frame 0xe0/0xe0**

The serial member assignment worked — but the exact winning shape was **copy-then-modify**, not bare memberwise assignment:

```cpp
mVec3_c mMin = pos;                       // copy: pos.x → mMin.x, pos.y → mMin.y, 0 → mMin.z
mMin.x -= m_pObjList[i].getSize().x * 0.5f;
mMin.y -= m_pObjList[i].getSize().y * 0.5f;
mVec3_c mMax = pos;                       // copy again: fresh pos values
mMax.x += m_pObjList[i].getSize().x * 0.5f;
mMax.y += m_pObjList[i].getSize().y * 0.5f;
```

### What moved (the Round 20 five-item table, re-measured)

| Round 20 line item | Before | After |
|---|---|---|
| neg+add vs subf (y-coordinate) | +1 target | still a selection diff (equal count) |
| mMin.z = 0 store | +1 target | **gone — .z now written by the copy** |
| mMax.z = 0 store | +2 target | **gone — .z now written by the copy** |
| separate vs combined mMin/mMax computation | +4 target | **gone — serial per-component** |
| dead store in draft | −1 draft | gone |

**The 9-word gap is closed.** Target and draft are both 179 instructions; the loop body is 125 instructions on both sides. The copy-then-modify form makes the compiler emit the target's exact serial structure: write mMin.x, mMin.y, mMin.z as a block, then mMax.x, mMax.y, mMax.z as a block — no interleaving.

### Why the copy matters (measured, not guessed)

Two shapes were tested before the winner:

1. **Bare memberwise assignment** (`mVec3_c mMin; mMin.x = pos.x - ...; mMin.y = ...; mMin.z = 0.0f;`) — **no change** (170 words). The compiler already serialises a 3-argument `mVec3_c` constructor, so converting the constructor to bare assignments changed nothing.
2. **Copy-then-modify** (`mVec3_c mMin = pos; mMin.x -= ...;`) — **+9 words, length closed.** The copy is the lever: it writes the base values (pos.x, pos.y, 0.0f) into the mMin/mMax slots *first*, which is exactly the target's "store base then overwrite" schedule. The `.z = 0` falls out of copying `pos` (whose z is 0), which reproduces the target's `stfs f30` of zero at the right point — both `.z` line items closed for free.

### What remains (40 instruction lines differ, all same-count)

The remaining diff is pure register allocation and scheduling:

- **Preamble struct copy** (`viewMin = mMin; viewMax = mMax;`): target copies via `lwz`/`stw` (integer path), draft via `lfs`/`stfs` (float path). Instruction-count-neutral — the arithmetic check holds: this was never the length gap, it is the residual selection difference.
- **neg+add vs subf** in the loop's y-coordinate: target `neg r0, r0; add r0, r28, r0` (2 instrs), draft `subf r5, r5, r28` (1 instr). The draft's one-instruction saving is now absorbed elsewhere in the equal count — this is the single remaining instruction-selection difference in the loop body.
- Register allocation drift throughout (r29/r28 vs r30/r31 for the x0/y0 hoisted values, load order of mX/mY swapped).

**What would settle it:** the neg+add vs subf choice. The target computes `y0 - mY` as `neg` + `add`; the draft collapses it to `subf`. Both are the same expression — the difference is which register the compiler assigns the negated operand. This is the FP/GPR scheduler's choice, and per the corrected note in AGENT_CONTEXT, the *statement order* axis is still open here: the preamble's `y0` computation (`int y0 = (int)(bg->m_8fe6c * 0.0625f)`) could be reshaped (e.g., negate the float before conversion, matching the target's `fneg`), which would give the loop a ready-made negative operand and let `add` do the work without `subf`. Worth a look next round.

---

## 2. `createObjList` — extrwi achieved (116 / 113, −3)

**Target: 116, Draft: 113, `_savegpr_17`/`_savegpr_19`, frame 0x60/0x50**

The two-variable split worked exactly as the prompt predicted — **2 of the 5 words recovered** (111 → 113), and the shift-and-mask now emits `extrwi` matching the target:

```cpp
int x1 = (int)(bg->m_8fe68 - bg->m_8fe64);   // int: keeps fctiwz for the float→int conversion
int y1 = (int)(bg->m_8fe6c - bg->m_8fe70);
u16 x1u = (u16)x1;                            // u16: makes the shift-and-mask emit extrwi
u16 y1u = (u16)y1;
x1u = (x1u & 0xF) ? (x1u >> 4) + 1 : (x1u >> 4);
y1u = (y1u & 0xF) ? (y1u >> 4) + 1 : (y1u >> 4);
```

The key was **declaring the shift-mask local as `u16`** (not casting at the use site, not `u32`). MWCC picks the shift from the variable's declared type:
- `int` → `srawi` + `clrlwi` (signed path)
- `u32` → `srwi` (logical but unmasked — measured 109 words, no extrwi)
- `u16` → **`extrwi rA, rA, 12, 16`** (matches target exactly)

### Variants tried (all measured)

| Variant | Words | extrwi? | Notes |
|---|---|---|---|
| `int` x1/y1 (Round 20 best) | 111 | no (srawi+clrlwi) | — |
| `u32` x1u = (u32)x1; shift on u32 | 109 | no (srwi) | loop bounds still on `(u16)` of u32 |
| `u16` x1u = (u16)x1; shift on u16 | **113** | **yes** | **the prompt's split — kept** |
| `u32` if/else with `& 0xFFF` masks | 109 | yes | hoisted extrwi, different loop shape |
| `u16` result cast on int shift (`(u16)((x1 & 0xF) ? ...)`) | 111 | no | cast at use site does not change the shift — confirmed the prompt's rule |
| `u16` x1u; shift on u16; loop on x1u (not (u16)x1u) | 113 | yes | same as kept |

The `u16` version costs +2 words over the `int` version (the `clrlwi` truncation on `(u16)x1`) but that is exactly the +2 the prompt predicted: the extrwi replaces `srawi`+`clrlwi` (2 instrs) with 1, and the declared-type truncation adds the loop-bound `clrlwi` the target has anyway (`clrlwi r31, r27, 16`).

### What remains (−3 words)

The residual is the register pressure difference the prompt said to leave alone: `_savegpr_19` vs `_savegpr_17` (2 fewer saved registers → 4 words) and frame 0x50 vs 0x60 (4 words), partially recovered by the extrwi and scheduling (−5 elsewhere). Confirmed consistent with AGENT_CONTEXT: the GPR note applies — declaration order does not drive which of r17/r19 the allocator picks.

---

## 3. `initialize` — closed, not revisited

66/66, symbol names only (`l_object_name` vs `SYM0`). Treat as matched; tooling fix is on the data-section side.

---

## Summary of remaining gaps

| Function | Gap | Cause | Fixable? |
|---|---|---|---|
| `initialize` | 0 (symbol names) | Tooling display gap | Yes — data section split |
| `execute` | 0 | — | MATCH (Round 20) |
| `ProcMain` | **0 (length)** | 40 same-count selection/alloc lines: lwz-vs-lfs struct copy, neg+add vs subf, register drift | Possibly — reshape `y0` to negate in the preamble, retry statement order |
| `createObjList` | −3 | `_savegpr_19` vs `_savegpr_17` + frame 0x50 vs 0x60 | Per AGENT_CONTEXT, GPR assignment is not source-addressable |

## Round 21 summary

1. **ProcMain −9 → −0.** The prompt's serial-member-assignment lever was correct; the winning shape was copy-then-modify (`mVec3_c mMin = pos; mMin.x -= ...`), which closes all five Round 20 line items at once. The remaining diff is 40 same-count lines of register allocation / instruction selection.
2. **createObjList −5 → −3.** The two-variable split (int conversion + u16 shift-mask) produced `extrwi` and recovered 2 words, exactly as predicted. Remaining −3 is the GPR register-pressure difference, off-limits per AGENT_CONTEXT.
3. **initialize** not touched.

The draft is at `scratch/round17/d_bg_actor_mng.cpp`; scripts `build_draft.py` / `diff_all.py` reproduce every number above.
