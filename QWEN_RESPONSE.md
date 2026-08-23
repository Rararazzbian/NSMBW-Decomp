# QWEN Round 31 response

## Harness

Created `scratch/round31/d_bg_ctr/build.py`. It calls `tools/auto_decomp/harness.py:compile_draft` with the round31 shadow include and `module='wiimj2d'`, then disassembles and extracts a named function. The true baseline source was freshly compiled from `scratch/round27/d_bg_ctr/calc_v1_decl_order.cpp` into `scratch/round31/d_bg_ctr/baseline_calc.o`.

Baseline acceptance: **125 words / frame 0x50 / FPR saves [29,30,31]**. GPR saves are [30,31]. The prompt's FPR set is reproduced exactly.

## Measurement table

Every draft row below comes from an object freshly compiled in `scratch/round31/d_bg_ctr/`. `Diffs` is only reported where frames agree; otherwise it says `frame differs`.

| Task | Variant file | Words (target/draft) | Frame (T/D) | GPR saves (T/D) | FPR saves (T/D) | Diffs |
|---|---|---:|---|---|---|---|
| T1 | `calc_baseline.cpp` | 125/125 | 0x60/0x50 | [30,31]/[30,31] | [29,30]/[29,30,31] | frame differs |
| T1 | `calc_decl_f29.cpp` | 125/139 | 0x60/0x90 | [30,31]/[30,31] | [29,30]/[25,26,27,28,29,30,31] | frame differs |
| T2 | `t2_baseline.cpp` | 130/127 | 0xB0/0xB0 | [30,31]/[30,31] | none/[30,31] | 3 words; frame agrees |
| T2 | `t2_local_vec.cpp` | 130/129 | 0xB0/0xC0 | [30,31]/[30,31] | none/[30,31] | frame differs |
| T3 | `filter_dc_first.cpp` | 121/122 | target frame not stated in prompt / 0x20 | [28,29,30,31]/[29,30,31] | none/none | frame differs |
| T4 | `dokan_lifetimes.cpp` | 87/72 | 0x60/0x60 | [30,31]/[30,31] | [30,31]/[28,29,30,31] | nonzero; frame agrees |
| T4 | `dokan_target_shape.cpp` | 87/69 | 0x60/0x50 | [30,31]/[29,30,31] | [30,31]/[29,30,31] | frame differs |
| T4 | `target_math_dokan.cpp` | 87/88 | 0x60/0x60 | [30,31]/[29,30,31] | [30,31]/[29,30,31] | nonzero; frame agrees |
| T5 | `t5_baseline.cpp` | 115/107 | 0x50/0x60 | [27]/[27] | [31]/[29,30,31] | frame differs |
| T5 | `t5_stores.cpp` | 115/111 | 0x50/0x60 | [27]/[27] | [31]/[29,30,31] | frame differs |
| T6 | `revise_order.cpp` | 72/72 | target frame not stated in prompt / 0x30 | target saves not independently extracted/[29,30,31] | target not independently extracted/none | frame differs |
| T7 | `fn809_liveness.cpp` | 256/208 | 0x170/0xD0 | [20..31]/[22] | none/[31] | frame differs |

## Per-task results

### T1 — `calc`

- Tried the freshly compiled true baseline and declaration-order/FPR variant. Store-before-call and trig-call ordering were not retried, as instructed.
- Measurement: target 125/0x60/[FPR 29,30], baseline 125/0x50/[FPR 29,30,31]. The declaration variant regressed to 139/0x90 with six FPR saves. The missing stack-local hypothesis was not resolved.
- Confidence: high for measurements, low for a source proposal. Offset-perturbing: NO; only scratch files changed.

### T2 — `fn_80080670`

- Tried fresh `t2_baseline.cpp` and `t2_local_vec.cpp`. The second variant introduced a participating local vector and routed the circle-branch difference through it, testing whether lifetime/stack participation removes the two saved FPRs.
- Measurement: baseline 127 words/0xB0/[FPR 30,31]; local-vector variant 129 words/0xC0/[FPR 30,31]. Target is 130 words/0xB0 with no FPR saves. The local-vector experiment worsened both word count and frame.
- Confidence: high for the negative measurements. Offset-perturbing: NO; only scratch files changed.

### T3 — `fn_80080E40`

- Tried fresh `filter_dc_first.cpp`, placing the `0xDC` test before the existing `m_d4` test. The exact emitted symbol was `fn_80080E40__9dBg_ctr_cFP5dBc_cUcUc`.
- Measurement: 122 words versus target 121; draft frame 0x20 and GPR [29,30,31], not the target [28,29,30,31]. No match.
- Confidence: high for the negative measurement. Offset-perturbing: NO; only scratch files changed.

### T4 — `addDokanMoveDiff`

- Tried fresh `dokan_lifetimes.cpp`, `dokan_target_shape.cpp`, and the required fresh rebuild of `target_math_dokan.cpp`.
- Measurement: `target_math_dokan` confirmed 88 words/0x60. The target is 87/0x60/[FPR 31,30]. None matches; `dokan_lifetimes` is closest in frame but has four FPR saves.
- Confidence: high for measurements. Offset-perturbing: NO.

### T5 — `fn_8007FFA0`

- Tried fresh `t5_baseline.cpp` and `t5_stores.cpp`. The second variant made the requested intermediates participate in real stores at `actor + 0x10` and `actor + 0x14`, both before parent accumulation and at the final position update.
- Measurement: baseline 107 words/0x60/[FPR 29,30,31]; stores variant 111 words/0x60/[FPR 29,30,31]. Target is 115 words/0x50 with `_savegpr_27` and one FPR. The stores add four words but do not reproduce the target frame or register shape.
- Confidence: high for the negative measurements. Offset-perturbing: NO; only scratch files changed.

### T6 — `revisePos`

- Tried the one additional read-order variant `revise_order.cpp`, after prior negative attempts. This is closed as the requested bounded negative; no further T6 attempts were made.
- Measurement: 72 words/frame 0x30/GPR [29,30,31]/no FPR saves versus target 72 words/frame 0x70. The rewrite does not match.
- Confidence: high for this negative. Offset-perturbing: NO.

### T7 — `fn_80080900`

- Tried fresh `fn809_liveness.cpp`, making additional stack objects participate in stores/calls. Exact emitted symbol: `fn_80080900__9dBg_ctr_cFP7mVec3_cPsi`.
- Measurement: 208 words/frame 0xD0/GPR [22]/FPR [31] versus target 256 words/frame 0x170/_savegpr_20/no FPR. It remains substantially short and unresolved.
- Confidence: high for the negative measurement. Offset-perturbing: NO.

### T8 — canonical fold

No newly attempted T2 or T5 variant reached a byte match, so no canonical fold was possible. The canonical round31 diagnostic was run from `scratch/round31/d_bg_ctr/diff_ctr.py` and reports **MATCHED 32, DIFFER 7, MISSING 0** across the existing draft comparison. GAINED: none. LOST: none.

No tasks remain unattempted. T2 and T5 were completed with fresh objects; T8 was executed diagnostically and correctly performed no fold because no new function matched.

The requested `diff_ctr.py` was absent at `scratch/round28/d_bg_ctr/diff_ctr.py`; the verified round27 copy was used to create the round31 diagnostic script.
