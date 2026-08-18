# Near-Miss Classification: Batches 1-4

| Function | Batch | Class | Our Instrs | Target Instrs | Note |
|---|---|---:|---:|---:|---|
| `initGame()` | B1 | base-register / anchor artifacts | 41 | 41 | The named `m_playerID` base relocation is an isolation artifact; remaining register-pair role swap is allocation/scheduling only. |
| `initStage()` | B1 | base-register / anchor artifacts | 96 | 96 | Only the `m_playerID` `@ha/@l` naming differs, caused by compiling without the assembled TU's shared `.bss` anchor context. |
| `setDefaultParam()` | B1 | instruction count differs | 35 | 41 | Same stores and values, but target preserves four live `mPlayerType` reads and has a callee-saved frame that the draft avoids. |
| `getPlayerSetPos(u8, u8)` | B1 | instruction count differs | 55 | 55 | One structural instruction is missing: target rounds the negated Y with an explicit `frsp` before storing it. |
| `fn_8005f4d0(mVec3_c*, int, int)` | B2 | OTHER | 39 | 39 | The only difference is isolated-compile relocation naming (`scBaseID` versus anonymous/pool `SYM0`), not an instruction or register difference. |
| `fn_8005f570(PLAYER_POWERUP_e, int)` | B2 | register allocation only | 20 | 20 | All operations and order match; six diffs only change which physical registers hold already-correct values, with shared-base effects resolved when definitions are present. |
| `createCourseInit()` | B2 | instruction count differs | 345 | 352 | Target keeps separate action tests, calls `getFileP` out of line, and uses a different boolean-materialization idiom; downstream differences are shifts from these shape changes. |
| `update()` | B3 | instruction count differs | 173 | 174 | Logic and instruction kinds match, but target materializes/reuses a base-plus-offset pointer differently, producing one extra instruction and register differences. |
| `decideCtrlPlrNo()` | B3 | instruction count differs | 26 | 25 | Logic and instruction order match, but target performs a one-instruction CSE for the unrolled `i == 1` case and uses different register allocation. |
| `getYoshi(int)` | B4 | register allocation only | 39 | 39 | The vtable call is identical in shape and count; only the first vtable load uses `r4` in ours instead of target `r12`. |

## Summary

| Class | Count |
|---|---:|
| base-register / anchor artifacts | 2 |
| register allocation only | 2 |
| instruction count differs | 5 |
| scheduling only | 0 |
| constant-folding differences | 0 |
| OTHER | 1 |

The `setDefaultParam`, `getPlayerSetPos`, `createCourseInit`, `update`, and
`decideCtrlPlrNo` rows are assigned to `instruction count differs` even where
their notes also mention scheduling or register allocation, since their
documented instruction counts differ.
