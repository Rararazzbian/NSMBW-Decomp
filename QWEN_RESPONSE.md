# GXStateSave_c

## Result

**4 of 4 exact — DIFFS 0 out of 4**

All four functions now compile and match the retail listing exactly.

## Derived layout

| Offset | Contents |
|---|---|
| `+0x000` | `unsigned long` saved-state mask |
| `+0x004` | 27 `GXVtxAttrFmtList` entries |
| `+0x1B4` | 27 `GXVtxDescList` entries |
| `+0x28C` | projection storage, 7 `f32` values |
| `+0x2A8` | viewport storage, 6 `f32` values |
| `+0x2C0` | scissor storage, four `unsigned long` values |
| `+0x2D0` | cull mode |
| `+0x2D4..+0x2D6` | color-update, alpha-update, and dither bytes |

The class is non-virtual. The layout is offset-preserving and totals `0x2D7` bytes. The attr-format count is 27 (`GX_VA_MAX_ATTR + 1`), placing the descriptor array at `+0x1B4`.

## Function results

### `__ct__13GXStateSave_cFv`

- fndiff: `DIFFS 0`
- Target: 3 words / frame none / GPR none / FPR none
- Draft: 3 words / frame none / GPR none / FPR none

### `__dt__13GXStateSave_cFv`

- fndiff: `DIFFS 0`
- Target: 16 words / frame `0x10` / GPR `[31]` / FPR none
- Draft: 16 words / frame `0x10` / GPR `[31]` / FPR none

### `save__13GXStateSave_cFUl`

- fndiff: `DIFFS 0`
- Target: 62 words / frame `0x10` / GPR `[30, 31]` / FPR none
- Draft: 62 words / frame `0x10` / GPR `[30, 31]` / FPR none

### `restore__13GXStateSave_cFv`

- fndiff: `DIFFS 0`
- Target: 75 words / frame `0x10` / GPR `[31]` / FPR none
- Draft: 75 words / frame `0x10` / GPR `[31]` / FPR none

## Fixes applied

- Changed `mVtxAttrFmt` from 32 to 27 elements.
- Corrected the restore cull guard to `mMask & 16`.
- Added the seven exact `EGG::StateGX` wrapper declarations and calls, including the required trailing underscore in each function name.
- Used `unsigned long` for scissor storage and wrapper parameters, and `bool` for the three boolean wrappers, matching the target mangling.
- The earlier prediction that the initial cache-byte source shape was sufficient was false; the exact EGG wrapper declarations/calls were required to close the remaining diffs.

## Landing readiness

**Landing-ready: 4/4 exact.** The source and scratch/shadow headers are complete, and all four functions have real objects and `fndiff.py --all -v` lines showing `DIFFS 0`. No prohibited commands were run, and no shared headers, source files, or symbol maps were modified.

Verified command:

```text
python tools/auto_decomp/fndiff.py scratch/qwen_gx/target.txt scratch/qwen_gx/close.txt --all -v
```
