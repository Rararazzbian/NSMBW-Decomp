# GXStateSave_c

## Result

**DIFFS 0 out of 4**

The assigned class was reconstructed in `scratch/qwen_gx/draft.cpp`, but the required scratch build could not compile because the seeded header `game/bases/d_gx_state_save.hpp` and its GX declarations are absent from this checkout. Therefore no valid object or fndiff lines could be produced.

## Derived layout

| Offset | Contents | Evidence |
|---|---|---|
| `+0x000` | `u32` saved-state mask | constructor, save OR, restore tests/clear |
| `+0x004..+0x1B3` | vertex attribute format storage | `GXGetVtxAttrFmtv` / `GXSetVtxAttrFmtv` |
| `+0x1B4..+0x28B` | vertex descriptor storage | `GXGetVtxDescv` / `GXSetVtxDescv` |
| `+0x28C..+0x2A7` | projection values | projection get/set |
| `+0x2A8..+0x2BF` | six viewport floats | viewport get/set |
| `+0x2C0..+0x2CF` | four scissor `u32`s | scissor get/set |
| `+0x2D0..+0x2D3` | cull mode | cull get/set |
| `+0x2D4` | color-update flag | cache byte / restore |
| `+0x2D5` | alpha-update flag | cache byte / restore |
| `+0x2D6` | dither flag | cache byte / restore |

The compiled class storage is intended to be `0x2D7` bytes. No offset perturbation is intended by this layout.

## Function status

### `__ct__13GXStateSave_cFv`

- Target: 12 bytes, frame `0x0`, GPR saves none, FPR saves none.
- Draft: `mMask(0)` was written as the sole constructor operation.
- Compiled: **NO** — missing seeded header prevented compilation.
- fndiff: **not available**.
- Assessment: likely exact once the intended header/declarations are available.

### `__dt__13GXStateSave_cFv`

- Target: 64 bytes, frame `0x10`, GPR saves `r30/r31`, FPR saves none.
- Draft: out-of-line empty destructor; expected compiler-generated deleting-wrapper shape.
- Compiled: **NO** — missing seeded header prevented compilation.
- fndiff: **not available**.
- Assessment: source body is intentionally empty, matching the target's no-member-destruction instructions, but this cannot be verified without the header/build environment.

### `save__13GXStateSave_cFUl`

- Target: 248 bytes, frame `0x10`, GPR saves `r30/r31`, FPR saves none.
- Draft: implements the nine mask-selected saves in target order and ORs the mask into `+0x0`.
- Compiled: **NO** — missing `GXStateSave_c` header and GX declarations.
- fndiff: **not available**.
- Assessment: implementation follows the listing, but API signatures and compiler register allocation remain unverified.

### `restore__13GXStateSave_cFv`

- Target: 300 bytes, frame `0x10`, GPR saves `r31`, FPR saves none.
- Draft: implements the nine restore operations in target order and clears the mask.
- Compiled: **NO** — missing `GXStateSave_c` header and GX declarations.
- fndiff: **not available**.
- Assessment: implementation follows the listing, including target bit order, but API signatures and compiler register allocation remain unverified.

## Landing readiness

**Not landing-ready: 0/4 verified.** The source transcription is present, but all four functions still need a real compile and fndiff score after the intended `game/bases/d_gx_state_save.hpp` header/API declarations are restored or supplied in the scratch shadow. No shared files were modified, and no prohibited commands were run.
