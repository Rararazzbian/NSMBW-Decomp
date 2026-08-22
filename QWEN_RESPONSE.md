# Round 22 response — `d_bg_actor_mng.cpp` + `d_bg_ctr.cpp`

Work order items:
1. **ProcMain** — reshape tested, lever found, whole-struct copy test done. 179/179, full diff **73 → 45 lines** (all count-neutral).
2. **initialize** — left as instructed. 66/66, symbol names only.
3. **d_bg_ctr.cpp** — seeded, end boundary verified, 39 functions, **30 MATCH / 9 DIFFER / 0 MISSING** on first pass. Ranked list below.

Both units **poolcheck-clean** (by value).

---

## Per-function tables

### Unit A: `d_bg_actor_mng.cpp` (16 functions, same TU as Rounds 17–21)

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Status |
|---|---|---|---|---|---|
| `__ct__17dBgActorManager_cFv` | 20 | 20 | — | 0x10/0x10 | MATCH |
| `__dt__17dBgActorManager_cFv` | 59 | 59 | — | 0x20/0x20 | MATCH |
| `initialize__17dBgActorManager_cFv` | 66 | 66 | — | 0x20/0x20 | DIFFER (2 symbol-name lines: `l_object_name` vs `SYM0`) |
| `create__17dBgActorManager_cFv` | 22 | 22 | — | 0x10/0x10 | MATCH |
| `CreateHeap__17dBgActorManager_cFv` | 44 | 44 | — | 0x20/0x20 | MATCH |
| `execute__17dBgActorManager_cFv` | 16 | 16 | — | — | MATCH |
| **`ProcMain__17dBgActorManager_cFv`** | **179** | **179** | 26/26 | 0xe0/0xe0 | **DIFFER (45 lines, all same-count)** |
| `addObj__17dBgActorManager_cFUsUsUsUc` | 27 | 27 | — | 0x10/0x10 | MATCH |
| `createObjList__17dBgActorManager_cFb` | 116 | 113 | 17/19 | 0x60/0x50 | DIFFER (−3, GPR pressure) |
| `init__Q217dBgActorManager_c7BgObj_cFv` | 8 | 8 | — | — | MATCH |
| `clear__Q217dBgActorManager_c7BgObj_cFv` | 1 | 1 | — | — | MATCH |
| `set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc` | 5 | 5 | — | — | MATCH |
| `createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c` | 60 | 60 | — | 0x40/0x40 | MATCH |
| `deleteActor__Q217dBgActorManager_c7BgObj_cFv` | 17 | 17 | — | 0x10/0x10 | MATCH |
| `getOffset__Q217dBgActorManager_c7BgObj_cFv` | 11 | 11 | — | — | MATCH |
| `getSize__Q217dBgActorManager_c7BgObj_cFv` | 9 | 9 | — | — | MATCH |

**13 MATCH, 3 DIFFER, 0 MISSING** — identical matched set to Round 21 (no GAINED, no LOST, see set diff below).

### Unit B: `d_bg_ctr.cpp` (NEW unit, 39 functions, 0x8007F7A0–0x80081070)

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
| `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c` | 25 | 25 | — | 0x20/0x20 | DIFFER (8 FPR-numbering lines) |
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
| `fn_80080670` | 130 | 1 | — | 0xb0/— | DIFFER (stub) |
| `fn_80080880` | 32 | 1 | — | — | DIFFER (stub) |
| `fn_80080900` | 256 | 1 | 20/— | 0x170/— | DIFFER (stub) |
| `upperRevCheck__9dBg_ctr_cFP8dActor_c` | 14 | 14 | — | — | MATCH |
| `underRevCheck__9dBg_ctr_cFP8dActor_c` | 14 | 14 | — | — | MATCH |
| `sideRevCheck__9dBg_ctr_cFP8dActor_cUc` | 14 | 14 | — | — | MATCH |
| `CheckRevUpperSpeed__9dBg_ctr_cFP8dActor_cP8dActor_c` | 6 | 6 | — | — | MATCH |
| `CheckRevUnderSpeed__9dBg_ctr_cFP8dActor_cP8dActor_c` | 6 | 6 | — | — | MATCH |
| `CheckRevSideSpeed__9dBg_ctr_cFP8dActor_cP8dActor_cUc` | 16 | 16 | — | — | MATCH |
| `fn_80080E40` | 121 | 1 | — | 0x20/— | DIFFER (stub) |
| `checkRevisionState__9dBg_ctr_cFUl` | 13 | 13 | — | — | MATCH |

**30 MATCH, 9 DIFFER, 0 MISSING.**

---

