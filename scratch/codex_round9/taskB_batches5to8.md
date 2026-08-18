# Batches 5-8 near-miss taxonomy

Instruction counts are `ours vs target`. Matching functions are omitted because
the request is for near-misses only. For mixed cases, the class is the primary
failure mechanism, with secondary effects described in the note.

| Function | Batch | Class | Our Instrs | Target Instrs | Note |
|---|---:|---|---:|---:|---|
| `getNumInGame()` | B5 | base-register / anchor artifacts | 46 | 46 | Logic and instruction order match, but the isolated compile uses three `.bss` bases while target anchors `mPlayerEntry`, `mPlayerType`, and `mRest` from `m_playerID`. |
| `getCoinAll()` | B5 | register allocation only | 20 | 20 | Same loads, indexing, additions, and order; only the intermediate/address registers differ. |
| `incCoin(int)` | B6 | instruction count differs | 134 | 130 | Four extra instructions come from separate array address materialization; the unresolved `getEntryNum() > 1` branchless idiom is also a shape difference. |
| `addRest(int, int, bool)` | B6 | register allocation only | 74 | 74 | Entire shape and count match; only the final clamp section has a uniform register-number shift. |
| `decRest(int)` | B6 | OTHER | 40 | 36 | The current header declares `bool`, causing return canonicalization; changing the declaration to `int` produced the target exactly. |
| `startMissBGM(int)` | B6 | register allocation only | 24 | 24 | Same vtable chase and call, but the first load uses `r4` rather than keeping the chain entirely in `r12`. |
| `checkLastAlivePlayer()` | B7 | OTHER | 34 | 34 | Same overall logic and count, but one five-instruction boolean-materialization idiom differs (`cntlzw/slw` versus `xori/srawi/and/subf/srwi`). |
| `deleteCullingYoshi()` | B7 | register allocation only | 86 | 86 | All content, control flow, calls, and constants match; only GPR/FPR assignments differ. |
| `setHipAttackQuake(int, u8)` | B7 | instruction count differs | 103 | 104 | Same shape and symbols, but the target has one extra instruction and different register choices in the unrolled timer scan/table write. |
| `initYoshiPriority(daPlBase_c*)` | B8 | OTHER | 46 | 46 | Same order and logic; the only opcode difference is symmetric `cmpw` operand order. |
| `setYoshiPriority(daPlBase_c*)` | B8 | register allocation only | 38 | 38 | Same branches and order, with two loop-invariant locals swapped between registers plus the same `cmpw` operand-order artifact. |
| `isCreateBalloon(int)` | B8 | scheduling only | 18 | 18 | Same instruction set and values, but true/false blocks are laid out in the opposite order with reversed branch polarity. |
| `checkCorrectCreateInfo()` | B8 | constant-folding differences | 105 | 105 | Target performs hoisted `.sdata` loads for `scRestMax`, `scCoinMax`, and `scScoreMax`; the isolated draft folds them to immediates, shifting dependent registers. |

## Summary

| Class | Count |
|---|---:|
| base-register / anchor artifacts | 1 |
| register allocation only | 5 |
| instruction count differs | 2 |
| scheduling only | 1 |
| constant-folding differences | 1 |
| OTHER | 3 |
| **Total** | **13** |

Note: the table contains 13 documented near-miss functions. `incCoin` is one
function despite having two independently documented unresolved gaps.
