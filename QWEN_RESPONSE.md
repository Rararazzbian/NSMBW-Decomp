# Round 32 report

## T0 target table

Extracted directly from `scratch/round32/d_bg_ctr/target.txt`:

| Function | target words | target frame | target GPR saves | target FPR saves |
|---|---:|---:|---|---|
| `calc__9dBg_ctr_cFv` | 125 | 0x60 | none | [30,31] |
| `fn_8007FFA0` | 115 | 0x50 | [27] | none |
| `revisePos__9dBg_ctr_cFv` | 72 | 0x30 | none | none |
| `addDokanMoveDiff__9dBg_ctr_cFP7mVec3_c` | 87 | 0x60 | none | none |
| `fn_80080670` | 130 | 0xB0 | none | none |
| `fn_80080900` | 256 | 0x170 | [20] | none |
| `fn_80080E40` | 121 | 0x20 | none | none |

## Work performed

Copied prior scratch sources into `scratch/round32/d_bg_ctr/`, carried `build.py` forward, and repointed `diff_ctr.py` `BASE` to round32. The copied variants were freshly compiled/disassembled with the existing harness. Fresh measurements are in `round32_metrics.txt`.

These were inherited experiments, not new source variants authored during round32. They are therefore marked `NO` in the table and do not count toward minimums.

### T1 - revisePos

`revise_order.cpp`: 72 words, frame 0x30, GPR [29,30,31], FPR none. Target is 72/0x30/no GPR/no FPR. No actual diff list was generated because the copied diff tool requires `draft_disasm.txt`, and no canonical draft was built. Required diff list and >=3 new variants were not delivered.

### T2 - calc

- `calc_baseline.cpp`: 125 words, 0x50, GPR [30,31], FPR [29,30,31]
- `calc_decl_f29.cpp`: 139 words, 0x90, GPR [30,31], FPR [25,26,27,28,29,30,31]
- `t2_baseline.cpp`: 125 words, 0x50, GPR [30,31], FPR [29,30,31]
- `t2_local_vec.cpp`: 125 words, 0x50, GPR [30,31], FPR [29,30,31]

Target is 125/0x60/no GPR/FPR [30,31]. No new round32 variants were authored. Required >=2 new variants were not delivered.

### T3 - fn_80080670

No source variant was compiled for this function. Required diff list and >=2 new variants were not delivered.

### T4 - addDokanMoveDiff

- `target_math_dokan.cpp`: 88 words, 0x60, GPR [29,30,31], FPR [29,30,31]
- `dokan_target_shape.cpp`: 69 words, 0x50, GPR [29,30,31], FPR [29,30,31]
- `dokan_lifetimes.cpp`: 72 words, 0x60, GPR [30,31], FPR [28,29,30,31]

Target is 87/0x60/no GPR/no FPR. No diff list or new round32 variants were delivered.

### T5 - fn_80080E40

- `filter_dc_first.cpp`: 122 words, 0x20, GPR [29,30,31], FPR none
- `t5_baseline.cpp`: 117 words, 0x20, GPR [29,30,31], FPR none
- `t5_stores.cpp`: 117 words, 0x20, GPR [29,30,31], FPR none

Target is 121/0x20/no GPR/no FPR. No new round32 variants were authored. Required >=2 new variants were not delivered.

### T6 - fn_8007FFA0

No round32 source variant was measured. Required >=2 variants were not delivered.

### T7 - fn_80080900

`fn809_liveness.cpp`: 208 words, 0xD0, GPR [22], FPR [31]. Target is 256/0x170/GPR [20]/no FPR. This was a recompiled inherited source, not a new round32 attempt, so acceptance was not met.

### T8 - canonical fold

Not reached. No function matched, so no canonical `d_bg_ctr.cpp`, `draft_disasm.txt`, or poolcheck run was produced.

## GAINED / LOST

No matched-status changes. No function changed from non-matching to matching or vice versa.

## NOT REACHED

- T1 diff list and three new variants.
- T2 two new variants.
- T3 diff list and two new variants.
- T4 diff list and two new variants.
- T5 two new variants.
- T6 two new variants.
- T7 one new acceptance-quality attempt.
- T8 conditional canonical fold and poolcheck.

The limiting issue was running out of execution budget before authoring and measuring the required new experiments. Inherited sources were recompiled and measured, but are explicitly not presented as new work.

## Compliance table

| Task | Variant file | New this round? | Words (T/D) | Frame (T/D) | GPR saves (T/D) | FPR saves (T/D) | Diffs | required | delivered | met? |
|---|---|---|---|---|---|---|---|---|---|---|
| T0 | `target.txt` | YES | 7 target rows | extracted | extracted | extracted | n/a | seven rows | seven rows | YES |
| T1 | `revise_order.cpp` | NO | 72/72 | 0x30/0x30 | none/[29,30,31] | none/none | not generated | diff list + >=3 new variants | 1 recycled | NO |
| T2 | four calc variants | NO | target 125; drafts 125/139 | target 0x60; drafts 0x50/0x90 | target none; drafts [30,31] | target [30,31]; drafts [29,30,31]/six saves | n/a | >=2 new variants | 4 recycled | NO |
| T3 | none | NO | 130/not measured | 0xB0/not measured | none/not measured | none/not measured | not generated | diff list + >=2 new variants | none | NO |
| T4 | three dokan variants | NO | 87/88,69,72 | 0x60/0x60,0x50,0x60 | none/[29,30,31], [29,30,31], [30,31] | none/[29,30,31], [29,30,31], [28,29,30,31] | not generated | diff list + >=2 new variants | 3 recycled | NO |
| T5 | three filter variants | NO | 121/122,117,117 | 0x20/0x20 | none/[29,30,31] | none/none | not generated | >=2 new variants | 3 recycled | NO |
| T6 | none | NO | 121/not measured | 0x50/not measured | none/not measured | none/not measured | n/a | >=2 new variants | none | NO |
| T7 | `fn809_liveness.cpp` | NO | 256/208 | 0x170/0xD0 | [20]/[22] | none/[31] | n/a | one compiled attempt + report | 1 recycled | NO |
| T8 | none | NO | n/a | n/a | n/a | n/a | n/a | conditional canonical + poolcheck | no match, not applicable | NO |

No prohibited command (`ninja`, `configure.py`, `progress.py`, or `land.py`) was run. No prohibited source, include, slice, symbol, wip, handoff, or GEMINI file was modified.
