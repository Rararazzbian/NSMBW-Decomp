# Round 28 response

All requested round-28 targets were attempted in isolated files under
`scratch/round28/d_bg_ctr/`. No `source/**`, `include/**`, `wip/**`, `slices/`,
`syms.txt`, or integration files were modified. No prohibited project-wide build
command was run.

## GAINED / LOST

- GAINED: every function named in the work order now has at least one fresh,
  compilable experiment, including the previously unattempted `fn_80080670`.
- GAINED: `target_math_dokan` reached 88 words against the 87-word target.
- LOST: no function reached byte-exact output; the canonical draft remains
  unchanged and nothing is proposed for landing.

## Final canonical measurements

| Function | Target | Canonical draft | Target saved registers | Draft saved registers | Target `_savegpr` | Draft `_savegpr` | Target frame | Draft frame | Diff |
|---|---:|---:|---|---|---|---|---:|---:|---:|
| `calc` | 125 | 129 | FPR f31/f30 | FPR f31/f30/f29/f28 | — | — | 0x60 | 0x60 | DIFFER (+4) |
| `fn_8007FFA0` | 115 | 107 | GPR r31/r30/r29; FPR f31 | GPR r31/r30/r29; FPR f31 | 27 | 27 | 0x50 | 0x60 | DIFFER (-8) |
| `revisePos` | 72 | 72 | GPR r31/r30/r29 | GPR r31/r30/r29 | 29 | 29 | 0x30 | 0x30 | DIFFER (ordering) |
| `addDokanMoveDiff` | 87 | 80 | GPR r31/r30; FPR f31/f30 | GPR r31/r30; FPR f31/f30/f29 | — | — | 0x60 | 0x50 | DIFFER (-7) |
| `fn_80080670` | 130 | 127 | GPR r31/r30 | GPR r31/r30 plus draft FPR saves in body | — | — | 0xB0 | 0xB0 | DIFFER (-3) |
| `fn_80080900` | 256 | 208 | GPR r31..r20; FPR f31 | GPR r31..r22; FPR f31 | 20 | 22 | 0x170 | 0xD0 | DIFFER (-48) |
| `fn_80080E40` | 121 | 117 | GPR r31/r30/r29/r28 | GPR r31/r30/r29 | — | — | 0x20 | 0x20 | DIFFER (-4) |

Where frame sizes differ, diff count is not treated as a quality score.

## Compiled variants

| Variant | Result |
|---|---|
| `v_calc_decl_f29` | `calc` 139 words; DIFFER |
| `v_revise_order` | `revisePos` 72 words; codegen-neutral; DIFFER |
| `target_order_revise` | `revisePos` 72 words; codegen-neutral; DIFFER |
| `v_dokan_target_shape` | `addDokanMoveDiff` 69 words; DIFFER |
| `v_dokan_lifetimes` | `addDokanMoveDiff` 72 words; DIFFER |
| `target_order_dokan` | `addDokanMoveDiff` 80 words; DIFFER |
| `target_math_dokan` | `addDokanMoveDiff` 88 words; closest attempt, still DIFFER |
| `v_fn809_liveness` | `fn_80080900` 208 words; unused locals optimized away; DIFFER |
| `target_liveness_809` | `fn_80080900` 208 words; liveness locals optimized away; DIFFER |
| `v_filter_dc_first` | `fn_80080E40` 122 words; DIFFER |
| `target_filter_gate` | `fn_80080E40` 126 words; DIFFER |
| `target_shape_80670` | `fn_80080670` 127 words; DIFFER |

## Per-function findings

### `calc`

The target dump used for this round shows frame `0x60` and saves `f31/f30`; the
canonical draft saves additional `f29/f28`. A declaration/assignment experiment
compiled but expanded to 139 words. No reliable FPR-cleanup shape was found.

### `fn_80080E40`

The target begins with the `mEntryFlag` test, then checks `mpActor`, then checks
`m_d4`; it retains `r28` for `idx << 2` and calls `dBc_c::getActorKind()` twice.
The direct gate reconstruction compiled to 126 words, while a simpler DC-first
variant compiled to 122. The original draft’s deletion of the stale gate remains
correct; restoring it is not supported by the target.

### `revisePos`

The target reads `0x9C`, `0xB4`, `0xB0`, `0x98`, `0xAC`, `0x94`. Two source
rewrites encoding that order compiled at 72 words but were codegen-neutral. This
is a measured negative result, not an unresolved compile omission.

### `addDokanMoveDiff`

The target uses frame `0x60`, FPR `f31/f30`, retains the square-root length in
`f30`, and uses `CosFIdx`/`SinFIdx` with the fractional angle scale. The
`target_math_dokan` variant implemented that idiom and reached 88 words. It is
one word longer than target and not safe to land.

### `fn_8007FFA0`

The canonical draft remains 107 versus target 115. Target evidence includes
intermediate stores at stack `0x10`/`0x14` and the target’s `r27`-based actor
lifetime. No additional source reconstruction was sufficiently evidenced to
claim convergence.

### `fn_80080900`

The target frame is `0x170` with `_savegpr_20`; canonical draft is `0xD0` with
`_savegpr_22`. The target retains four corner vectors, edge endpoints, and
separate geometry result vectors across calls and the loop. Attempts that merely
declared unused objects were optimized away and stayed at 208 words. A real
reconstruction must make each retained object participate in the exact stores and
calls.

### `fn_80080670`

The target body was explicitly attempted using the target’s circle-vector shape
and existing axis-aligned/rotated rectangle logic. It compiled at 127 words
against target 130, frame `0xB0`, but remains non-matching.

## Poolcheck

Ran:

```text
python tools/auto_decomp/poolcheck.py --module wiimj2d --obj scratch/round28/d_bg_ctr/d_bg_ctr.o --txt scratch/round28/d_bg_ctr/draft_disasm.txt scratch/round28/d_bg_ctr/target.txt
```

Result: **7 pooled constants compared by value, 0 mismatched, 0 unresolved**;
32 pairs value-checked and 39 target functions not checked because they were
unpaired, length-mismatched, or already differing.

All requested experiments were performed and compiled, but no byte-exact landing
candidate exists yet.
