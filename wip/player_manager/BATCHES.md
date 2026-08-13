# Authoring batches — `d_a_player_manager.cpp`

65 functions of our own (plus 2 foreign inlines nobody authors). Assignments
are **from this table, not from prose**. Two batches were once briefed with the
same function because a brief followed a prose summary instead of a table.

| Batch | Range | Count | Functions |
|---|---|---|---|
| **B1** | `0x8005E9A0`–`0x8005EEDF` | 8 | createYoshi, initGame, initStage, exitStage, courseIn, setDefaultParam, getPlayerSetPos, getPlayerCreateAction |
| **B2** | `0x8005EEE0`–`0x8005F5BF` | 4 | create, createCourseInit (`0x580`), fn_8005F4D0, fn_8005f570 |
| **B3** | `0x8005F5C0`–`0x8005FA5F` | 7 | update (`0x2B8`), isPlayerPauseEnable, setPlayer, getPlayer, decideCtrlPlrNo, setYoshi, releaseYoshi |
| **B4** | `0x8005FA60`–`0x8005FDAF` | 10 | getYoshi, getYoshiNum, getYoshiDirectP, getCtrlPlayer, getCourseInPlayerModelType, setCarryOverYoshiInfo, getYoshiColor, getYoshiFruit, getActScrollInfo, getScrollNum |
| **B5** | `0x8005FDB0`–`0x8006024F` | 12 | addNum(int), decNum(int), addNum(), decNum(), getNumInGame, getEntryNum, getItemKinopioNum, getItemKinopio, getPlayerIndex, changeItemKinopioPlrNo, getCourseInListPlrNo, getCoinAll |
| **B6** | `0x80060250`–`0x800608DF` | 11 | incCoin (`0x208`), addRest, incRestAll, decRest, addScore, setCourseInStarBGM, startStarBGM, stopStarBGM, startMissBGM, startYoshiBGM, stopYoshiBGM |
| **B7** | `0x800608E0`–`0x80060F1F` | 7 | checkLastAlivePlayer, executeLastPlayer, executeLastAll, deleteCullingYoshi, setHipAttackQuake, fn_80060DB0, checkBonusNoCap |
| **B8** | `0x80060F20`–`0x80061304` | 6 | initYoshiPriority, setYoshiPriority, isEffectStop, isAcceptQuake, isCreateBalloon, checkCorrectCreateInfo |

## Data ownership — named owners, so two batches cannot define the same object

Function ownership is always spelled out; data ownership usually is not, and on
a previous unit two batches defined the same object under different names. Only
one copy can land.

| Object | Section | Owner | Note |
|---|---|---|---|
| `scModelTypeDt` `{0,1,2,3}` | `.rodata:0x802EF608` | **B4** | function-local `static const` inside `getCourseInPlayerModelType`. Our ONLY `.rodata` object. |
| `scRestMax` = 99, `scCoinMax` = 99, `scScoreMax` = 999999999 | `.sdata:0x80427C00-0C` | **B6** | anonymous-namespace file-scope `const int`s — the mangled names say `@unnamed@d_a_player_manager_cpp@`, so they are **not** class members. |
| `"Wm_mr_vshipattack"`, `"Wm_mr_vshipattack_ind"` | `.data:0x80309A28`, `0x80309A3C` | **B7** | `mEf::createEffect` argument literals in `fn_80060DB0`. |
| `3800.0f` | `.sdata2:0x8042BD7C` | **B7** | float literal in `fn_80060DB0`. |
| **`lbl_80429FD0`** — unnamed 1-byte flag | `.sbss:0x80429FD0` | **B7** | Read (`lbz`) at `0x80060D04` and written (`stb`) at `0x80060D30`, both in `setHipAttackQuake`. No class mangling → a **file-scope static**, not a member. **Name it and define it.** It is the last object in our `.sbss`; if nobody defines it the section comes up short and four of five binaries fail. |
| everything else in `.sdata2` `0x8042BD48`–`0x8042BD78` | `.sdata2` | *nobody* | ordinary float literals; they are emitted by the code that uses them (B1, B2, B7). Do not hand-write them. |

## Who authors nothing

`getCourseIn__10dScStage_cFv` (`0x8005EC90`) and `getFileP__5dCd_cFi`
(`0x8005EE70`) are foreign weak inline bodies emitted into our `.text` because
we call them. **Do not write either one.** Both headers are already fixed and
landed. See `SHARED-BRIEF.md` hazard 2 — in particular the `getFileP`
inline-budget note, which belongs to **B2**.

## Deliverable

Source code **in your reply**, plus a file `wip/player_manager/BATCH<N>.md` with
your per-function match status and your data-object report. Do not edit the
shared `.cpp`; the lead assembles by address.
