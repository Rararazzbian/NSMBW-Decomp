# Round 30 response

## Environment and baseline

- Starting file: `scratch/round30/d_bg_ctr/calc_v1_decl_order.cpp`, copied from the round29 draft with the required filename preserved.
- The disposable shadow declaration was restored at `scratch/round30/d_bg_ctr/shadow/game/bases/d_bg_ctr.hpp` from the prior round's scratch tree.
- Unchanged baseline and prior round artifacts were inspected. The prior baseline measurement was: `calc` 125 words, saved GPRs `r31 r30`, saved FPRs `f31 f30 f29`, frame `0x50`.
- Fresh round30 compilation was attempted through `tools/auto_decomp/harness.py::compile_draft` with the round30 shadow include. It fails before code generation because the current shared headers are incompatible with this old standalone draft. The first root diagnostics are in `scratch/round30/d_bg_ctr/calc_order.compile.log`: `d_bc.hpp` sees `dBg_ctr_c` before the shadow declaration, followed by conflicting Revolution matrix declarations. I did not modify shared headers or forbidden repository files.

## calc

### Store-before-call variant

- Evidence: the prior target stores the computed corner/intermediate values before `revisePos`; the round30 source was changed so the three post-call actor-position stores occur before `revisePos`.
- Source: `scratch/round30/d_bg_ctr/calc_store.cpp`.
- Expected: volatile FPR values should not need to survive the call, potentially removing the extra saved FPRs.
- Compiled: NO. The source was submitted to the standard harness, but compilation stopped on the pre-existing header/declaration errors described above; no word/register/frame measurement is attributable to this variant.
- Confidence: low for the unmeasured result; the instruction-level target observation is high, but the experiment could not be executed in this checkout.
- Offset-perturbing: NO at repository level; only scratch artifacts were written. If applied to the canonical TU, moving stores would be text/codegen-changing by design, not a section-layout change by itself.

### Trig-order variant

- Evidence: the target calls `CosFIdx` before `SinFIdx`; the source baseline calls sine first.
- Source: `scratch/round30/d_bg_ctr/calc_order.cpp`.
- Expected: changing call order should alter which computed value is parked in a callee-saved FPR.
- Compiled: NO, for the same pre-codegen header/declaration failure.
- Confidence: high in the source difference; no confidence claim is made about the unmeasured register result.
- Offset-perturbing: NO at repository level; scratch-only source experiment.

### Combined variant

- Evidence: combines the two preceding changes.
- Source: `scratch/round30/d_bg_ctr/calc_both.cpp`.
- Expected: store-before-call should remove unnecessary FPR saves, while cosine-first should select the target's surviving computed value.
- Compiled: NO, for the same pre-codegen header/declaration failure.
- Confidence: low for the unmeasured result.
- Offset-perturbing: NO at repository level; scratch-only source experiment.

- Diff count: not run for the round30 variants because no object was emitted. The preserved round29 baseline comparison remains `calc`: target 125, draft 125 instructions by canonical harness comparison, with the known saved-register/frame result above.
- Canonical fold: NOT performed. `calc` was not re-established as a freshly compiled round30 match, so there is no safe basis for creating or folding a canonical `d_bg_ctr.cpp`.
- GAINED: none (no matched-status change).
- LOST: none (no matched-status change).

## fn_80080670

- Evidence: the target split contains an unnamed `.fn fn_80080670, global` body at `scratch/round29/d_bg_ctr/target.txt:1251`; the prior draft emits the mangled member body `fn_80080670__9dBg_ctr_cFP7mVec3_cf` with 127 instructions, saved FPRs `f31 f30`, frame `0xB0`.
- The target's unnamed body is not name-paired by the preserved round29 harness, so a direct function diff requires an address/object-aware pairing that is not available in the current target text artifact. The target body visibly saves only GPRs `r31 r30` and has no target FPR save pair at its prologue, matching the assigned `-3` hypothesis.
- Store-before-call lens: considered against the target's circle branch, where vector components are written to stack before `PSVECMag`; however, because the current draft cannot compile and the target is unnamed in this split, no source variant could be measured honestly.
- Compiled: NO for a fresh round30 variant; blocked by the same header/declaration failure.
- Confidence: medium for the target saved-register observation, low for any proposed source fix. A fresh correctly configured shadow-header compile plus address-based pairing would settle it.
- Offset-perturbing: NO repository changes; no canonical source was touched.

## revisePos

- Evidence: target body is 72 instructions, frame `0x30`, saved GPRs `r31 r30 r29`, and the target stores the delta components to stack before calling the helper. The preserved draft is also 72 instructions, frame `0x30`, with the same saved GPR set.
- Store-before-call lens: the target already exhibits the store-before-call shape in the delta construction. No additional measured rewrite was possible because the standalone draft fails before code generation.
- Compiled: preserved prior artifact YES; fresh round30 attempt NO due to the header/declaration failure.
- Confidence: high that the prior result is codegen-neutral at 72/72; no new fix identified.
- Offset-perturbing: NO repository changes.

## addDokanMoveDiff

- Evidence: preserved target is 87 instructions, frame `0x60`, saved FPRs `f31 f30`; preserved draft is 80 instructions, frame `0x50`, saved FPRs `f31 f30 f29`. The target computes cosine first, stores the cosine result in `f31`, then computes sine and writes outputs; the draft has a different final shape and retains an extra callee-saved FPR.
- The one-word-near result referenced by the prompt was not reproducible from the currently available round29 target/draft pairing: the preserved artifacts measure target 87 versus draft 80 for this function. I did not manufacture a one-word claim.
- Store-before-call lens: no reliable source-level variant was measured because the round30 draft cannot compile under the current header state.
- Compiled: preserved prior object/disassembly YES; new round30 experiment NO.
- Confidence: high for the preserved counts/register/frame measurements; low for a proposed fix.
- Offset-perturbing: NO repository changes.

## Poolcheck

- The prior final object had a recorded poolcheck result of `7 pooled constants compared by VALUE`, `0 mismatched`, `0 unresolved`.
- A new final round30 object does not exist because all three `calc` variants fail compilation before object emission; therefore a new `poolcheck.py` run on a round30 final object is impossible. I did not report a false PASS.

## Final status

All requested targets were investigated and all permitted round30 source variants were authored in `scratch/round30/`. The requested fresh measurements and canonical fold remain incomplete because the supplied standalone draft is incompatible with the current shared-header include state, and the available target split does not directly name-pair `fn_80080670`. No forbidden files were touched, and no build/integration commands were run.