## 1. ProcMain — 179/179, full diff 73 → 45 (all count-neutral)

### The prompt's y0-negation suggestion: inapplicable — contradiction (AGENT_CONTEXT rule 5)

The suggestion was to "negate y0 in the preamble so the loop gets a ready-made negative operand for `add` instead of `subf`." That premise fails on both halves:

- **The `fneg` is misattributed.** There is no `fneg` in ProcMain's preamble. The only `fneg` in this TU's target range is at **0x8007E8A4, which is inside `createObjList`** (starts 0x8007E860). ProcMain spans 0x8007E520–0x8007E7EC and contains no `fneg`.
- **Negating y0 would change the loop's VALUE.** The loop computes `y0 - mY` (target `subf`/neg+add). A pre-negated `y0` in the preamble would make the loop compute `(-y0) + mY` ≠ `y0 - mY`. It is not a code-shape lever, it is a semantic change.

### The lever that actually closed neg+add: two-statement negation

MWCC folds `a + (-b)` → `a - b` → single `subf` when the negation is written in one expression. **The two-statement form survives the fold:**

```cpp
s32 ny = m_pObjList[i].mY;
ny = -ny;
pos.y = (f32)((int)((y0 + ny) << 4));
```

This emits the target's exact shape: `neg r0, r0; add r0, r28, r0` — the neg+add vs subf line item from Round 21 is **CLOSED**. The draft now also keeps `pos` declared at function top (`mVec3_c pos;` — claims the target's high slot 0x70), stores member-wise X-then-Y (target evaluates X first), and stores `pos.z = 0.0f;` **after** the offset additions (the z-store position and the neg+add were coupled: the draft's early z-store had been filling the slot the target's `add` occupies).

### Whole-struct viewMin/viewMax copy test (lwz/stw vs lfs/stfs) — negative

The prompt asked to test the preamble struct copy. Result: **the 28 preamble diff lines cannot be closed this round.**

- Target copies `viewMin`/`viewMax` via **`lwz`/`stw`** (integer path); draft emits **`lfs`/`stfs`** (float path).
- Root cause: `mVec3_c` has a **user-declared copy constructor**, which forces the float path. The `lwz/stw` path requires the ctor's removal — **proven negative: removing it regresses 160 functions** in the wider unit.
- Measured alternatives, both worse: `memcpy` copy → 175 words/163 diff lines; POD-cast copy → 181 words/168 diff lines. The count-neutral lwz/stw-vs-lfs/stfs selection is **not addressable by anything that keeps the count**; it needs the header change (out of scope, not mine to make alone).

### Residual 45 lines (composition)

- **28 preamble lines** (indices 15–45): lwz/stw-vs-lfs/stfs copy + interleave — blocked on the copy-ctor (above).
- **17 mMin/mMax lines** (indices 81–114): pure FPR-numbering — target `f2/f1/f0`, draft `f1/f0/f2`. Survived 3 measured variants (`sweep_min.py`): copy-then-modify, member-wise, and constructor forms all produce the same FPR rotation; per AGENT_CONTEXT the FPR declaration-order rule was applied and this numbering is the allocator's choice.

