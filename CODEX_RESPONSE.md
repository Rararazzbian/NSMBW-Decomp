# Codex Round 6 response

## Task A: link blockers for d_a_player_manager.cpp

The TU calls 68 unique external symbols (67 `bl` targets + `sqrt`).
Of these, 25 are self-calls (functions our TU defines), 2 are known weak
inlines the TU emits, 3 are CRT helpers, 13 are already pinned in syms.txt,
and **25 are blockers** that need new syms.txt entries.

### Already pinned (no action needed)

createEffect__3mEfFPCcUlPC7mVec3_cPC7mAng3_cPC7mVec3_c=0x8016C9D0
cvtSndObjctPos__6dAudioFRC7mVec2_c=0x8006A3F0
fn_800e25a0__11dScoreMng_cFUlii=0x800E25A0
fn_8014eb70__9daYoshi_cFP7dAcPy_ci=0x8014EB70
fn_8019bd90__11SndSceneMgrFi=0x8019BD90
fn_8019be60__11SndSceneMgrFi=0x8019BE60
getDispCenterX__8dGameComFv=0x800B30C0
getRemotePlayer__6dAudioFi=0x80069530
rnd__8dGameComFv=0x800B2F00
shockMotor__8dQuake_cFScQ28dQuake_c12TYPE_SHOCK_eib=0x800D8CA0
startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul=0x80198D70
startSystemSe__11SndAudioMgrFUiUl=0x801954C0
startSystemSe__11SndAudioMgrFUlUl=0x801954B0

### Weak inlines (TU emits its own copy ? no action)

- getCourseIn__10dScStage_cFv
- getFileP__5dCd_cFi

### CRT helpers (no action)

- _savegpr_25, _savegpr_27, _restgpr_25, _restgpr_27, sqrt__Q23EGG7Math<f>Ff

### Lines to ADD to syms.txt (25 blockers)

```
addScore__11dMultiMng_cFii=0x800CEA40
construct__8dActor_cFUsUlPC7mVec3_cPC7mAng3_cUc=0x80064610
deleteRequest__7fBase_cFv=0x80162650
fn_8005D280=0x8005D280
fn_80060DB0=0x80060DB0
getGameDisplay__10dScStage_cFv=0x80101A70
getNextGotoP__9dCdFile_cFUc=0x8008E3D0
getRideYoshi__7dAcPy_cFv=0x80139A90
incCoin__11dMultiMng_cFi=0x800CEAC0
initCourseIn__13daPyDemoMng_cFv=0x8005B470
initStage__11dMultiMng_cFv=0x800CE950
initStage__13daPyDemoMng_cFv=0x8005B430
init__13daPyDemoMng_cFv=0x8005B4A0
isStatus__10daPlBase_cFi=0x80056CF0
onPowerImpact__11SndSceneMgrFv=0x8019C620
onStatus__10daPlBase_cFi=0x80056C70
searchBaseByID__10fManager_cF9fBaseID_e=0x80162E40
setCoinNum__14dGameDisplay_cFi=0x80159AA0
setCollect__14dGameDisplay_cFv=0x80159C30
setPauseEnable__14PauseManager_cFb=0x800D0C10
setPlayNum__14dGameDisplay_cFPi=0x801599C0
setScore__14dGameDisplay_cFi=0x80159DF0
startMiss__11SndSceneMgrFv=0x8019C470
startShock__8dQuake_cFScQ28dQuake_c12TYPE_SHOCK_eiib=0x800D8BF0
update__13daPyDemoMng_cFv=0x8005B550
update__14dPyEffectMng_cFv=0x800D2E70
```

All 25 confirmed in `bin/dtk/wiimj2d_symbols.txt`. No blockers fall outside
wiimj2d.dol sections.

### Lines to REMOVE from syms.txt (31 entries ? our TU now defines these)

```
addNum__9daPyMng_cFi=0x8005FDB0
addScore__9daPyMng_cFii=0x80060690
changeItemKinopioPlrNo__9daPyMng_cFRi=0x80060170
createYoshi__9daPyMng_cFR7mVec3_ciP7dAcPy_c=0x8005E9A0
decNum__9daPyMng_cFi=0x8005FE30
decRest__9daPyMng_cFi=0x80060600
deleteCullingYoshi__9daPyMng_cFv=0x80060AB0
fn_8005f570__9daPyMng_cF16PLAYER_POWERUP_ei=0x8005F570
getCourseInPlayerModelType__9daPyMng_cFUc=0x8005FBE0
getCtrlPlayer__9daPyMng_cFi=0x8005FB90
getEntryNum__9daPyMng_cFv=0x8005FFB0
getItemKinopioNum__9daPyMng_cFv=0x80060010
getNumInGame__9daPyMng_cFv=0x8005FEF0
getPlayerIndex__9daPyMng_cF13PLAYER_TYPE_e=0x80060110
getPlayerSetPos__9daPyMng_cFUcUc=0x8005ED90
getPlayer__9daPyMng_cFi=0x8005F900
getYoshiColor__9daPyMng_cFUc=0x8005FC40
getYoshiDirectP__9daPyMng_cFi=0x8005FB70
getYoshiFruit__9daPyMng_cFUc=0x8005FC50
getYoshiNum__9daPyMng_cFv=0x8005FB00
getYoshi__9daPyMng_cFi=0x8005FA60
incCoin__9daPyMng_cFi=0x80060250
isCreateBalloon__9daPyMng_cFi=0x80061110
setCarryOverYoshiInfo__9daPyMng_cFUcUci=0x8005FC20
setHipAttackQuake__9daPyMng_cFiUc=0x80060C10
setPlayer__9daPyMng_cFiP7dAcPy_c=0x8005F8C0
startMissBGM__9daPyMng_cFi=0x800607D0
startStarBGM__9daPyMng_cFv=0x80060720
startYoshiBGM__9daPyMng_cFv=0x80060830
stopStarBGM__9daPyMng_cFv=0x80060750
stopYoshiBGM__9daPyMng_cFv=0x80060860
```

