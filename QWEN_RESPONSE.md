# GXStateSave_c

## Result

**DIFFS 0 out of 4**

The build is running. Two of four functions are byte-identical; save and restore have the correct length/frame/register shape but still contain instruction differences.

## Derived layout

| Offset | Contents |
|---|---|
| `+0x000` | `u32` saved-state mask |
| `+0x004` | vertex attribute format storage |
| `+0x1B4` | vertex descriptor storage |
| `+0x28C` | projection storage, 7 `f32` values |
| `+0x2A8` | viewport storage, 6 `f32` values |
| `+0x2C0` | scissor storage, four `u32` values |
| `+0x2D0` | cull mode |
| `+0x2D4..+0x2D6` | color-update, alpha-update, and dither bytes |

The class has no virtual functions. The constructor's exact three-word body and the deleting destructor wrapper confirm that the apparent 64-byte destructor is not evidence of a virtual slot. The member layout is offset-preserving and totals `0x2D7` bytes.

## Function results

### `__ct__13GXStateSave_cFv`

- fndiff: `DIFFS 0`
- Target: 3 words / frame none / GPR none / FPR none
- Draft: 3 words / frame none / GPR none / FPR none
- Assessment: exact match. The constructor only clears the mask at `+0x0`.

### `__dt__13GXStateSave_cFv`

- fndiff: `DIFFS 0`
- Target: 16 words / frame `0x10` / GPR `[31]` / FPR none
- Draft: 16 words / frame `0x10` / GPR `[31]` / FPR none
- Assessment: exact match. It is the standard deleting destructor wrapper with no member destruction.

### `save__13GXStateSave_cFUl`

- fndiff: `DIFFS 11`
- Target: 62 words / frame `0x10` / GPR `[30, 31]` / FPR none
- Draft: 62 words / frame `0x10` / GPR `[30, 31]` / FPR none
- Assessment: all call sites and storage offsets are now represented, including the three EGG cache-byte reads. The remaining differences are likely source-expression/API-shape differences in those cache reads or the typed member addressing, not layout or function size.

### `restore__13GXStateSave_cFv`

- fndiff: `DIFFS 10`
- Target: 75 words / frame `0x10` / GPR `[31]` / FPR none
- Draft: 75 words / frame `0x10` / GPR `[31]` / FPR none
- Assessment: operation order, mask tests, offsets, and call sequence now match the target's shape. The remaining differences are likely typed member/call-expression differences; frame and register allocation are exact.

## Landing readiness

**Not landing-ready: 2/4 exact.** The constructor and destructor are complete. `save` and `restore` still require instruction-level cleanup to reach `DIFFS 0`; no shared files were modified and no prohibited commands were run.

Full fndiff output is preserved in `scratch/qwen_gx/fndiff.txt`.
