# Sibling correspondence map — `d_a_player_manager.cpp` (`daPyMng_c`)

Scope: the 67 real `.fn` entries in `wip/player_manager/target_text.txt`
(`0x8005E9A0`-`0x80061304`, size `0x2964`; the file's remaining bytes up to
`0x80061310` are the closing `gap_03_80061304_text` padding). **Count note:**
the brief says "68 functions" — mechanical extraction of every non-`gap_`
`.fn`/`.endfn` pair in `target_text.txt` (`grep -oP '^\.fn \K[^,]+' | grep -v
gap_` and independently a Python `.fn`/`.endfn`-tracking pass) both find
**67**, not 68. No duplicate-named `.fn` was found (each of the four
`addNum`/`decNum` overloads has a distinct mangled name and a verified
distinct body — see the overload section below), so this isn't the
duplicate-body bug the brief warns about. Reporting the discrepancy per the
"report contradictions" rule rather than silently reconciling it.

## Method

For every function below I read its full target disassembly (all 67 bodies,
in `target_text.txt`) and searched already-banked `source/dol/` (chiefly
`d_a_player_demo_manager.cpp`, `d_a_player.cpp`, `d_a_player_base.cpp`,
`f_manager.cpp`) for a candidate twin. A twin claim is only shipped VERIFIED
when `tools/auto_decomp/harness.py`'s `extract()`/`canonicalise()` on **both
sides' target disassembly** (not source, not a draft) produces identical
token streams, per the brief. **Result: zero VERIFIED external twins were
found**, for a structural reason worth stating up front rather than
per-row: `canonicalise()` only numbers *pool* references (the
`@NNNNN[_ADDR]` / `lbl_ADDR` / `...section.N` forms) — it does **not**
touch ordinary named symbols (`mFoo__9daPyMng_c@ha`, `mFoo__9daPyMng_c@sda21`,
etc.). Every one of `daPyMng_c`'s ~30 static members has a name that exists
nowhere else in the codebase, so any external function touching the
equivalent data can *at best* match in shape, never in canonicalised text.
The only way this unit gets a byte-identical twin is the mechanism the brief
already names in hazard 2 (a foreign function's own weak body reproduced
verbatim because *our* TU calls it) — covered under "Authors nobody" below.
Two internal near-duplicate pairs exist (noted in the table) but the brief's
"elsewhere" requirement excludes them from counting as map twins.

## Per-function table