Data symbols (mPlayerEntry, mNum, mCtrlPlrNo, m_star_time, mKinopioMode,
etc.) are **not** in this list ? the TU only defines .text, not .bss/.data,
so those data pins stay.

### Edge case: fn_8005F4D0 (judgment needed)

The TU defines `fn_8005F4D0` at address 0x8005F4D0. syms.txt has
`fn_8005f4d0__9daPyMng_cFP7mVec3_cii=0x8005F4D0`. Different mangled name
but **same address**. If the linker resolves by address this is a duplicate.
The sub-agent flagged it for retention because names dont match exactly.
You should decide ? I lean toward removing it since the addresses collide.

---

## Task B: EGG::Effect unnamed data

### Virtual count: 37
vtable at 0x80350AF8, size 0x9C. (0x9C - 8) / 4 = 37 virtuals. The existing
header has 34 virtual declarations plus the destructor = 35, missing 2.
Need to audit.

### Constructor/destructor
Constructor at 0x802D7D90 (size 0x74), destructor at 0x802D7E10 (size 0x5C).
Both are in `bin/dtkspl/obj/auto_03_802D72FC_text.o`.

Key finding: constructor calls `ExEffectParam::ExEffectParam` at object
offset 0x7C. Destructor calls `ExEffectParam::~ExEffectParam` at same offset.
So `ExEffectParam` is embedded at 0x7C and occupies the remaining 0x98 bytes
(0x7C through 0x113).

### Effect-level fields (offsets 0x00-0x7B)

```
0x00  void* vptr
0x04  u8 state/creation flag (ctor init to 0)
0x05  u8 pad[3]
0x08  u8 pad[0x1C]          // 28 bytes unknown
0x24  u32 effect state
0x28  u32 effect flags
0x2C  float scaleX
0x30  float scaleY
0x34  float scaleZ
0x38  float posX
0x3C  float posY
0x40  float posZ
0x44  MTX34 matrix (PSMTXIdentity called here)
0x74  nw4r::ef::HandleBase effect handle
0x7C  ExEffectParam param    // embedded, 0x98 bytes
```

### ExEffectParam fields (relative to Effect + 0x7C)

```
+0x00  u8 pad[4]             // vptr or flags
+0x04  u32 status flags      // ctor inits to 0xFFFFFFFF
+0x0C  u16 life
+0x10  float emitRatio
+0x14  u16 emitInterval
+0x16  u16 emitEmitDiv
+0x18  s8 initVelocityRandom
+0x1C  float powerYAxis
+0x20  float powerRadiationDir
+0x24  float powerSpecDir
+0x28  float powerSpecDirAdd
+0x2C  VEC3 specDir
+0x38  VEC3 specDirAdd
+0x44  nw4r::ut::Color color
+0x48  register color array (4 Color objects)
+0x58  VEC2 defaultParticleSize  // overlaps with register colors
+0x60  VEC2 particleScale        // overlaps with register colors
+0x68  VEC3 defaultParticleRotate
+0x74  VEC3 particleRotate
+0x80  VEC3 emitterSize
+0x8C  tail bytes (emitter mode flag etc.)
```

### Evidence hierarchy
- Constructor/destructor: embedded object at 0x7C, initializations at 0x04, 0x24, 0x28, 0x44
- Setter virtuals: each setter adds 0x7C to this and tail-calls the corresponding ExEffectParam setter
- ExEffectParam setters store to the offsets listed above with matching widths

### Confidence
- **High**: embedded ExEffectParam at 0x7C, life at +0x0C, scalar params at +0x10..+0x28, vector params at +0x2C..+0x8C
- **Medium**: Effect-level fields at 0x04-0x28 (ctor evidence only, no setter cross-validation)
- **Low**: 0x08-0x23 pad (28 bytes with no observed access in the constructor)

sizeof stays 0x114. The region 0x08-0x23 remains the biggest unknown.

Partial progress is a fine result ? see CODEX_PROMPT.md.

Offset-perturbing: NO if proposed as a scratch/ copy. Header edit would need
the 37-virtual audit completed first.

Evidence, proposal, compiled status, and confidence are in
`scratch/codex_round6/task_b_effect.txt`.

---

## Round summary

| Item | Count | Status |
|---|---|---|
| Blocker addresses resolved | 25 | All verified in wiimj2d_symbols.txt |
| syms.txt entries to add | 25 | Ready |
| syms.txt entries to remove | 31 | Verified, data symbols excluded |
| fn_8005F4D0 conflict | 1 | Judgment needed |
| EGG::Effect fields named | ~25 | Partial, 0x08-0x23 unknown |
| EGG::Effect virtual count | 37 | Header has 35, 2 missing |
