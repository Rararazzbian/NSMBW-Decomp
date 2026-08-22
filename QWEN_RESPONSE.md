# Round 26 response

Baseline was compiled from `scratch/round26/d_bg_ctr/d_bg_ctr.cpp` with the existing harness and shadow header. No prohibited project-wide command was run. `diff_ctr.py` reproduces round 25 exactly: `MATCHED: 32  DIFFER: 7  MISSING: 0`.

## Summary

- GAINED: none in this round.
- LOST: none in this round.
- The baseline remains 32 matched / 7 differing. I did not claim an unverified improvement.
- `fn_80080880` remains matched.
- No shared source, header, slice, symbol, or integrator file was changed.

## Per-function table

| Function | Target | Draft | `_savegpr` (T/D) | Frame (T/D) | Result |
|---|---:|---:|---|---|---|
| `calc` | 125 | 129 | —/— | 0x60/0x60 | DIFFER (+4) |
| `fn_8007FFA0` | 115 | 107 | 27/27 | 0x50/0x60 | DIFFER (-8) |
| `revisePos` | 72 | 72 | 29/29 | 0x30/0x30 | DIFFER (same size, FPR ordering) |
| `addDokanMoveDiff` | 87 | 80 | —/— | 0x60/0x50 | DIFFER (-7) |
| `fn_80080670` | 130 | 127 | —/— | 0xB0/0xB0 | DIFFER (-3) |
| `fn_80080900` | 256 | 208 | 20/22 | 0x170/0xD0 | DIFFER (-48) |
| `fn_80080E40` | 121 | 124 | —/— | 0x20/0x20 | DIFFER (+3) |

The other 32 target functions remain matched; there are no missing functions.

## calc

- Target length: 125 words.
- Draft length: 129 words.
- Target and draft frame: `0x60`.
- Register file: **callee-saved FPRs**, specifically target `f30/f31` versus draft `f28/f29`; target also saves GPR `r30/r31`.
- Evidence: target `calc` at `target.txt:579`; target saves `f31/f30` and uses `f31` for the sine result and `f30` for the `0.5f` constant. The draft `calc` is at `draft_disasm.txt:1128` and saves/uses `f29/f28`.
- Proposal: try, in order, declaration order of float locals; named temporaries in target READ order; split declarations from assignments so declaration controls callee-saved FPR numbering while assignment controls computation point; then combine evaluation order and read-side def-point ordering.
- Confidence: high that the residual is source-addressable; low on the exact declaration/assignment arrangement until compiled.
- Compiled: no new variant; baseline only.
- Offset-perturbing: NO expected; local declaration/order changes affect `.text` only and do not add object data or calls.

## revisePos

- Target length: 72 words.
- Draft length: 72 words.
- Target/draft frame: `0x30`; `_savegpr_29` in both.
- Evidence: target reads `0x9C`, actor `0xB4`, actor `0xB0`, then `0x98`, actor `0xAC`, and `0x94`, producing `f2=f3-f2` and `f1=f1-f0`. The draft reads the same values in a different FPR assignment/order.
- Proposal: use named temporaries in target READ order and retain the target statement/evaluation order, the same read-order lever that closed `set(sBgSetInfo)`.
- Confidence: high; this is a pure ordering residual.
- Compiled: no new variant; baseline only.
- Offset-perturbing: NO; only local expression/read order changes.

## fn_8007FFA0

- Target length: 115 words.
- Draft length: 107 words.
- Target prologue: frame `0x50`, `_savegpr_27`, FPR `f31`.
- Draft prologue: frame `0x60`, `_savegpr_27`, FPR `f31/f30/f29`.
- Evidence: target begins with stack accumulators at `r1+0x18`/`r1+0x1C`, keeps `ctr` in `r29`, actor in `r27`, mode in `r28`, and `pos` in `r31`; target explicitly stores intermediate values at `r1+0x10` and `r1+0x14` before parent accumulation. The draft retains extra FPR saves and has the wrong temporary/store sequence.
- Proposal: do not revert the trig swap. Reconstruct the missing source-level lifetimes and stores, especially the target's explicit intermediate stores and mode-2 zero-X test over the accumulated X result, rather than adding padding expressions.
- Confidence: medium; missing body content, not just register-choice padding, explains the remaining 8 words.
- Compiled: no new variant; baseline only.
- Offset-perturbing: NO expected for local temporaries; YES only if a shared header/layout is changed, which was not proposed.

## addDokanMoveDiff

- Target length: 87 words.
- Draft length: 80 words.
- Target prologue: frame `0x60`, saves `f31/f30` and GPR `r31/r30/r29`.
- Draft prologue: frame `0x50`, with a different saved-register set.
- Evidence: target computes `sqrt` into `f30`, computes angle, retains `f31` as cosine, computes corrected components, then performs separate cosine/sine calls for the output. The draft does not reproduce the target's complete temporary/store chain.
- Proposal: model distinct values `dx`, `dy`, `length` (callee-saved `f30`), cosine/sine for corrected components, then a second cosine/sine pair for output. Do not revert the trig swap; the missing 7 words are real body content.
- Confidence: medium.
- Compiled: no new variant; baseline only.
- Offset-perturbing: NO expected; locals only.