| # | Address | Size | Mangled name | Twin (file:function) | Verified? | Notes |
|---|---|---|---|---|---|---|
| 1 | 0x8005E9A0 | 0xB8 | `createYoshi__9daPyMng_cFR7mVec3_ciP7dAcPy_c` | — | NO TWIN | Header-declared. Branches on rider==null between two `construct__8dActor_c` call shapes (fBaseID 0xe = daYoshi_c), then conditionally `fn_8014eb70__9daYoshi_cFP7dAcPy_ci` and a vtable dispatch at actor+0x270. |
| 2 | 0x8005EA60 | 0xA4 | `initGame__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** Zeroes several 4-word regions computed as offsets from `m_playerID`'s own relocation (`+0x40/+0x50/+0x60/+0x70`), sets `mActPlayerInfo` bit 0, calls `setDefaultParam`, zeroes `mBonusNoCap`/`mKinopioCarryCount`. |
| 3 | 0x8005EB10 | 0x180 | `initStage__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** Calls `checkCorrectCreateInfo`; zeroes `mNum`/`mCtrlPlrNo`/`mActPlayerInfo`; loop0..3 `setPlayer(i,null)` + zero `m_star_time`/`m_star_count`; loop0..3 over `m_playerID` testing nonzero→`addNum(i)`; **calls `getCourseIn__10dScStage_cFv` directly — see Authors-nobody #4 below**; conditional `getEntryNum`+4-iter `mtctr` loop `m_playerID[k]==0`→`fn_8005f570`; zeroes BGM/quake/pause statics; `checkBonusNoCap`; `initStage__13daPyDemoMng_cFv`; `initStage__11dMultiMng_cFv`. |
| 4 | 0x8005EC90 | 0x8 | `getCourseIn__10dScStage_cFv` | — | **AUTHORS NOBODY** | Foreign weak inline body, see dedicated section below. |
| 5 | 0x8005ECA0 | 0x4 | `exitStage__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Empty body (`blr` only) — a deliberate no-op, author out-of-line. |
| 6 | 0x8005ECB0 | 0x30 | `courseIn__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. `createCourseInit`; zero `mPauseDisable`; `daPyDemoMng_c::initCourseIn`. |
| 7 | 0x8005ECE0 | 0xA4 | `setDefaultParam__9daPyMng_cFv` | — | NO TWIN | Header-undeclared (internal helper, called only by `initGame`). Initialises default per-player type/mode arrays and `mScore=0`. |
| 8 | 0x8005ED90 | 0xDC | `getPlayerSetPos__9daPyMng_cFUcUc` | — | NO TWIN | Header-declared (`nw4r::math::VEC3 getPlayerSetPos(u8,u8)`; disasm returns via hidden struct-return pointer, ordinary ABI, not a contradiction). Calls `getNextGotoP__9dCdFile_cFUc`; **the compiler INLINES `dCd_c::getFileP`'s body here** (no `bl`) against `l_start_pos_ofs` (`.rodata`, 12-byte-stride position table) — see Authors-nobody #9. |
| 9 | 0x8005EE70 | 0x20 | `getFileP__5dCd_cFi` | — | **AUTHORS NOBODY** | Foreign weak inline body, see dedicated section below. |
| 10 | 0x8005EE90 | 0x50 | `getPlayerCreateAction__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Reads `dScStage_c` fields `0x120e`/`0x1211` (undeclared fields); **also gets `getFileP`'s body inlined** (2nd inline site, see #9); `getNextGotoP`. |
| 11 | 0x8005EEE0 | 0x64 | `create__9daPyMng_cFiP7mVec3_ciUc` | — | NO TWIN | Header-undeclared despite being an internal workhorse (4 in-unit call sites). Tests `mPlayerEntry[idx]`; on 0, `construct__8dActor_c` with an fBaseID assembled from `(type<<16)|(flag<<24)|idx`. |
| 12 | 0x8005EF50 | 0x580 | `createCourseInit__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Largest ordinary function. Branches on `getPlayerCreateAction()` (0/1/0x17 = respawn-in-place vs course-in). Calls `fn_8005D280` (see note below — this is `daPyDemoMng_c`'s own already-landed unnamed helper, called cross-TU). **Calls `getFileP__5dCd_cFi` directly — see Authors-nobody #9.** Contains a large runtime-shift float insertion-sort block (`0x8005F160`-`0x8005F388`) — checked against `daPyDemoMng_c::makeCourseInList`'s shift loop and REJECTED (see Rejected candidates). |
| 13 | 0x8005F4D0 | 0x9C | `fn_8005F4D0` | — | NO TWIN | Unnamed in symbol map. **Confirmed static member `daPyMng_c::fn_8005f4d0(mVec3_c*, int, int)`** — r3/r4/r5 are all read as real incoming args before any use, matching the header's existing placeholder declaration exactly. See flag #2 below. |
| 14 | 0x8005F570 | 0x50 | `fn_8005f570__9daPyMng_cF16PLAYER_POWERUP_ei` | — | NO TWIN | Header-declared (`fn_8005f570(PLAYER_POWERUP_e,int)`). Sets a per-player default-mode word and `mActPlayerInfo`/`mKinopioMode`. |
| 15 | 0x8005F5C0 | 0x2B8 | `update__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Large per-frame dispatcher: `checkLastAlivePlayer`, HUD setup, 4 createItem-timer decrements, a quake-trigger loop, pause toggling, stop-timer sync, `daPyDemoMng_c::update`, `dPyEffectMng_c::update`. Its two inner loops use the exact `for(i<4) if(checkPlayer(i)){p=getCtrlPlayer(i); if(p){...}}` skeleton that `d_a_player_demo_manager.cpp`'s `calcNotGoalPlayer`/`executeGoalDemo_Pole` (lines 229-248, 286-314) already prove compiles correctly — a genuine STYLE precedent, not a whole-function twin (see Rejected candidates #6). |
| 16 | 0x8005F880 | 0x3C | `isPlayerPauseEnable__9daPyMng_cFSc` | — | NO TWIN | Header-undeclared. Two-stage bit test: `mActPlayerInfo` then `mPauseEnableInfo`. |
| 17 | 0x8005F8C0 | 0x38 | `setPlayer__9daPyMng_cFiP7dAcPy_c` | — | NO TWIN | Header-undeclared (despite being called everywhere). **Odd detail worth flagging to the author:** when `player!=null` the stored value is `*player` (a dereference of the incoming pointer, `lwz r3,0x0(r4)`), not the pointer itself — i.e. it stores some ID field *out of* the actor, matching `getPlayer`'s later `fManager_c::searchBaseByID` reload. |
| 18 | 0x8005F900 | 0x14 | `getPlayer__9daPyMng_cFi` | — | NO TWIN | Header-declared. Tail-calls `fManager_c::searchBaseByID(m_playerID[idx])`. Checked against `daPlBase_c::getHipAttackDamagePlayer` (`d_a_player_base.cpp:4943`) — REJECTED, different addressing (member-offset read vs. SDA array index). Its internal shape-twin `getYoshiDirectP` (#24) is in the *same*, still-undecompiled unit, so it doesn't count as a map twin either. |
| 19 | 0x8005F920 | 0x64 | `decideCtrlPlrNo__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Unrolled 4-way `mActPlayerInfo` bit test → `mCtrlPlrNo` = first set bit. |
| 20 | 0x8005F990 | 0x64 | `setYoshi__9daPyMng_cFP10daPlBase_c` | — | NO TWIN | Header-undeclared. Finds a free (`==0`) `m_yoshiID` slot, stores `*player` (dereferences arg, an ID field) into it. |
| 21 | 0x8005FA00 | 0x58 | `releaseYoshi__9daPyMng_cFP10daPlBase_c` | — | NO TWIN | Header-undeclared. Reverse of #20 — finds the matching `m_yoshiID` slot, zeroes it. In-unit mirror of #20, not an external twin. |
| 22 | 0x8005FA60 | 0x9C | `getYoshi__9daPyMng_cFi` | — | NO TWIN | Header-declared. Loop4 `searchBaseByID(m_yoshiID[i])`→vtable call @0x6c→compare returned byte to arg. |
| 23 | 0x8005FB00 | 0x6C | `getYoshiNum__9daPyMng_cFv` | — | NO TWIN | Header-declared. Loop4 counting non-null `searchBaseByID(m_yoshiID[i])`. |
| 24 | 0x8005FB70 | 0x14 | `getYoshiDirectP__9daPyMng_cFi` | — | NO TWIN | Header-declared. Tail-calls `searchBaseByID(m_yoshiID[idx])` — identical SHAPE to `getPlayer` (#18) but a different symbol, hence a different in-unit pair, not an external twin. |
| 25 | 0x8005FB90 | 0x50 | `getCtrlPlayer__9daPyMng_cFi` | — | NO TWIN | Header-undeclared, but **its exact behaviour is already load-bearing in the landed `d_a_player_demo_manager.cpp`**, which calls `daPyMng_c::getCtrlPlayer()` dozens of times and treats a null return as "no controllable player". Disasm: `getPlayer(i)`; if found, `getRideYoshi()`; **returns null when riding a yoshi** (not "prefer the yoshi"), the raw pointer otherwise. Worth flagging to the author as strong corroborating evidence, not a byte twin. |
| 26 | 0x8005FBE0 | 0x40 | `getCourseInPlayerModelType__9daPyMng_cFUc` | — | NO TWIN | Header-declared. Indexes `mPlayerType[idx]` into `mCreateItem[type]`; bit 2 set → returns `4`; else looks up a 4-entry function-local static `@LOCAL@getCourseInPlayerModelType__9daPyMng_cFUc@scModelTypeDt` (i.e. `static const dPyMdlMng_c::ModelType_e scModelTypeDt[4] = {...};`). |
| 27 | 0x8005FC20 | 0x1C | `setCarryOverYoshiInfo__9daPyMng_cFUcUci` | — | NO TWIN | Header-declared. Stores color→`m_yoshiColor[idx]` (`.sbss`, byte), fruit→`m_yoshiFruit[idx]` (`.bss`, word). |
| 28 | 0x8005FC40 | 0xC | `getYoshiColor__9daPyMng_cFUc` | — | NO TWIN | Header-declared. 3-instruction `li`+`lbzx`+`blr` accessor. This exact 3-instruction shape is the generic MWCC idiom for *any* byte-array accessor and recurs project-wide against a different symbol every time — checked and REJECTED as a specific-function claim (see Rejected candidates #5). |
| 29 | 0x8005FC50 | 0x14 | `getYoshiFruit__9daPyMng_cFUc` | — | NO TWIN | Header-declared. `lwzx` from `m_yoshiFruit[idx]`. |
| 30 | 0x8005FC70 | 0xA8 | `getActScrollInfo__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Loop4 over `mPlayerEntry`; if set, `getPlayer(i)`, tests byte field `0x153c` (undeclared `daPlBase_c` field) `==1` to build a 4-bit mask. |
| 31 | 0x8005FD20 | 0x8C | `getScrollNum__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Same loop/field shape as #30 (in-unit near-twin, not external) but returns a count instead of a mask. |
| 32 | 0x8005FDB0 | 0x74 | `addNum__9daPyMng_cFi` | — | NO TWIN | Header-declared (`bool addNum(int)`). Sets `mActPlayerInfo` bit; if no live actor yet (or its vtable-@0xe0 test fails), falls back to `addNum__9daPyMng_cFv` (#34). See overload note below. |
| 33 | 0x8005FE30 | 0x7C | `decNum__9daPyMng_cFi` | — | NO TWIN | Header-declared (`bool decNum(int)`). Mirror of #32 (clears bit), calls `decNum__9daPyMng_cFv` (#35) and also `decideCtrlPlrNo` (#19). See overload note below. |
| 34 | 0x8005FEB0 | 0x18 | `addNum__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared** — only the `Fi` overload is declared. Clamped `++mNum` (cap 4). See overload note below. |
| 35 | 0x8005FED0 | 0x18 | `decNum__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared** — only the `Fi` overload is declared. Clamped `--mNum` (floor 0). See overload note below. |
| 36 | 0x8005FEF0 | 0xB8 | `getNumInGame__9daPyMng_cFv` | — | NO TWIN | Header-undeclared. Unrolled 4-way test `m_playerID[i]!=0 && (per-player type array entry)>0` → count. |
| 37 | 0x8005FFB0 | 0x58 | `getEntryNum__9daPyMng_cFv` | — | NO TWIN | Header-declared. Unrolled 4-way count of nonzero `mPlayerEntry[i]`. |
| 38 | 0x80060010 | 0x74 | `getItemKinopioNum__9daPyMng_cFv` | — | NO TWIN | Header-declared. Loop4 `getPlayer(i)`+vtable-@0xe0 (the `isItemKinopio()` slot the header's own `isItemKinopio(int)` inline calls), counts trues. |
| 39 | 0x80060090 | 0x74 | `getItemKinopio__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared** (only the per-player `isItemKinopio(int)` inline exists, not this global search). Same vtable-@0xe0 test as #38, in-unit near-twin, but returns the *first matching pointer*, not a count. |
| 40 | 0x80060110 | 0x5C | `getPlayerIndex__9daPyMng_cF13PLAYER_TYPE_e` | — | NO TWIN | Header-declared. Unrolled 4-way `mPlayerType[i]==arg` search, else `-1`. |
| 41 | 0x80060170 | 0x64 | `changeItemKinopioPlrNo__9daPyMng_cFRi` | — | NO TWIN | Header-declared (`changeItemKinopioPlrNo(int&)`). `getPlayer(*idx)`, vtable-@0xe0 (`isItemKinopio` again), if true zero `*idx`. |
| 42 | 0x800601E0 | 0x14 | `getCourseInListPlrNo__9daPyMng_cFi` | — | NO TWIN | **Header-undeclared** (header has the raw array `mCourseInList[4]` but not this accessor). `lwzx` from `mCourseInList[idx]`. |
| 43 | 0x80060200 | 0x50 | `getCoinAll__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** Fully unrolled sum of `mCoin[mPlayerType[0..3]]` (4 indirections, no loop). |
| 44 | 0x80060250 | 0x208 | `incCoin__9daPyMng_cFi` | — | NO TWIN | Header-declared. `changeItemKinopioPlrNo`, `dMultiMng_c::incCoin`, compares against file-local `scCoinMax` (anon-namespace `.sdata` constant, value `0x63`=99 — confirmed by reading `target_sdata.txt`), plays two positional bonus-coin stereo cues when crossing the cap parity, else `fn_800e25a0__11dScoreMng_c` or `addRest`. |
| 45 | 0x80060460 | 0x128 | `addRest__9daPyMng_cFiib` | — | NO TWIN | **Header signature MISMATCH** — see flag #4. Conditionally plays two positional "1-up" stereo cues (gated on the 2nd bool-ish param `==1`), clamps `mRest[mPlayerType[idx]]` to `scRestMax` (`0x63`=99). |
| 46 | 0x80060590 | 0x70 | `incRestAll__9daPyMng_cFb` | — | NO TWIN | **Header-undeclared.** Loop4 over `mPlayerEntry`, calls `addRest(i,1,arg)` per present player. |
| 47 | 0x80060600 | 0x90 | `decRest__9daPyMng_cFi` | — | NO TWIN | Header-declared, signature matches (`decRest(int)`). `changeItemKinopioPlrNo`, plays a "miss" system SE, decrements `mRest[mPlayerType[idx]]` (floor 0), zeroes `mBonusNoCap` when it lands exactly on 0. |
| 48 | 0x80060690 | 0x68 | `addScore__9daPyMng_cFii` | — | NO TWIN | Header-declared. Clamps `mScore` to `scScoreMax` (`0x3B9AC9FF`=999,999,999); if 2nd param `<=3` also forwards to `dMultiMng_c::addScore`. |
| 49 | 0x80060700 | 0x1C | `setCourseInStarBGM__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** If `mBgmState` bit0 set, tail-calls `fn_8019bd90__11SndSceneMgrFi(4)`. |
| 50 | 0x80060720 | 0x24 | `startStarBGM__9daPyMng_cFv` | — | NO TWIN | Header-declared. Sets `mBgmState` bit0 (guarded), tail-calls `fn_8019bd90(4)`. In-unit near-twin of #49 (guard polarity differs). |
| 51 | 0x80060750 | 0x74 | `stopStarBGM__9daPyMng_cFv` | — | NO TWIN | Header-declared. If `mBgmState` bit0 set: loop4 `getPlayer(i)`'s field `0x1070>=0x3c` (undeclared timer field) gates clearing bit0 + `fn_8019be60(4)`. |
| 52 | 0x800607D0 | 0x60 | `startMissBGM__9daPyMng_cFi` | — | NO TWIN | Header-declared. `getPlayer(idx)`, vtable-@0xe0 negated, field `0x1554` nonzero test, `SndSceneMgr::startMiss()`. |
| 53 | 0x80060830 | 0x24 | `startYoshiBGM__9daPyMng_cFv` | — | NO TWIN | Header-declared. Sets `mBgmState` bit1, `fn_8019bd90(0x200)`. Structural in-unit sibling of #50. |
| 54 | 0x80060860 | 0x78 | `stopYoshiBGM__9daPyMng_cFv` | — | NO TWIN | Header-declared. If `mBgmState` bit1 set: loop4 `getPlayer(i)`+`isStatus(0x4b)`, clear bit1, `fn_8019be60(0x200)`. Structural in-unit sibling of #51. |
| 55 | 0x800608E0 | 0x88 | `checkLastAlivePlayer__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** `getEntryNum()==1` test (the `xori/srawi/and/subf/srwi` MWCC idiom for equality-to-1), then a second `mNum<=1` gate, toggling `mBgmState` bit2 for a "last alive" sting via `fn_8019bd90(0x400)`/`fn_8019be60(0x400)`. |
| 56 | 0x80060970 | 0x98 | `executeLastPlayer__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** Loop4 `searchBaseByID(m_yoshiID[i])`→vtable@0xd8; loop4 `getPlayer(i)`→vtable@0xd8. |
| 57 | 0x80060A10 | 0x98 | `executeLastAll__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** Identical skeleton to #56 but vtable slot `0xdc`. **Near-byte-twin of #56 within this unit** — only the two vtable-offset immediates differ — flagged for the author as a one-edit derivation, not eligible as an external map twin. |
| 58 | 0x80060AB0 | 0x158 | `deleteCullingYoshi__9daPyMng_cFv` | — | NO TWIN | Header-declared (returns "whether a Yoshi was deleted"). Loop4 `searchBaseByID(m_yoshiID[i])`, skip flagged/non-culled, vtable@0x6c state-byte test, `isStatus(0xb9)`, squared-distance-to-camera-centre via `sqrt`, tracks farthest, deletes it via `fBase_c::deleteRequest`. Uses `ms_Instance_p__14dBgParameter_c` the same way `incCoin`/`addRest` do. |
| 59 | 0x80060C10 | 0x1A0 | `setHipAttackQuake__9daPyMng_cFiUc` | — | NO TWIN | Header-declared, signature matches. arg1==-1 → no-op; arg0==2 → single `shockMotor`; else unrolled 4-way logic over an `m_playerID`-relative region, a one-shot `lbl_80429FD0` flag gating a 3-word `.data` write near `0xea0(m_playerID)` + `startSystemSe`, **calls `fn_80060DB0`** (see #2 below), then `startShock`/`shockMotor` depending on arg0. |
| 60 | 0x80060DB0 | 0x138 | `fn_80060DB0` | — | NO TWIN | Unnamed in symbol map; **header-undeclared entirely** (not even a placeholder like `fn_8005f4d0`/`fn_8005f570` got one). See flag #2 below — confirmed **static member `void daPyMng_c::fn_80060DB0()`**, zero parameters. |
| 61 | 0x80060EF0 | 0x24 | `checkBonusNoCap__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared.** `mBonusNoCap = (mRest[0] >= 0x63)`. |
| 62 | 0x80060F20 | 0xB8 | `initYoshiPriority__9daPyMng_cFP10daPlBase_c` | — | NO TWIN | **Header-undeclared.** Builds a local 4-byte priority array from `m_yoshiID` matches against `player`'s own field `0x1036` (undeclared `daPlBase_c` field), zeroes the arg's own `0x1036`, finds first free rank via an `mtctr 4` loop, calls `setYoshiPriority`. |
| 63 | 0x80060FE0 | 0x98 | `setYoshiPriority__9daPyMng_cFP10daPlBase_c` | — | NO TWIN | **Header-undeclared.** Loop over `m_yoshiID` bumping other players' `0x1036` priority byte above the arg's saved priority. In-unit companion of #62. |
| 64 | 0x80061080 | 0x5C | `isEffectStop__9daPyMng_cFi` | — | NO TWIN | **Header-undeclared.** `dInfo_c` byte `@0xafc` test, else `getPlayer(idx)` + `dActor_c::mExecStop` bitmask against a `daPlBase_c` byte `@0x38e` (undeclared field, same offset `d_a_player_demo_manager.cpp` calls "not `dBc_c::mLayer`, confirmed at `0x38f`" — one byte away, corroborating evidence for that TU's own uncertainty note). |
| 65 | 0x800610E0 | 0x24 | `isAcceptQuake__9daPyMng_cFi` | — | NO TWIN | **Header-undeclared.** `mActPlayerInfo` bit test, boolified via the generic `neg+or+srwi 31` MWCC nonzero-to-bool idiom — checked against many similarly-shaped accessors codebase-wide and REJECTED as a specific-function claim (too generic, see Rejected candidates #4). |
| 66 | 0x80061110 | 0x48 | `isCreateBalloon__9daPyMng_cFi` | — | NO TWIN | Header-declared. Tests `mCreateItem[mPlayerEntry[idx]]` bit2 and a related array `@+0x80(m_playerID)` `>0`. |
| 67 | 0x80061160 | 0x1A4 | `checkCorrectCreateInfo__9daPyMng_cFv` | — | NO TWIN | **Header-undeclared** (called by `initStage`, #3, as its first call). `mtctr 2`-driven loop touching 2 `m_playerID`-relative slots per iteration (4 total): clamps to `<4`, clamps a related array to `<=6`, conditionally clears a low-3-bit field, clamps a signed value to `[0,scRestMax]`; then clamps `mKinopioMode<=6`; clamps coin total against `scCoinMax` with a 4-slot zero-out; clamps `mScore<=scScoreMax`. |

**67/67 rows are NO TWIN or AUTHORS NOBODY. Zero VERIFIED or PARTIAL external
twins were found or are claimed** — see "Method" above for why, and
"Rejected candidates" below for what was actually tried and ruled out rather
than assumed.

## Authors nobody (flag #1)

Both are foreign functions **defined inline in an already-frozen header**,
whose class has no primary `.cpp` calling them from inside its own TU — so
the *only* reason either shows up inside `d_a_player_manager.cpp`'s object
is that a `daPyMng_c` function calls it directly with `bl`, and the
compiler, per the usual weak/inline-linkage rule, emits its own private copy
of the callee's body into the calling TU's object.

1. **`getCourseIn__10dScStage_cFv`** (`0x8005EC90`, 8 B) — defined at
   `include/game/bases/d_s_stage.hpp:58`:
   `static NOINLINE bool getCourseIn() { return m_isCourseIn; }`. There is
   **no `d_s_stage.cpp`** in `source/` at all — the class's primary TU isn't
   banked, it doesn't exist yet. **Triggering call:** `initStage__9daPyMng_cFv`
   (function #3 above, `0x8005EB10`) calls `dScStage_c::getCourseIn()`
   directly via `bl` at `0x8005EBA0`. Get that one call right (include
   `d_s_stage.hpp`, call `dScStage_c::getCourseIn()` from inside
   `daPyMng_c::initStage`) and this 8-byte weak copy reappears verbatim;
   nothing else in the unit should reference it.

2. **`getFileP__5dCd_cFi`** (`0x8005EE70`, 0x20 B) — defined at
   `include/game/bases/d_cd.hpp:101`:
   ```cpp
   static dCdFile_c *getFileP(int idx) {
       dCdFile_c *file = &m_instance->mFiles[idx];
       if (file->mpAreas != nullptr) { return file; }
       ...
   }
   ```
   `d_cd.cpp` (296 lines) **is** banked, but grepping it finds **no call to
   `getFileP`** anywhere in its own body — so it is never the direct trigger.
   **Triggering call:** `createCourseInit__9daPyMng_cFv` (function #12,
   `0x8005EF50`) calls `dCd_c::getFileP(...)` directly via `bl` at
   `0x8005F0A8`. That is the one call site that must exist for this weak
   body to appear out-of-line. **Extra wrinkle, worth the author's
   attention:** the *same* `getFileP` source is also reached from
   `getPlayerSetPos__9daPyMng_cFUcUc` (#8) and `getPlayerCreateAction__9daPyMng_cFv`
   (#10) — but at both of those call sites the compiler chose to **inline
   the body instead of emitting a `bl`** (visible as the identical
   `mulli...0x3b0; lwz...m_instance__5dCd_c@sda21; add; lwz...0x2c; cmpwi`
   sequence appearing directly in their disassembly). So the same header
   function produces three different outcomes in one TU: inlined twice,
   emitted out-of-line once. If `getFileP` comes out **missing or
   duplicated at the wrong call site**, that's a real finding, not something
   to hand-author — matches the brief's warning exactly.

## Unnamed functions (flag #2)

- **`fn_8005F4D0`** (`0x8005F4D0`, 0x9C B) — r3, r4, r5 are all read as
  genuine incoming arguments before any other use (no self-referential
  `this`-style access), so this **is** a static member function, and its
  signature and behaviour match the header's *existing* placeholder
  declaration exactly: `static void fn_8005f4d0(mVec3_c *pos, int mode, int
  flag);`. Behaviour: loop `i<4`; if `getPlayer(i)==null`, call
  `fn_8005f570(mode,i)` (function #14 above) then `create(i,&pos[i],flag,0)`
  and return; if no free slot exists over all 4 iterations, return false. No
  header edit needed — this one is already correctly documented.

- **`fn_80060DB0`** (`0x80060DB0`, 0x138 B) — **zero parameters**: r3 is
  never read before it is freshly assigned deep inside the loop body
  (`mr r3, r26`), which rules out both "real integer argument" and "`this`
  pointer" for r3. This is a **static member function with no arguments**:
  `void daPyMng_c::fn_80060DB0();` — and it is currently **not represented
  in the header at all**, not even as a placeholder (unlike `fn_8005f4d0`/
  `fn_8005f570`, which the header already documents). Behaviour:
  `SndSceneMgr::sInstance->onPowerImpact()`; loop `i<4` over
  `m_quakeTimer[i]`/`m_quakeEffectFlag[i]` (both confirmed real `.bss`
  statics, `daPyMng_c::m_quakeTimer` at `0x803551B0` and
  `daPyMng_c::m_quakeEffectFlag` at `0x803551C0`, each `int[4]`); if the
  timer is set and the flag is clear, set the flag, `getPlayer(i)`
  (redirected through `getRideYoshi()` when `isStatus(0x4b)`), then
  **`mEf::createEffect` twice using the two `.data` strings at
  `0x80309A28`/`0x80309A3C`** — the exact range the brief marks as "not
  ours" (Codex's `.data 0x80309A28-0x80309A58`). Reporting for Codex's
  benefit: those two strings are the `pcName` argument of the two
  `createEffect` calls in **this** function (`fn_80060DB0`, called only from
  `setHipAttackQuake__9daPyMng_cFiUc`, #59), each `0x12`/`0x16` bytes,
  `Wm_mv_...`-prefixed effect names (exact byte contents in
  `target_data.txt`, not re-transcribed here to avoid a transcription
  error); then `dQuake_c::startShock`. **Caller:** `setHipAttackQuake`
  (#59) calls it unconditionally near its end.

## Overload pairs (flag #3)

`addNum`/`decNum` each have two distinct mangled names, each with a
genuinely distinct, verified-different body (read in full, not assumed):

- `addNum__9daPyMng_cFi` (#32, `bool addNum(int)`, 0x74 B) — sets a per-player
  bit, falls back to calling...
- `addNum__9daPyMng_cFv` (#34, `void addNum()`, 0x18 B) — clamped `++mNum`.
- `decNum__9daPyMng_cFi` (#33, `bool decNum(int)`, 0x7C B) — clears a
  per-player bit, falls back to calling...
- `decNum__9daPyMng_cFv` (#35, `void decNum()`, 0x18 B) — clamped `--mNum`.

No name collision exists in `target_text.txt` (`grep -oP '^\.fn \K[^,]+'`
lists all four distinctly), and I independently confirmed the four bodies
are byte-distinct by reading each in full — so this unit does **not**
reproduce the "one overload's body emitted twice under the same key"
assembler bug the brief warns a previous unit hit. Assign by parameter
count as above.

## Header contradictions (flag #4)

**Signature mismatch (not just "undeclared" — actively wrong):**
- `addRest` — header declares `static bool addRest(int);` (one parameter).
  The target's mangled name is `addRest__9daPyMng_cFiib` = `(int, int,
  bool)`, **three parameters**. Confirmed from the disassembly (#45): the
  function reads three distinct incoming values (`r3`→player index via
  `changeItemKinopioPlrNo`, `r4`→amount added to `mRest[...]`, `r5`→a
  bool-like flag gating the two positional "1-up" sound cues, tested via
  `cmplwi r30,0x1`). Reporting, not editing `d_a_player_manager.hpp`.

**Entirely undeclared static member functions** (behaviour and signature
above per row; listed together here for scan-ability): `initGame`,
`initStage`, `exitStage`, `courseIn`, `setDefaultParam`,
`getPlayerCreateAction`, `create`, `createCourseInit`, `update`,
`isPlayerPauseEnable`, `setPlayer`, `decideCtrlPlrNo`, `setYoshi`,
`releaseYoshi`, `getItemKinopio`, `getCourseInListPlrNo`, `getCoinAll`,
`incRestAll`, `setCourseInStarBGM`, `checkLastAlivePlayer`,
`executeLastPlayer`, `executeLastAll`, `checkBonusNoCap`,
`initYoshiPriority`, `setYoshiPriority`, `isEffectStop`, `isAcceptQuake`,
`checkCorrectCreateInfo`, `addNum()`/`decNum()` (the `Fv` overloads, see
flag #3), and the unnamed `fn_80060DB0` (see flag #2).

**Entirely undeclared static data members**, all confirmed by cross-checking
`target_bss.txt`/`target_sdata.txt` (exact address+size+section for each,
not inferred from `.text` alone):
- `.bss` (`0x80355110`-...): `m_playerID` (`0x10`, likely `fBaseID_e[4]`,
  the backing store `getPlayer`/`setPlayer`/`getCtrlPlayer` index),
  `m_yoshiID` (`0x10`), `m_yoshiFruit` (`0x10`), `mPlayerEntry` (`0x10`,
  *wait* — this one **is** declared, see below), `mCoin` (`0x10`, `int[4]`),
  `m_quakeTimer` (`0x10`, `int[4]`), `m_quakeEffectFlag` (`0x10`, `int[4]`).
  (`mPlayerEntry`/`mPlayerType`/`mPlayerMode`/`mCreateItem`/`mRest`/
  `mCourseInList` are all already declared in the header and their `.bss`
  addresses/sizes match exactly — no contradiction there, listed only to
  show the `.bss` region is otherwise fully accounted for.)
- `.sbss` (small, `@sda21`-addressed, not covered by any `target_*.txt` —
  the brief's table gives only the whole-region size `0x30` by subtraction):
  `m_yoshiColor` (byte array) and `mActPlayerInfo` (already declared) are
  both `@sda21`-addressed, so both plausibly live here alongside the many
  single-word `.sdata` members below.
- `.sdata` (small, single-word, `@sda21`-addressed): `mBonusNoCap`,
  `mQuakeTrigger`, `mBgmState`, `mStopTimerInfoOld`, `mScore`,
  `mKinopioMode` (**declared** already, consistent). Plus the file-scope
  anonymous-namespace constants **confirmed with their actual values** by
  reading `target_sdata.txt`: `scCoinMax = 0x63` (99), `scRestMax = 0x63`
  (99), `scScoreMax = 0x3B9AC9FF` (999,999,999) — i.e.
  `static const int scCoinMax = 99;` etc. in an anonymous namespace at file
  scope (needs `extern`-style forcing per this project's established
  precedent for unreferenced/ODR-sensitive file statics, or plain use is
  enough since all three are referenced multiple times).
- `.rodata`: `l_start_pos_ofs` (referenced from `getPlayerSetPos`, 12-byte
  stride) and the function-local static
  `@LOCAL@getCourseInPlayerModelType__9daPyMng_cFUc@scModelTypeDt` (4-entry
  `dPyMdlMng_c::ModelType_e` table, function-scope `static const`).
- `.sdata2` (pool floats, all confirmed via `target_sdata2.txt`):
  `lbl_8042BD70` (2 ints, `{0x19,0x1a}` = `{25,26}`, a 2-entry table indexed
  by yoshi colour inside `fn_8005F4D0`, likely per-colour `fBaseID_e`
  bases); `lbl_8042BD58` = `504.0f` (used in `createCourseInit`);
  `"@80832_8042BD78"` = `0.5f`, shared by `incCoin`/`addRest`/
  `deleteCullingYoshi` for a `dBgParameter_c`-bounds midpoint —
  **bearing on the brief's own "`.sdata2 ~0x8042BD78` open question"**:
  the value at exactly `0x8042BD78` is a plain `0.5f` pool literal
  referenced from *inside this unit* (three call sites), so whatever else
  is contested about that address range, this particular slot's owner and
  value are not in doubt. Reporting per "if you find evidence bearing on
  it, report it and keep going" — not resolving the open question myself.
- `.data`: two of `fn_80060DB0`'s (#60) `createEffect` name-string
  arguments at `0x80309A28`/`0x80309A3C` — **inside the brief's
  Codex-owned `.data 0x80309A28-0x80309A58` range**, reported above under
  flag #2 for attribution, not touched further.

**One more small cross-TU finding, not a contradiction but worth recording:**
`createCourseInit__9daPyMng_cFv` (#12) calls `fn_8005D280` — this is
**`d_a_player_demo_manager.cpp`'s own already-documented unnamed function**
(see that file's large comment block right above `makeCourseInList`,
`0x8005D280`, 1064 B, "no in-TU caller found" reported by that batch). This
*is* the in-TU caller that batch was looking for — just in a different TU
(`daPyMng_c`, not `daPyDemoMng_c` itself), which is fully consistent with
that batch's own conclusion ("non-static... the most likely caller is
world-map code deciding which players enter the next course" — actually the
real caller turns out to be `daPyMng_c::createCourseInit`, not world-map
code). Worth relaying to whoever integrates both units.

## Rejected candidates

| # | Candidate A | Candidate B | Score / verdict | Why rejected |
|---|---|---|---|---|
| 1 | `getPlayer__9daPyMng_cFi` (#18, tail-call SDA-array-index→`searchBaseByID`) | `daPlBase_c::getHipAttackDamagePlayer` (`d_a_player_base.cpp:4943`) | Shape-similar, text different | `getHipAttackDamagePlayer` reads a **member offset off `this`** (`fManager_c::searchBaseByID(mHipAttackPlayerID)`); ours indexes a **static SDA array by an integer parameter**. Different addressing mode entirely, not adaptable. |
| 2 | `createCourseInit__9daPyMng_cFv`'s float insertion-sort block (`0x8005F160`-`0x8005F388`) | `daPyDemoMng_c::makeCourseInList`'s `cand[]` shift-insert loop (`d_a_player_demo_manager.cpp` ~1388-1406) | Same *intent* (insertion sort), incompatible code | `makeCourseInList` is a tiny unconditional 3-iteration integer shift (`cand[j]=cand[j-1]`); the target block is a much larger runtime-computed-shift-count **float** insertion sort with a real `bdnz`-counted memmove loop. Different element type, different shift mechanism, no textual overlap possible. |
| 3 | `executeLastPlayer__9daPyMng_cFv` (#56) / `executeLastAll__9daPyMng_cFv` (#57) | `d_a_player_demo_manager.cpp`'s "loop4, call something on `daPyMng_c::getCtrlPlayer(i)`" idioms (`onLandStopReq`, `setEnemyStageClearDemo`, `endControlDemoAll`) | Same rough intent, one level of indirection apart | Our functions loop over `m_yoshiID`/`m_playerID` **directly** and dispatch through a raw `fBase_c` vtable slot (`0x6c`/`0xd8`/`0xdc`/`0xe0`) via `fManager_c::searchBaseByID`; the demo-manager idiom instead **calls into `daPyMng_c::getCtrlPlayer()`** — it's a consumer of this unit, one layer removed, and can't be textually identical to the array-owning code itself. |
| 4 | `isAcceptQuake__9daPyMng_cFi` (#65)'s `neg r0,r3; or r0,r0,r3; srwi r3,r0,31` tail | (many `bool`-returning bit-test accessors project-wide) | Too generic to name one twin | This is MWCC's standard "canonicalise a nonzero word to `0`/`1`" codegen for any expression the compiler can't fold into a `cmpwi`/`beq` — it recurs throughout the project against a different register/symbol every time. Checked, no single external twin makes sense to claim. |
| 5 | `getYoshiColor__9daPyMng_cFUc` (#28)'s `li`+`lbzx`+`blr` 3-instruction body | (any trivial byte-array accessor codebase-wide, e.g. `d_cd.hpp`/`d_a_player_base.cpp` one-liners) | Too generic to name one twin | Same reasoning as #4 — the shape is the universal MWCC idiom for `T arr[idx]`; it always references a different SDA symbol, so `canonicalise()` never lets two such functions compare equal even when the underlying C++ pattern is identical. |
| 6 | `update__9daPyMng_cFv` (#15)'s two `for(i<4) if(checkPlayer(i)){p=getCtrlPlayer(i); if(p){...}}` inner loops | `d_a_player_demo_manager.cpp`'s `calcNotGoalPlayer`/`executeGoalDemo_Pole` outer loop skeleton (lines 229-248, 286-314) | Loop **skeleton** proven-good; whole function not a twin | The outer control-flow shape is demonstrably correct C++ (that exact skeleton already compiles to this exact machine shape in the landed demo-manager TU), so it's a legitimate STYLE precedent recorded in row #15's Notes — but `update`'s bodies diverge immediately inside the `if`, so the *function* isn't a twin or partial match of any single sibling. Recording here so the whole-function candidate isn't retried. |
| 7 | `addRest__9daPyMng_cFiib` (#45) / `incCoin__9daPyMng_cFi` (#44)'s `getRemotePlayer`+`cvtSndObjctPos`+`startSound` positional-stereo-cue pairs | Similar-looking sound-cue code in `d_a_player.cpp` (`dAcPy_c` members) | Different addressing basis | `d_a_player.cpp`'s cue code is `this`-relative (`mPos`), a member function's own actor position; ours builds a temporary `mVec2_c` pair from `dBgParameter_c`-derived screen-bounds constants on the stack. Not the same data source, not adaptable as a twin. |

## Summary

- 67 real functions in scope (not 68 — see count note above).
- 2 **AUTHORS NOBODY** (`getCourseIn__10dScStage_cFv`, `getFileP__5dCd_cFi`) —
  do not author, verify their triggering call sites instead (both identified
  above).
- 0 VERIFIED external twins, 0 PARTIAL external twins — structurally
  impossible for this unit for the reason given under "Method", confirmed by
  seven concrete rejected-candidate searches rather than assumed.
- 65 functions are NO TWIN and must be written from the disassembly; each
  row above already carries the behavioural summary an authoring agent needs
  to start from (loop shape, branch conditions, callees, and which raw
  offsets are still-undeclared fields) so the map is still useful for
  authoring speed even without adaptable bodies.
- 1 real signature contradiction (`addRest`, one declared param vs. three
  actual) and ~30 undeclared static members/functions catalogued above —
  the header is far more incomplete than usual for this unit's size; do not
  edit it, report and let the lead reconcile.
