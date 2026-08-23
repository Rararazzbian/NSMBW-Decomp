# dIceEfMaker_c report

## Result

**DIFFS 0: 1 out of 6 functions.**

`fin__13dIceEfMaker_cFv` matches exactly.

## Function results

| Function | Target | Draft | Result |
|---|---|---|---|
| `init__13dIceEfMaker_cFiP13dIceEfScale_c` | 11 words / frame none / GPR none / FPR none | 11 words / frame none / GPR none / FPR none | DIFFS 2 |
| `execute__13dIceEfMaker_cFv` | 39 words / frame 0x30 / GPR none / FPR none; `_savegpr_27` | 44 words / frame 0x40 / GPR none / FPR none; `_savegpr_27` | DIFFS 33; the temporary vector representation and return-value handling produce a larger frame and different instruction sequence. |
| `fin__13dIceEfMaker_cFv` | 1 word / frame none / GPR none / FPR none | 1 word / frame none / GPR none / FPR none | DIFFS 0 |
| `setEfScale__13dIceEfMaker_cFRC13dIceEfScale_c` | 33 words / frame none / GPR none / FPR none | 33 words / frame none / GPR none / FPR none | DIFFS 32; the loads are allocated into the reverse FPR order from the target, so the stores do not match despite the identical shape. |
| `createEffect__13dIceEfMaker_cFQ213dIceEfMaker_c8EfKind_e` | 31 words / frame 0x20 / GPR [30, 31] / FPR none | emitted as `createEffect__13dIceEfMaker_cFi` (35 words / frame 0x30 / GPR [30, 31] / FPR none) | Not comparable under the authoritative mangled-name extraction; the draft declaration uses `int`, while the target parameter is the scoped `EfKind_e` type. |
| `hahenEffect__13dIceEfMaker_cFv` | 26 words / frame 0x20 / GPR [31] / FPR none | 33 words / frame 0x30 / GPR [31] / FPR none | DIFFS 20; the manager call is unresolved in the available headers and the raw function-pointer reconstruction adds instructions and frame space. |

## Landing readiness

**Not ready: 1/6, not 6/6.** Five functions remain: `init`, `execute`, `setEfScale`, `createEffect`, and `hahenEffect`.

## Reached / did not reach

All six requested functions were attempted, compiled into a real object, and measured with `fndiff --all`. I reached an exact match for `fin` only. I did not reach a byte-identical unit or a landing-ready 6/6 result. No project-wide build, `ninja`, `configure.py`, `progress.py`, or `land.py` was run.
