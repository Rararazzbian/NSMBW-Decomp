# Round 29 response

### calc
- Starting file: `scratch/round27/d_bg_ctr/calc_v1_decl_order.cpp`, copied to `scratch/round29/d_bg_ctr/calc_v1_decl_order.cpp`.
- Evidence: `cxs` held `cx * sin`, compiled as `fmuls f29, f5, f31`, then was consumed by the two Y-corner expressions. The unchanged copy reproduced the assigned baseline.
- Baseline compiled: YES. 125 words; saved GPRs `r31 r30`; saved FPRs `f31 f30 f29`; frame `0x50`. This is the expected v1 result.
- Change: removed `cxs` and inlined `cx * sin` at both uses (`mScratch[0].y` and `mScratch[1].y`).
- Variant 1 compiled: YES. 129 words; saved GPRs `r31 r30`; saved FPRs `f31 f30 f29 f28`; frame `0x60`. `calc` DIFF; frames agree with target, but instruction diff is displaced by the extra saved FPR.
- Variant 2 compiled: YES. Added separate `corner0Y` and `corner1Y` temporaries while keeping the product inline at each assignment. 129 words; saved GPRs `r31 r30`; saved FPRs `f31 f30 f29 f28`; frame `0x60`. `calc` DIFF; frames agree with target, but instruction diff is displaced by the extra saved FPR.
- Poolcheck: PASS. `7 pooled constants compared by VALUE`; `0 mismatched`, `0 could not be resolved`.
- Result: neither permitted live-range shape removed `f29`; both instead selected the baseline six-save shape including `f28`. Stopped after the two allowed additional shapes.
- GAINED: none (no matched-status change).
- LOST: none (no matched-status change).
- Offset-perturbing: NO. Only `scratch/round29/` artifacts and this report were changed; no source, headers, slices, or build configuration were touched.
