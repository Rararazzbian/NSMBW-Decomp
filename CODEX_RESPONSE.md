# Codex Round 7 response

## Task A: EGG::Effect virtual count

Result: no discrepancy. The prompt premise was stale.

- Evidence: `__vt__Q23EGG6Effect` is `.data:0x80350AF8` with size `0x9C`
  (`bin/dtk/wiimj2d_symbols.txt`). `(0x9C - 8) / 4 = 37`.
- The vtable object contains exactly 37 function-pointer slots after the two
  leading zero words.
- `include/lib/egg/util/eggEffect.hpp` declares 37 virtuals, including the
  destructor, on lines 13-49.
- Both lists match one-to-one in declaration order. No missing slots and no
  pure-virtual stub.
- The prior 35-vs-37 claim was a miscount. `git show 902a0b3b:include/lib/egg/util/eggEffect.hpp`
  already has 37 virtuals, so this is not an uncommitted working-tree edit either.

Impact: no insertion is needed, so no override in any derived class moves.

Compiled: n/a, no edit proposed.
Confidence: high.
Offset-perturbing: NO, because nothing changes.

## Task B: EGG::Effect 0x08..0x23

Result: negative. Neither derived constructors nor the base lifecycle virtuals
access the 28 bytes at `0x08..0x23`, so labelled padding remains the safest
proposal.

Constructor angle:
- `EGG::Effect::Effect()` writes `0x00` (vtable), `0x04` (byte zero), `0x24`
  (u32 zero), `0x28` (u32 zero), and constructs embedded subobjects at `0x74`
  (`nw4r::ef::HandleBase`) and `0x7C` (`ExEffectParam`).
- `dEf::followEffect_c` ctor writes only its own vtable; no base-range stores.
- `mEf::levelEffect_c` ctor writes only its own vtable and its own tail fields
  `0x114..0x127`; no base-range stores.
- `dPyEffect_c` is special: it invokes `EGG::Effect` at original offset `0x04`,
  so the base byte store at subobject `0x04` lands at original object offset
  `0x08`. That is an artifact of the embedded layout, not an independent
  `EGG::Effect` field at `0x08`.

Lifecycle angle:
- `create`, `fade`, `followFade`, `kill`, `update`, and `reset`, plus the
  `mEf::effect_c` helpers, have no load or store at `0x08..0x23`.
- Confirmed accesses around it: `create` loads `0x24` (effect resource id);
  `update` reads `0x28` flags and `0x2C..0x40` scale/translation; `reset`
  clears `0x28` and sets scale to `1.0f` / translation to `0.0f`.

Proposal for `0x08..0x23`:

    u8 pad[0x1C]; // 0x08..0x23, unobserved; keep as labelled padding

`sizeof` stays `0x114`.

Confidence: high that no examined function touches the region (negative result);
low that a semantic field exists there. What would raise it is a broader
cross-TU store scan or an upstream reference, both outside this round.

Offset-perturbing: NO, as long as the region stays one `0x1C` padding block.

## Files

- `scratch/codex_round7/task_a_vtable_audit.md`
- `scratch/codex_round7/task_b_ctor_offsets.md`
- `scratch/codex_round7/task_c_lifecycle_offsets.md`

## Notes for Claude

- Task A is already settled in the working tree; nothing to apply.
- Task B is a stronger negative than round 6: both requested angles found no
  access, so keep `0x08..0x23` as padding unless another access source exists.