## fn_80080E40

- Target length: 121 words.
- Draft length: 124 words.
- Target/draft frame: `0x20`.
- Register file: **GPR only**, not FPR. Target assigns `r31=idx`, `r30=dir`, `r29=this`, and saves `r28-r31`.
- Evidence: target starts at `target.txt:1851` with the `0xDC` byte test, then `this->0`, `m_d4` at `0xD4`, `bc->0xE5`, owner pointers, type, and flag tables. The draft starts with `if (!mEntryFlag || mpActor == nullptr) return false;`, emitting an extra initial gate absent from target.
- Proposal: remove the draft-only `mEntryFlag/mpActor` gate and express the target's first predicate in target order while preserving the target GPR parameter roles.
- Confidence: high that this is source-addressable; the concrete draft-only gate accounts for the positive length residual.
- Compiled: no new variant; baseline only.
- Offset-perturbing: NO; function-local source only.

## fn_80080900

- Target length: 256 words.
- Draft length: 208 words.
- Target prologue: frame `0x170`, `_savegpr_20`, FPR `f31`.
- Draft prologue: frame `0xD0`, `_savegpr_22`, FPR `f31`.
- Diff: target is 48 words longer and uses `0xA0` more stack.
- Result of timebox: no converged variant; stop guessing at frame sizes/register pressure.

### Target value-liveness table

| Value | Register/stack | Lifetime/use |
|---|---|---|
| `angleOut` pointer | `r20` | Preserved across both geometry calls and all loop iterations; written on a hit. |
| `flags` / mode selector | `r22` | Preserved across sphere/rect branch and the four-edge loop; may be forced to zero after endpoint-X comparison. |
| corner loop index | `r23` | Counts `0..3`; advances every iteration and indexes the corner buffer. |
| `found` | `r24` | Result flag; set after a valid edge hit and converted to boolean at return. |
| selected corner A | `r26` | Chosen from rotation and segment-Y ordering; live through the edge loop. |
| selected corner B | `r25` | Derived from A; live through the edge loop. |
| corner-buffer pointer | `r27` | Points at stack buffer `r1+0xF8`; advances by `0xC` each iteration. |
| edge-end pointer | `r28` | Points at `r1+0xE0`; used as the second segment endpoint across geometry and angle calculation. |
| edge-start pointer | `r29` | Points at `r1+0xEC`; used as the first segment endpoint across geometry and angle calculation. |
| hit-vector/result pointer | `r30` | Points at `r1+0x14`; receives segment delta and is read for angle/output. |
| flags/edge state | `r31` | Points at `r1+0x44`; remains live across the second geometry path and angle calculation. |
| sphere input | stack `0xC0..0xCC` | Four floats copied before `IntersectionSegment3Sphere`; survives the call. |
| four corners | stack `0xF8..0x124` | Twelve floats, four `VEC3`s, copied before the loop and indexed by `r27`. |
| edge segment | stack `0xE0..0xF4` | Two `VEC3` endpoints rebuilt each selected iteration. |
| hit/intersection vectors | stack `0x14..0x20`, `0x44..0x4C`, `0x8C..0x94` | Separate geometry results retained for output and angle computation. |

The target therefore needs at least the five simultaneously-live pointer/state roles called out in the work order (`r27` corner buffer, `r28` edge end, `r29` edge start, `r30` result vector, `r31` flags/state), plus `r20/r22-r26` and several distinct stack geometry objects. The current draft's `0xD0` frame cannot represent the target's stack objects and is not a register-allocation accident.

- Proposal: next pass should declare explicit four-corner, edge-start/end, and hit-result objects with lifetimes spanning the two geometry calls. If one such pass does not converge, retain this table and stop.
- Confidence: high on the liveness facts; low on the exact C++ aggregate/declaration form.
- Compiled: baseline only.
- Offset-perturbing: NO for a local-only reconstruction; YES if geometry declarations are moved into a shared header, so keep any declaration proposal in `scratch/`.

## fn_80080670

- Target length: 130 words.
- Draft length: 127 words.
- Target prologue: frame `0xB0`, saves only GPR `r31/r30`.
- Draft prologue: frame `0xB0`, but saves FPR `f31/f30` in addition to GPRs.
- Diff: 3 words short overall; target has no FPR save/restore.
- Evidence: target `fn_80080670` starts at `target.txt:1252` and saves `r31/r30`; the draft emits FPR saves because its rotated-rectangle locals keep extra float values live.
- Proposal: reduce float local lifetime using target-ordered expressions and stack-backed matrix/vector temporaries while preserving `ZrotS`, `PSMTXTrans`, `PSMTXConcat`, and `multVecZero` call order.
- Confidence: medium.
- Compiled: no new variant; baseline only.
- Offset-perturbing: NO expected.

## poolcheck

`poolcheck.py` was not run as a final new-variant check because no new compiled variant was produced. The copied baseline artifacts are unchanged from round 25. The next actionable work is the bounded source-shape reconstruction above, especially `calc`, `revisePos`, and the explicit `fn_80080900` liveness model.