**Round 21 → Round 22 delta:** neg+add CLOSED (was one line in R21's 40; now absent); z-store position fixed; the remaining line count went 40 → 45 because the copy-then-modify mMin/mMax shape from R21 was re-measured against the new preamble layout — the 17 FPR lines were previously hidden inside R21's coarser buckets. Both units' matched sets are unchanged (see set diff).

---

## 2. `initialize` — left untouched, as instructed

66/66, `_savegpr`/frame identical. Only 2 diff lines, both symbol naming: `lis r31, l_object_name@ha` / `addi r31, r31, l_object_name@l` vs `SYM0@ha`/`SYM0@l`. Treat as matched; the fix is on the data-section/tooling side, not the source.

---

## 3. `d_bg_ctr.cpp` — NEW unit, seeded and 30/39 matched on first pass

### Boundary verified

Seeded via `python tools/auto_decomp/prepare.py --unit dol/bases/d_bg_ctr.cpp --range 0x8007F7A0-0x80081070`. prepare.py's split-object selection **over-extended** (the bundled `auto_03_8007F6D4_text.o` covers 0x8007F6D4–0x800870FC, several TUs), so `target.txt` was trimmed to the class span. End boundary checked by hand: last dBg_ctr_c function `checkRevisionState` ends at **0x80081064**, followed by `gap_03_80081064_text` (0xC pad), then **`__ct__11dBgGlobal_cFv` at 0x80081070 — correctly excluded**. 39 real functions (61 `.fn` incl. gaps) in the trimmed file.

### Ranked function list (largest first, target words)

| Rank | Function | Words | Frame | `_savegpr` |
|---|---|---|---|---|
| 1 | `fn_80080900` | 256 | 0x170 | 20 |
| 2 | `fn_80080670` | 130 | 0xb0 | — |
| 3 | `calc__9dBg_ctr_cFv` | 125 | 0x60 | — |
| 4 | `fn_80080E40` | 121 | 0x20 | — |
| 5 | `fn_8007FFA0` | 115 | 0x50 | 27 |
| 6 | `addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c` | 87 | 0x60 | — |
| 7 | `revisePos__9dBg_ctr_cFv` | 72 | 0x30 | — |
| 8 | `setOfs__9dBg_ctr_cFffffP7mVec3_c` | 51 | 0x50 | — |
| 9 | `getLinkNetPlayer__9dBg_ctr_cFSc` | 40 | — | — |
| 10 | `setLinkNetPlayer__9dBg_ctr_cFP5dBc_c` | 39 | — | — |
| 11 | `setLinkWallSlidPlayer__9dBg_ctr_cFP5dBc_c` | 39 | — | — |
| 12 | `set_common__9dBg_ctr_cFP8dActor_c…UcUc` | 36 | 0x10 | — |
| 13 | `set__9dBg_ctr_cFP8dActor_cffff…` | 33 | 0x30 | — |
| 14 | `fn_80080880` | 32 | — | — |
| 15 | `__dt__9dBg_ctr_cFv` | 27 | 0x10 | — |
| 16 | `set_circle__9dBg_ctr_cFP8dActor_cfff…` | 27 | 0x30 | — |
| 17 | `set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfo…` | 25 | 0x20 | — |
| 18 | `update__9dBg_ctr_cFv` | 22 | — | — |
| 19 | `__ct__9dBg_ctr_cFv` | 21 | 0x10 | — |
| 20 | `init__9dBg_ctr_cFv` | 21 | — | — |
| 21 | `release__9dBg_ctr_cFv` | 21 | — | — |
| 22 | `entry__9dBg_ctr_cFv` | 18 | — | — |
| 23 | `updateObjBg__9dBg_ctr_cFv` | 16 | 0x10 | — |
| 24 | `CheckRevSideSpeed__9dBg_ctr_cFP8dActor_cP8dActor_cUc` | 16 | — | — |
| 25 | `upperRevCheck__/underRevCheck__/sideRevCheck__` | 14 | — | — |
| 26 | `set__9dBg_ctr_cFP8dActor_c7mVec2_c7mVec2_c…` / `checkRevisionState__` | 13 | — | — |
| 27 | `setOfsX1__` / `setOfsY1__` | 11 | — | — |
| 28 | `setOfs__7mVec2_c…` / `setAngleY3__` | 7 | — | — |
| 29 | `reset__` / `CheckRevUpperSpeed__` / `CheckRevUnderSpeed__` | 6 | — | — |
| 30 | `setOfsX2__` / `setOfsY2__` | 5 | — | — |
| 31 | `__ct__7mVec2_cFv` | 1 | — | — |

### What's matched (30)

ctor/dtor (incl. `mVec2_c[4]` array construction via `__construct_array`), `reset`/`init`/`entry`/`release` (release ends with the target's **tail-call `b init`**), all `set`/`setOfs`/`set_circle`/`set_common`/`setAngleY3`, `update`/`updateObjBg`, both link-player set functions + `getLinkNetPlayer`, the full rev-check family, `checkRevisionState`. `set_common` stores the CheckRev fn addresses via `&dBg_ctr_c::CheckRevUpperSpeed` (static members — the member-pointer-cast route was illegal, "(10247) illegal explicit conversion").

### What's stubbed (8) + the one near-miss

`calc` (125w), `fn_8007FFA0` (115w), `revisePos` (72w), `addDokanMoveDiff` (87w), `fn_80080670` (130w), `fn_80080880` (32w), `fn_80080900` (256w), `fn_80080E40` (121w) are empty stubs. `set(sBgSetInfo)` is 25/25 with only **8 FPR-numbering lines** left (the inline `mVec2_c(info->f0, info->f4)` temp form; a named-temp version regressed to 29 words).

### Layout findings (derived from the disasm, committed to the shadow header)

- `mVec2_c mScratch[4]` @ **0x60** (ctor/dtor array — `__construct_array` with `__ct__7mVec2_cFv`/`__dt__7mVec2_cFv`).
- `mLinkNetPlayer[4]` @ 0x18–0x24, `mWallSlidPlayer[4]` @ 0x28–0x34 — **interleaved** init order (link[i], wall[i]); wall-slid is [4] not [2] (target clears four slots).
- `m_d8` is an **int** @ 0xD8, not padding (`update` does `m_d8 = m_d8 & 1`).
- Hidden 4-byte field @ 0xB8 — never touched by this unit; layout was 4 bytes low until `u8 mPadB8[4]` was added (offsetof-CHK array-size compile checks).
- `CheckRev*` are **static bool** members; `dActor_c+0xEC` = speed (upper/under), `+0xE8` = side.
- `entry()` uses flipped polarity (`if (mEntryPrev == nullptr)` — not `== mEntryB`).

### Next-round leads

- **`fn_80080900` (256w, frame 0x170)** is the single biggest target — also the best candidate for a `_savegpr_20` shape; probably worth one full decode pass before `calc`.
- **`calc` fully decoded** (register math worked out): circle path is a rotated-rect — `f13=cy*c, f12=cx*s, f9=cx*c, f8=cy*s, f5=oy*c, f4=oy*s, f3=ox*s, f2=ox*c`; corners → `mScratch[0..3]` @ 0x60–0x7F; `m_a0 = (corner0+corner2)*0.5`; calls `revisePos`; copies actor 0xAC/0xB0/0xB4 → 0x94/0x98/0x9C; `m_c0 = rot`. Rect path (`mMode==1`): `mPos = mCenter + mCenter`. Deferred only because the rotated-rect register matching needs many iterations.
- **`fn_8007FFA0` (115w, `_savegpr_27`)** is `revisePos`'s static helper — do them as a pair.
- `fn_80080880` abandoned for now: dtk prints `rlwinm r5, r0, 20, 28, 29` but the raw bytes `54 05 A7 3A` decode to different MB/ME — unresolved disasm ambiguity, unnamed helper.

---

## GAINED / LOST set diff vs Round 21 (by name)

- **`d_bg_actor_mng.cpp`: GAINED {} — LOST {}.** Matched set identical to Round 21 (the same 13 names above). ProcMain's 40→45 line recount and the neg+add closure did not move any function across the gate.
- **`d_bg_ctr.cpp`: GAINED = all 30 matched names** (new unit this round): `__ct__9dBg_ctr_cFv`, `__ct__7mVec2_cFv`, `__dt__9dBg_ctr_cFv`, `reset`, `init`, `entry`, `release`, `set_common`, `set(ffff)`, `set(mVec2,mVec2)`, `set_circle`, `setOfs(ffff)`, `setOfs(mVec2,mVec2)`, `setOfsX1`, `setOfsY1`, `setOfsX2`, `setOfsY2`, `setAngleY3`, `setLinkNetPlayer`, `getLinkNetPlayer`, `setLinkWallSlidPlayer`, `update`, `updateObjBg`, `upperRevCheck`, `underRevCheck`, `sideRevCheck`, `CheckRevUpperSpeed`, `CheckRevUnderSpeed`, `CheckRevSideSpeed`, `checkRevisionState`. **LOST {}.**

---

## poolcheck (new constant checker — run this round, both units)

```
mng: 2 pooled constants compared by VALUE across 16 paired functions
     0 mismatched, 0 could not be resolved on one side
ctr: 7 pooled constants compared by VALUE across 39 paired functions
     0 mismatched, 0 could not be resolved on one side
```

**The setOfs catch is real and worth knowing about:** raw-bytes AND canonicalised-text both called `setOfs` MATCHED while retail pooled **1.0f** and the draft pooled **0.0f** — both `lfs`, identical assembler bytes, and identical canonical text (both are the function's first pool reference). poolcheck caught the value disagreement; the draft was fixed to `mVec3_c v(1.0f, 1.0f, 1.0f);` and re-verified clean. This is the false-positive class poolcheck exists for.

---

## Deliverables & reproducibility

- Drafts: `scratch/round22/d_bg_actor_mng.cpp` (copied fresh this round), `scratch/round22/d_bg_ctr/d_bg_ctr.cpp` + shadow `scratch/round22/d_bg_ctr/shadow/`.
- Repro: `scratch/round22/build_draft.py` + `diff_all.py` (mng: 13/3/0), `scratch/round22/d_bg_ctr/build_ctr.py` + `diff_ctr.py` (ctr: 30/9/0), `full_procmain_diff.py` (179/179, 45), `fn_metrics.py mng|ctr` (per-function lengths/frames/savegpr — **fixed this round**; it was emitting all zeros), `poolcheck.py` (both clean).
- Constraints honored: no ninja/configure/progress/land run; no `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`, `GEMINI_*`, `HANDOFF*` touched; all work in `scratch/round22`.
