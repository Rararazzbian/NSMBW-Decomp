# Round 27 response

All experiments below were compiled as fresh objects from `scratch/round27/d_bg_ctr/` using `harness.compile_draft`, then disassembled and diffed. No prohibited project-wide command was run. No `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`, `configure.py`, `GEMINI_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md` files were touched.

## GAINED / LOST

- GAINED: none.
- LOST: none.
- The final canonical-name object remains `32 MATCHED / 7 DIFFER / 0 MISSING`.
- `fn_80080880` remains matched.

## Results table

| Function | Target | Final draft | Target `_savegpr` | Draft `_savegpr` | Target frame | Draft frame | Diff |
|---|---:|---:|---|---|---|---|---:|
| `calc` | 125 | 129 | — | — | 0x60 | 0x60 | DIFFER (+4) |
| `fn_8007FFA0` | 115 | 107 | 27 | 27 | 0x50 | 0x60 | DIFFER (-8) |
| `revisePos` | 72 | 72 | 29 | 29 | 0x30 | 0x30 | DIFFER (ordering) |
| `addDokanMoveDiff` | 87 | 80 | — | — | 0x60 | 0x50 | DIFFER (-7) |
| `fn_80080670` | 130 | 127 | — | — | 0xB0 | 0xB0 | DIFFER (-3) |
| `fn_80080900` | 256 | 208 | 20 | 22 | 0x170 | 0xD0 | DIFFER (-48) |
| `fn_80080E40` | 121 | 117 | — | — | 0x20 | 0x20 | DIFFER (-4) |

The final object’s global diff report is `MATCHED: 32  DIFFER: 7  MISSING: 0`.

## fn_80080E40

Variant compiled: `round27_gate.cpp`, with the `mEntryFlag/mpActor` gate deleted, then rebuilt under the canonical `d_bg_ctr.cpp` filename to avoid filename-mangling effects.

- Compile: YES.
- Disassembly: YES.
- Result: target 121, draft 117; DIFFER (-4).
- The simple deletion was not sufficient. The target assigns `r31=idx`, `r30=dir`, `r29=this`, whereas the edited draft keeps `this` in `r3` and starts with `m_d4` at `0xD4`. The target first performs the `0xDC` byte test, then loads `this->0`, before checking `m_d4`; this requires a different source predicate/parameter-lifetime shape, not a bare deletion.
- Register file: GPR-only.

## calc variants

All three variants compiled and disassembled successfully. The unchanged baseline was also compiled in the final canonical object.

| Variant | Source change | Target | Draft | Result |
|---|---|---:|---:|---|
| baseline | unchanged round-26 source | 125 | 129 | DIFFER (+4) |
| `calc_v1_decl_order` | reversed declaration order of `sin`/`cos` locals | 125 | 125 | DIFFER (same length) |
| `calc_v2_named_reads` | reordered named local reads to target-like order | 125 | 125 | DIFFER (same length) |
| `calc_v3_split_assign` | split `cos`/`sin` declarations from assignments | 125 | 129 | DIFFER (+4) |

The 125-word variants still differ in instructions; matching length alone did not close `calc`. This confirms the residual is not just word count. The target’s callee-saved FPR assignment is `f30/f31`, so further work should combine declaration order, named temporaries in exact target READ order, and evaluation/read-side def-point shaping rather than treating these variants as equivalent.

## revisePos

Variant status: no new source variant was compiled before the round’s remaining time was exhausted. The final canonical object is the fresh round-27 object and reports target 72 / draft 72 with the same FPR-ordering residual as round 26.

Target read order is `0x9C`, actor `0xB4`, actor `0xB0`, `0x98`, actor `0xAC`, `0x94`; the next variant should encode those as named temporaries in precisely that order.

## fn_8007FFA0

Variant status: no new source variant was compiled this round. Fresh final-object measurement: target 115 / draft 107, target frame `0x50` with `_savegpr_27` and `f31`, draft frame `0x60` with `_savegpr_27` plus extra FPR saves.

Target explicitly stores intermediate accumulated values at `r1+0x10` and `r1+0x14` before parent accumulation. The existing draft’s trig swap remains; the remaining deficit is real body/lifetime content, not a reason to revert it.

## addDokanMoveDiff

Variant status: no new source variant was compiled this round. Fresh final-object measurement: target 87 / draft 80, target frame `0x60`, draft frame `0x50`.

Target retains the square-root length in `f30`, uses `f31` for the trigonometric result, computes corrected components, and then performs the final output cosine/sine pair. The existing draft does not preserve the same temporary/store lifetime.

## fn_80080900

Variant status: no new source variant was compiled this round; the requested liveness analysis was already completed in round 26 and remains the bounded result. Fresh final-object measurement: target 256 / draft 208, target frame `0x170` with `_savegpr_20`, draft frame `0xD0` with `_savegpr_22`.

Target live values:

| Value | Target location | Lifetime/use |
|---|---|---|
| `angleOut` | `r20` | Across geometry calls and loop; written on a hit. |
| flags/mode | `r22` | Across branch and four-edge loop. |
| edge index | `r23` | Counts 0..3 and indexes corners. |
| found | `r24` | Set on hit and converted at return. |
| selected corner A/B | `r26/r25` | Live through filtered edge loop. |
| corner buffer | `r27` -> `r1+0xF8` | Advances by `0xC` per edge. |
| edge end/start | `r28/r29` -> `r1+0xE0`/`r1+0xEC` | Live across `DistSqSegment3ToSegment3` and angle calculation. |
| hit-result vector | `r30` -> `r1+0x14` | Receives segment delta and feeds output. |
| flags/edge state | `r31` -> `r1+0x44` | Live across second geometry path and angle calculation. |
| four corners | `r1+0xF8..0x124` | Four `VEC3` values copied before loop. |
| edge segment | `r1+0xE0..0xF4` | Two `VEC3` endpoints rebuilt per selected edge. |
| geometry results | `r1+0x14..0x20`, `0x44..0x4C`, `0x8C..0x94` | Separate vectors retained for output/angle. |

No converged object was produced for this function. The table remains the actionable result; the next attempt should explicitly declare these objects and compile once, then report its actual frame and `_savegpr` level.

## fn_80080670

Variant status: no new source variant was compiled this round. Fresh final-object measurement: target 130 / draft 127, both frame `0xB0`; target saves GPR `r31/r30`, while draft saves extra FPRs. This remains last priority.

## poolcheck

The copied `poolcheck.py` cannot run against the current harness: it calls `harness.poolcheck`, which raises `AttributeError: module 'harness' has no attribute 'poolcheck'`. No replacement poolcheck entry point was available in the round workspace. This is reported as a tooling blocker rather than a fabricated result.
