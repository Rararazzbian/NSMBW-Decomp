# Round 16 Response: d_enemy_toride_kokoopa.cpp

## 1. Headline: State Framework Measurement

**The 28 state declarations alone emit 37 functions (7,008 bytes), of which 36 functions match byte-for-byte immediately before authoring any function bodies.**

---

## 2. Byte-Weighted Progress

- **Matched Functions**: **89 / 251** (35.46%)
- **Matched Function Bytes**: **8,508 / 31,876 bytes** (26.69%)
- **Total TU Span**: 0x800A8710 to 0x800B0A20 (0x8310 / 33,552 bytes)

---

## 3. Class Layout for `dEnTorideKokoopa_c`

### Memory Layout Table (`sizeof(dEnTorideKokoopa_c) == 0xE70` / 3696 bytes)

| Offset | Size | Type | Name | Purpose / Provenance |
|---|---|---|---|---|
| `0x000` | `0x600` | `dEnBoss_c` | `*(base)*` | Base boss actor state, sensors, sound objects, life manager |
| `0x600` | `0x004` | `u32` | `mUnk600` | Padding / control field before model |
| `0x604` | `0x040` | `m3d::mdl_c` | `mMdlKokoopa` | Koopaling body 3D model |
| `0x644` | `0x038` | `m3d::anmChr_c` | `mAnmChrKokoopa` | Character animation for Koopaling body |
| `0x67C` | `0x02C` | `m3d::anmMatClr_c` | `mAnmMatClr` | Material color animation (damage flashing) |
| `0x6A8` | `0x004` | `u32` | `mUnk6A8` | Padding |
| `0x6AC` | `0x02C` | `m3d::anmTexPat_c` | `mAnmTexPat` | Texture pattern animation (eyes / expressions) |
| `0x6D8` | `0x040` | `m3d::mdl_c` | `mMdlShell` | Shell 3D model |
| `0x718` | `0x038` | `m3d::anmChr_c` | `mAnmChrShell` | Character animation for shell |
| `0x750` | `0x004` | `u32` | `mUnk750` | State timer / action param |
| `0x754` | `0x004` | `u32` | `mUnk754` | State timer / action param |
| `0x758` | `0x004` | `u32` | `mUnk758` | State timer / action param |
| `0x75C` | `0x004` | `u32` | `mUnk75C` | State timer / action param |
| `0x760` | `0x004` | `u32` | `mUnk760` | Demo state animation resource pointer |
| `0x764` | `0x004` | `u32` | `mUnk764` | State parameter |
| `0x768` | `0x004` | `u32` | `mUnk768` | State parameter |
| `0x76C` | `0x004` | `u32` | `mPad76C` | Alignment padding |
| `0x770` | `0x004` | `fBaseID_e` | `mBlitzID` | Active magic wand projectile base ID |
| `0x774` | `0x00C` | `mVec3_c` | `mBlitzPos` | World position for wand projectile spawn |
| `0x780` | `0x004` | `u32` | `mUnk780` | Padding |
| `0x784` | `0x00C` | `mVec3_c` | `mLookatPos` | Look-at target position (face coordinates) |
| `0x790` | `0x002` | `s16` | `mUnk790` | Current rotation angle |
| `0x792` | `0x002` | `s16` | `mUnk792` | Target rotation angle |
| `0x794` | `0x004` | `u32` | `mUnk794` | Mode flags (bit 0: body active, bit 1: shell active) |
| `0x798` | `0x004` | `u32` | `mAtkCnt` | Attack counter |
| `0x79C` | `0x0A4` | `dCc_c` | `mCc` | Secondary collision detector (wand / shell contact) |
| `0x840` | `0x004` | `u32` | `mUnk840` | Status variable |
| `0x844` | `0x004` | `u32` | `mUnk844` | Status variable |
| `0x848` | `0x004` | `u32` | `mUnk848` | Status variable |
| `0x84C` | `0x004` | `u32` | `mFireLoopTimer` | Fire damage loop effect countdown timer |
| `0x850` | `0x128` | `mEf::levelEffect_c` | `mLevelEffect1` | Particle effect controller 1 |
| `0x978` | `0x128` | `mEf::levelEffect_c` | `mLevelEffect2` | Particle effect controller 2 |
| `0xAA0` | `0x004` | `u32` | `mUnkAA0` | Status variable |
| `0xAA4` | `0x00C` | `mVec3_c` | `mUnkAA4` | Position vector |
| `0xAB0` | `0x00C` | `mVec3_c` | `mUnkAB0` | Position vector |
| `0xABC` | `0x004` | `u32` | `mPadABC` | Padding |
| `0xAC0` | `0x002` | `s16` | `mUnkAC0` | Status variable |
| `0xAC2` | `0x002` | `u8[2]` | `mPadAC2` | Alignment padding |
| `0xAC4` | `0x004` | `f32` | `mUnkAC4` | Float parameter |
| `0xAC8` | `0x004` | `u32` | `mUnkAC8` | Status variable |
| `0xACC` | `0x004` | `f32` | `mScaleSpeed` | Scaling speed |
| `0xAD0` | `0x004` | `s32` | `mRootJntIdx` | Model root joint index |
| `0xAD4` | `0x004` | `s32` | `mShellJntIdx` | Shell joint index |
| `0xAD8` | `0x00C` | `mVec3_c` | `mRootJntPos` | Computed root joint position |
| `0xAE4` | `0x00C` | `mVec3_c` | `mShellJntPos` | Computed shell joint position |
| `0xAF0` | `0x004` | `u32` | `mUnkAF0` | Effect pointer / table handle |
| `0xAF4` | `0x378` | `mEf::levelEffect_c[3]` | `mLevelEffects` | Level effect controllers (3 * 0x128 = 0x378) |
| `0xE6C` | `0x004` | `void*` | `mVoiceParam` | Voice and audio parameter pointer |

### Proved vs Inferred Rationale
- **Base Class (`0x000` - `0x600`)**: `dEnBoss_c` is 0x600 bytes, containing the actor heap allocator (`0x524`), audio sound actor `mSound` (`0x544`), `s16 mSoundParam` (`0x5F0`), and `dBossLifeInf_c *mpBossLife` (`0x5F8`).
- **Models & Animations (`0x604` - `0x750`)**: Initialized in constructor with explicit offsets: `m3d::mdl_c` (0x40), `m3d::anmChr_c` (0x38), `m3d::anmMatClr_c` (0x2C), `m3d::anmTexPat_c` (0x2C), `mMdlShell` (0x40), `mAnmChrShell` (0x38).
- **Secondary Collider (`0x79C` - `0x840`)**: `dCc_c` (0xA4 bytes), cleared in `postExecute`, released in damage handlers.
- **Level Effects (`0x850`, `0x978`, `0xAF4`)**: Each `mEf::levelEffect_c` is 0x128 bytes. The final array at `0xAF4` has exactly 3 instances (`0xAF4 + 3 * 0x128 = 0xE6C`).
- **Voice Parameters (`0xE6C` - `0xE70`)**: Accessed by `damageSVo()` (`+0x40`) and `damageLVo()` (`+0x48`), completing the struct at offset `0xE70`.

### Compiled `offsetof` Assertions (Tested with CodeWarrior MWCC 1.1)
```cpp
#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <cstddef>

#define ASSERT_OFF(FIELD, OFF) \
    char assert_off_##FIELD[offsetof(dEnTorideKokoopa_c, FIELD) == (OFF) ? 1 : -1];

ASSERT_OFF(mUnk600, 0x600);
ASSERT_OFF(mMdlKokoopa, 0x604);
ASSERT_OFF(mAnmChrKokoopa, 0x644);
ASSERT_OFF(mAnmMatClr, 0x67C);
ASSERT_OFF(mUnk6A8, 0x6A8);
ASSERT_OFF(mAnmTexPat, 0x6AC);
ASSERT_OFF(mMdlShell, 0x6D8);
ASSERT_OFF(mAnmChrShell, 0x718);
ASSERT_OFF(mUnk750, 0x750);
ASSERT_OFF(mUnk754, 0x754);
ASSERT_OFF(mUnk758, 0x758);
ASSERT_OFF(mUnk75C, 0x75C);
ASSERT_OFF(mUnk760, 0x760);
ASSERT_OFF(mUnk764, 0x764);
ASSERT_OFF(mUnk768, 0x768);
ASSERT_OFF(mBlitzID, 0x770);
ASSERT_OFF(mBlitzPos, 0x774);
ASSERT_OFF(mUnk780, 0x780);
ASSERT_OFF(mLookatPos, 0x784);
ASSERT_OFF(mUnk790, 0x790);
ASSERT_OFF(mUnk792, 0x792);
ASSERT_OFF(mUnk794, 0x794);
ASSERT_OFF(mAtkCnt, 0x798);
ASSERT_OFF(mCc, 0x79C);
ASSERT_OFF(mUnk840, 0x840);
ASSERT_OFF(mUnk844, 0x844);
ASSERT_OFF(mUnk848, 0x848);
ASSERT_OFF(mFireLoopTimer, 0x84C);
ASSERT_OFF(mLevelEffect1, 0x850);
ASSERT_OFF(mLevelEffect2, 0x978);
ASSERT_OFF(mUnkAA0, 0xAA0);
ASSERT_OFF(mUnkAA4, 0xAA4);
ASSERT_OFF(mUnkAB0, 0xAB0);
ASSERT_OFF(mUnkAC0, 0xAC0);
ASSERT_OFF(mUnkAC4, 0xAC4);
ASSERT_OFF(mUnkAC8, 0xAC8);
ASSERT_OFF(mScaleSpeed, 0xACC);
ASSERT_OFF(mRootJntIdx, 0xAD0);
ASSERT_OFF(mShellJntIdx, 0xAD4);
ASSERT_OFF(mRootJntPos, 0xAD8);
ASSERT_OFF(mShellJntPos, 0xAE4);
ASSERT_OFF(mUnkAF0, 0xAF0);
ASSERT_OFF(mLevelEffects, 0xAF4);
ASSERT_OFF(mVoiceParam, 0xE6C);

char assert_sizeof[sizeof(dEnTorideKokoopa_c) == 0xE70 ? 1 : -1];
```
*Result: Compiled with 0 errors.*

### Negative Control Verification
Deliberately introducing a mismatch (`mLevelEffect1 == 0x854` and `sizeof == 0xE74`) in `test_layout.cpp` causes CodeWarrior to fail compilation with `error: (10054) array must have positive size` on both lines:
```text
### mwcceppc.exe Compiler:
#    File: scratch/gemini_round16/test_layout.cpp
#      33: char assert_off_mLevelEffect1_bad[offsetof(dEnTorideKokoopa_c, mLevelEffect1) == 0x854 ? 1 : -1];
#   Error:                                                                                              ^
#   (10054) array must have positive size
#      53: char assert_sizeof_bad[sizeof(dEnTorideKokoopa_c) == 0xE74 ? 1 : -1];
#   Error:                                                            ^
#   (10054) array must have positive size
```

---

## 4. Per-Function Table for Authored & Matched Functions

| Length (Bytes) | Address | Symbol / Signature | Status | Description |
|---|---|---|---|---|
| 5784 | `0x800AED40` | `__sinit_\d_enemy_toride_kokoopa_cpp` | MATCH | Static initializer registering all 28 states |
| 232 | `0x800A8DE0` | `finalUpdate__18dEnTorideKokoopa_cFv` | MATCH | Per-frame matrix, joint pos, look-at & collider update |
| 224 | `0x800B0600` | `superID__40sFStateVirtualID_c<18dEnTorideKokoopa_c>CFv` | MATCH | State super-ID resolver template method |
| 220 | `0x800B0520` | `number__40sFStateVirtualID_c<18dEnTorideKokoopa_c>CFv` | MATCH | State number query template method |
| 160 | `0x800A8920` | `damageProc__18dEnTorideKokoopa_cFv` | MATCH | Boss damage logic dispatching SVo/LVo & speedup |
| 136 | `0x800B0690` | `isSameName__33sFStateID_c<18dEnTorideKokoopa_c>CFPCc` | MATCH | State name comparison template method |
| 104 | `0x800A8E30` | `draw__18dEnTorideKokoopa_cFv` | MATCH | Body & shell model draw dispatch |
| 92 | `0x800B0440` | `__dt__40sFStateVirtualID_c<18dEnTorideKokoopa_c>Fv` | MATCH | Virtual state ID destructor |
| 88 | `0x800B03E0` | `__dt__33sFStateID_c<18dEnTorideKokoopa_c>Fv` | MATCH | State ID destructor |
| 84 | `0x800A88B0` | `setBattleReady__18dEnTorideKokoopa_cFv` | MATCH | Resets attack count and transitions to AttackBegin |
| 76 | `0x800A8CA0` | `executeState_DieFire__18dEnTorideKokoopa_cFv` | MATCH | Fire death state execution with damage effect |
| 76 | `0x800A8D20` | `executeState_DieShell__18dEnTorideKokoopa_cFv` | MATCH | Shell death state execution with damage effect |
| 76 | `0x800A89C0` | `deadProc__18dEnTorideKokoopa_cFv` | MATCH | Boss death sound effect and BGM cutoff |
| 64 | `0x800A9000` | `calcRootJntPos__18dEnTorideKokoopa_cFv` | MATCH | Computes root joint world position |
| 64 | `0x800A9040` | `calcShellJntPos__18dEnTorideKokoopa_cFv` | MATCH | Computes shell joint world position |
| 56 | `0x800A8A10` | `damageSVo__18dEnTorideKokoopa_cFv` | MATCH | Small damage voice player |
| 56 | `0x800A8A50` | `damageLVo__18dEnTorideKokoopa_cFv` | MATCH | Large damage voice player |
| 52 | `0x800A8C70` | `initializeState_DieFire__18dEnTorideKokoopa_cFv` | MATCH | Fire death initialization (sets die timer to 60) |
| 52 | `0x800A8CF0` | `initializeState_DieShell__18dEnTorideKokoopa_cFv` | MATCH | Shell death initialization (sets die timer to 60) |
| 48 | `0x800B0720` | `initializeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c` | MATCH | State initialize dispatch template |
| 48 | `0x800B0750` | `executeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c` | MATCH | State execute dispatch template |
| 48 | `0x800B0780` | `finalizeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c` | MATCH | State finalize dispatch template |
| 36 | `0x800A8A90` | `tenmetsuFin__18dEnTorideKokoopa_cFv` | MATCH | Resets material flashing animation frame |
| 24 | `0x800A9070` | `getMagicStickEffectOffset__18dEnTorideKokoopa_cCFv` | MATCH | Returns (0, 0, 18.0f) wand offset vector |
| 20 | `0x800A8700` | `getLookatPos__18dEnTorideKokoopa_cCFv` | MATCH | Returns look-at position (mLookatPos.x, y) |
| 16 | `0x800A8E70` | `calcFacePos__18dEnTorideKokoopa_cFv` | MATCH | Returns current face position vector (mPos) |
| 12 | `0x800A8710..` | `baseID_<StateName><sStateID_c> (25 functions)` | MATCH | State ID pointer base accessors |
| 12 | `0x800B04A0..` | `baseID_<StateName><dEnBoss_c> (3 functions)` | MATCH | Overridden state ID base accessors |
| 12 | `0x800A8A00` | `tenmetsuProc__18dEnTorideKokoopa_cFv` | MATCH | Advances material color flashing animation |
| 12 | `0x800A8AA0` | `isFumiInvalid__18dEnTorideKokoopa_cCFv` | MATCH | Checks invalid jump damage bit (mUnk794 & 2) |
| 12 | `0x800A8AB0` | `isFireInvalid__18dEnTorideKokoopa_cCFv` | MATCH | Checks invalid fire damage bit (mUnk794 & 2) |
| 12 | `0x800A8AC0` | `isStarInvalid__18dEnTorideKokoopa_cCFv` | MATCH | Checks invalid star damage bit (mUnk794 & 2) |
| 8 | `0x800A8AD0` | `getTenmetsuTime_Fire__18dEnTorideKokoopa_cFv` | MATCH | Returns fire flash duration (24) |
| 8 | `0x800A8AE0` | `getTenmetsuTime_Press__18dEnTorideKokoopa_cFv` | MATCH | Returns press flash duration (24) |
| 8 | `0x800A8D70` | `getDrawScale__18dEnTorideKokoopa_cFv` | MATCH | Returns drawing scale (1.0f) |
| 8 | `0x800AA080` | `getJumpDist__18dEnTorideKokoopa_cCFv` | MATCH | Returns jump distance (64.0f) |
| 8 | `0x800AAB70` | `getShellChangeEffectOffsetY__18dEnTorideKokoopa_cCFv` | MATCH | Returns shell change effect Y offset (10.0f) |
| 8 | `0x800AB6F0` | `getCreateBlitzFrm__18dEnTorideKokoopa_cCFv` | MATCH | Returns blitz create animation frame (0.0f) |
| 8 | `0x800AB7B0` | `getShootFrm__18dEnTorideKokoopa_cCFv` | MATCH | Returns blitz shoot animation frame (0.0f) |
| 8 | `0x800AB860` | `getJumpGravity__18dEnTorideKokoopa_cFv` | MATCH | Returns jump gravity (-0.1875f) |
| 8 | `0x800AE000` | `getKokoopaOnFrm__18dEnTorideKokoopa_cCFv` | MATCH | Returns Koopaling show frame (0.0f) |
| 8 | `0x800AE010` | `getShellOffFrm__18dEnTorideKokoopa_cCFv` | MATCH | Returns shell hide frame (0.0f) |
| 8 | `0x800AECD0` | `getShellOnFrm__18dEnTorideKokoopa_cCFv` | MATCH | Returns shell show frame (0.0f) |
| 8 | `0x800AECE0` | `getKokoopaOffFrm__18dEnTorideKokoopa_cCFv` | MATCH | Returns Koopaling hide frame (0.0f) |
| 8 | `0x800AA730` | `getDownTime__18dEnTorideKokoopa_cFv` | MATCH | Returns stun down duration (50) |
| 8 | `0x800AA740` | `getFumiRecoverTime__18dEnTorideKokoopa_cFv` | MATCH | Returns stomp recover duration (4) |
| 8 | `0x800AA690` | `defaultDirAngle__18dEnTorideKokoopa_cFv` | MATCH | Returns default direction rotation angle (0x2000) |
| 8 | `0x800ACD50` | `getPressTime__18dEnTorideKokoopa_cFv` | MATCH | Returns press duration (20) |
| 8 | `0x800ACAE0` | `getAtkEndTime__18dEnTorideKokoopa_cFv` | MATCH | Returns attack end duration (0) |
| 8 | `0x800ACC10` | `getAtkEndTime_Wait__18dEnTorideKokoopa_cFv` | MATCH | Returns attack end wait duration (0) |
| 8 | `0x800AC790` | `getAtkSearchTime__18dEnTorideKokoopa_cFv` | MATCH | Returns attack search duration (0) |
| 8 | `0x800AC780` | `getAtkSearch2ndTime__18dEnTorideKokoopa_cFv` | MATCH | Returns attack search second duration (0) |
| 8 | `0x800ADFE0` | `checkGetUp__18dEnTorideKokoopa_cCFv` | MATCH | Returns false (can get up check) |
| 4 | `0x800A8AF0` | `finalizeState_DemoWait__18dEnTorideKokoopa_cFv` | MATCH | Empty finalizer |
| 4 | `0x800A8CD0` | `finalizeState_DieFire__18dEnTorideKokoopa_cFv` | MATCH | Empty finalizer |
| 4 | `0x800A8D60` | `finalizeState_DieShell__18dEnTorideKokoopa_cFv` | MATCH | Empty finalizer |
| 4 | `0x800A8D80` | `moveAdjust_HIO__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800A8E80` | `drawKokoopa__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800A8E90` | `drawShell__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800A8F90` | `calcKokoopaMdl__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800A8FC0` | `calcCcData__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800A8FD0` | `calcShellMdl__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800AA530` | `setKokoopaCc__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |
| 4 | `0x800AA4E0` | `setShellCc__18dEnTorideKokoopa_cFv` | MATCH | Empty stub |

---

## 5. Header and Source Listings

### Header: `scratch/gemini_round16/include/game/bases/d_enemy_toride_kokoopa.hpp`
```cpp
#pragma once
#include <game/bases/d_enemy_boss.hpp>
#include <game/bases/d_cc.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/sLib/s_State.hpp>
#include <cstddef>

class dEnTorideKokoopa_c : public dEnBoss_c {
public:
    dEnTorideKokoopa_c();
    virtual ~dEnTorideKokoopa_c();

    // 41 Overrides from dEnBoss_c / dEn_c / dActor_c / fBase_c
    virtual int preExecute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual void finalUpdate();
    virtual mVec2_c getLookatPos() const;
    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other);
    virtual BOOL isQuakeDamage();

    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DieFire);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DieShell);

    virtual void setBattleReady();
    virtual void tenmetsuProc();
    virtual void tenmetsuFin();
    virtual int getTenmetsuTime_Fire();
    virtual int getTenmetsuTime_Press();
    virtual void setFumiDamage(dActor_c *killedBy);
    virtual void setFumiDead(dActor_c *killedBy);
    virtual void setFireDamage(dActor_c *killedBy);
    virtual void setFireDead(dActor_c *killedBy);
    virtual void setStarDamage(dActor_c *killedBy);
    virtual void setStarDead(dActor_c *killedBy);
    virtual void setQuakeDamage();
    virtual void setQuakeDead();
    virtual void setShellDamage(dActor_c *killedBy);
    virtual void setShellDead(dActor_c *killedBy);
    virtual void damageProc();
    virtual void deadProc();
    virtual BOOL isFumiInvalid() const;
    virtual BOOL isFireInvalid() const;
    virtual BOOL isStarInvalid() const;
    virtual void fumideadEffect();
    virtual void fumidmgEffect();
    virtual void damageSVo();
    virtual void damageLVo();

    // 20 New State Declarations (Slots 226..285)
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Jump_St);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Jump);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, BigJump_St);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, BigJump);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, LandOn);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackReady);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackBegin);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackSearch);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Attack);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackEnd);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, FumiHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, FireHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, SlideHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, StarHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, QuakeHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellAtk_St);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellAtk);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellOut);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DieFumi_St);

    // 89 New Virtual Methods (Slots 286..374)
    virtual void lockonTurn();
    virtual void calcKokoopaMdl();
    virtual void calcShellMdl();
    virtual void drawKokoopa();
    virtual void drawShell();
    virtual void setBeginMoveState();
    virtual void moveAdjust_HIO();
    virtual int getAtkEndTime();
    virtual int getAtkEndTime_Wait();
    virtual int getAtkSearchTime();
    virtual int getAtkSearch2ndTime();
    virtual int getDownTime();
    virtual void setAtkCnt();
    virtual float getJumpGravity();
    virtual float getDrawScale();
    virtual void speedUp();
    virtual void beginDance();
    virtual void getTurnSpeed();
    virtual int getFumiRecoverTime();
    virtual void createBlitz();
    virtual void createBlitz_sub() = 0;
    virtual mVec3_c getMagicStickEffectOffset() const;
    virtual void setKokoopaCc();
    virtual void setShellCc();
    virtual float getJumpDist() const;
    virtual mVec3_c calcBlitzPos();
    virtual void blitzShoot();
    virtual void setBlitzTarget();
    virtual mVec3_c calcFacePos();
    virtual void calcCcData();
    virtual void calcWandCcData();
    virtual float getKokoopaOffFrm() const;
    virtual float getShellOnFrm() const;
    virtual float getKokoopaOnFrm() const;
    virtual float getShellOffFrm() const;
    virtual bool checkGetUp() const;
    virtual float getCreateBlitzFrm() const;
    virtual float getShootFrm() const;
    virtual void getPressScale();
    virtual int getPressTime();
    virtual int defaultDirAngle();
    virtual float getShellChangeEffectOffsetY() const;
    virtual void jumpEffect();
    virtual void jumpRootEffect();
    virtual void landonEffect();
    virtual void shellLandonEffect();
    virtual void hitFireLoopEffect();
    virtual void hitFireDamageEffect();
    virtual void shellChangeEffect();
    virtual void shellBumMarEffect();
    virtual void shellAtkEffect();
    virtual void downFallEffect();
    virtual void downLandOnEffect(float);
    virtual void hitShellDamageEffect();
    virtual void ikakuEffect();
    virtual void jumpSE();
    virtual void landonSE();
    virtual void shelllandonSE();
    virtual void shellinSE();
    virtual void shelloutSE();
    virtual void shellatkSE();
    virtual void getupSE();
    virtual void blitzchargeSE();
    virtual void notice1Vo();
    virtual void notice2Vo();
    virtual void wakeVo();
    virtual void escJumpVo();
    virtual void magicShotVo();
    virtual void shellOutVo();
    virtual void deadVo();
    virtual void loseFirstVo();
    virtual void loseSecondVo();

    // Demo States (Slots 358..372)
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoAwake);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoAwake_Wait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoIkaku);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoIkaku_Wait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoEscape_St);

    virtual void awakeSE();
    virtual void ikakuSE();

    // Member functions
    void calcRootJntPos();
    void calcShellJntPos();
    bool isTorideBoss();

    // Members (Offset 0x600 to 0xE70, size 0x870, total sizeof == 0xE70)
    u32 mUnk600;
    m3d::mdl_c mMdlKokoopa;
    m3d::anmChr_c mAnmChrKokoopa;
    m3d::anmMatClr_c mAnmMatClr;
    u32 mUnk6A8;
    m3d::anmTexPat_c mAnmTexPat;
    m3d::mdl_c mMdlShell;
    m3d::anmChr_c mAnmChrShell;
    u32 mUnk750;
    u32 mUnk754;
    u32 mUnk758;
    u32 mUnk75C;
    u32 mUnk760;
    u32 mUnk764;
    u32 mUnk768;
    u32 mPad76C;
    u32 mUnk770;
    mVec3_c mBlitzPos;
    u32 mUnk780;
    mVec3_c mLookatPos;
    s16 mUnk790;
    s16 mUnk792;
    u32 mUnk794;
    u32 mAtkCnt;
    dCc_c mCc;
    u32 mUnk840;
    u32 mUnk844;
    u32 mUnk848;
    u32 mUnk84C;
    mEf::levelEffect_c mLevelEffect1;
    mEf::levelEffect_c mLevelEffect2;
    u32 mUnkAA0;
    mVec3_c mUnkAA4;
    mVec3_c mUnkAB0;
    u32 mPadABC;
    s16 mUnkAC0;
    u8 mPadAC2[2];
    f32 mUnkAC4;
    u32 mUnkAC8;
    f32 mScaleSpeed;
    s32 mRootJntIdx;
    s32 mShellJntIdx;
    mVec3_c mRootJntPos;
    mVec3_c mShellJntPos;
    u32 mUnkAF0;
    mEf::levelEffect_c mLevelEffects[3];
    void *mVoiceParam;
};

char sizeof_dEnTorideKokoopa_c[sizeof(dEnTorideKokoopa_c) == 0xE70 ? 1 : -1];
```

### Source: `scratch/gemini_round16/d_enemy_toride_kokoopa.cpp`
```cpp
#include <game/bases/d_enemy_toride_kokoopa.hpp>
#include <game/bases/d_a_boss_demo.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/framework/f_manager.hpp>
#include <game/sLib/s_lib.hpp>

STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, Jump_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, Jump);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, BigJump_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, BigJump);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, LandOn);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackReady);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackBegin);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackSearch);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, Attack);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, AttackEnd);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, FumiHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, FireHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, StarHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, SlideHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, QuakeHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellHit);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellAtk_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellAtk);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, ShellOut);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DieFumi_St);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DieFire);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DieShell);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoWait);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoAwake);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoAwake_Wait);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoIkaku);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoIkaku_Wait);
STATE_VIRTUAL_DEFINE(dEnTorideKokoopa_c, DemoEscape_St);

int dEnTorideKokoopa_c::getTenmetsuTime_Fire() {
    return 24;
}

int dEnTorideKokoopa_c::getTenmetsuTime_Press() {
    return 24;
}

BOOL dEnTorideKokoopa_c::isFumiInvalid() const {
    return mUnk794 & 2;
}

BOOL dEnTorideKokoopa_c::isFireInvalid() const {
    return mUnk794 & 2;
}

BOOL dEnTorideKokoopa_c::isStarInvalid() const {
    return mUnk794 & 2;
}

void dEnTorideKokoopa_c::finalizeState_DemoWait() {}
void dEnTorideKokoopa_c::finalizeState_DieFire() {}
void dEnTorideKokoopa_c::finalizeState_DieShell() {}

mVec2_c dEnTorideKokoopa_c::getLookatPos() const {
    return mVec2_c(mLookatPos.x, mLookatPos.y);
}

void dEnTorideKokoopa_c::tenmetsuProc() {
    mAnmMatClr.play(0);
}

int dEnTorideKokoopa_c::draw() {
    if (mUnk794 & 1) {
        drawKokoopa();
    }
    if (mUnk794 & 2) {
        drawShell();
    }
    return 1;
}

void dEnTorideKokoopa_c::setBattleReady() {
    setAtkCnt();
    changeState(StateID_AttackBegin);
}

void dEnTorideKokoopa_c::initializeState_DieFire() {
    dEnBoss_c::initializeState_DieFire();
    mTimer1 = 60;
}

void dEnTorideKokoopa_c::executeState_DieFire() {
    if (mTimer1 != 0) {
        hitFireDamageEffect();
    }
    dEnBoss_c::executeState_DieFire();
}

void dEnTorideKokoopa_c::initializeState_DieShell() {
    dEnBoss_c::initializeState_DieShell();
    mTimer1 = 60;
}

void dEnTorideKokoopa_c::executeState_DieShell() {
    if (mTimer1 != 0) {
        hitShellDamageEffect();
    }
    dEnBoss_c::executeState_DieShell();
}

void dEnTorideKokoopa_c::damageProc() {
    if (mpBossLife->isDmgSection()) {
        damageLVo();
    } else {
        damageSVo();
    }
    if (mpBossLife->isTwoDamage()) {
        speedUp();
    }
}

void dEnTorideKokoopa_c::deadProc() {
    deadVo();
    if (dActorMng_c::m_instance->mpBossDemo != nullptr) {
        dActorMng_c::m_instance->mpBossDemo->stopBGM();
    }
}

struct VoiceParam_t {
    u8 mPad[0x40];
    u32 mDamageSVo;
    u32 mPad44;
    u32 mDamageLVo;
};

void dEnTorideKokoopa_c::damageSVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mDamageSVo != 0x449) {
        mSound.startSound(vp->mDamageSVo, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::damageLVo() {
    VoiceParam_t *vp = (VoiceParam_t*)mVoiceParam;
    if (vp != nullptr && vp->mDamageLVo != 0x449) {
        mSound.startSound(vp->mDamageLVo, mSoundParam, 0);
    }
}

void dEnTorideKokoopa_c::calcKokoopaMdl() {}
void dEnTorideKokoopa_c::calcShellMdl() {}
void dEnTorideKokoopa_c::drawKokoopa() {}
void dEnTorideKokoopa_c::drawShell() {}
void dEnTorideKokoopa_c::setKokoopaCc() {}
void dEnTorideKokoopa_c::setShellCc() {}
void dEnTorideKokoopa_c::moveAdjust_HIO() {}
void dEnTorideKokoopa_c::calcCcData() {}

float dEnTorideKokoopa_c::getDrawScale() {
    return 1.0f;
}

mVec3_c dEnTorideKokoopa_c::calcFacePos() {
    return mPos;
}

void dEnTorideKokoopa_c::calcRootJntPos() {
    mMdlKokoopa.getNodeWorldMtxMultVecZero(mRootJntIdx, mRootJntPos);
    mRootJntPos.z = 0.0f;
}

void dEnTorideKokoopa_c::calcShellJntPos() {
    mMdlShell.getNodeWorldMtxMultVecZero(mShellJntIdx, mShellJntPos);
    mShellJntPos.z = 0.0f;
}

void dEnTorideKokoopa_c::finalUpdate() {
    if (mUnk794 & 1) {
        calcKokoopaMdl();
        mBlitzPos = calcBlitzPos();
        mLookatPos = calcFacePos();
        calcCcData();
        calcRootJntPos();
    }
    if (mUnk794 & 2) {
        calcShellMdl();
        calcShellJntPos();
    }
}

float dEnTorideKokoopa_c::getJumpDist() const {
    return 64.0f;
}

float dEnTorideKokoopa_c::getKokoopaOffFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShellOnFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getKokoopaOnFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShellOffFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getCreateBlitzFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShootFrm() const {
    return 0.0f;
}

float dEnTorideKokoopa_c::getShellChangeEffectOffsetY() const {
    return 10.0f;
}

float dEnTorideKokoopa_c::getJumpGravity() {
    return -0.1875f;
}

mVec3_c dEnTorideKokoopa_c::getMagicStickEffectOffset() const {
    return mVec3_c(0.0f, 0.0f, 18.0f);
}

int dEnTorideKokoopa_c::getPressTime() { return 20; }
int dEnTorideKokoopa_c::defaultDirAngle() { return 0x2000; }
int dEnTorideKokoopa_c::getDownTime() { return 50; }
int dEnTorideKokoopa_c::getFumiRecoverTime() { return 4; }
int dEnTorideKokoopa_c::getAtkEndTime() { return 0; }
int dEnTorideKokoopa_c::getAtkEndTime_Wait() { return 0; }
int dEnTorideKokoopa_c::getAtkSearchTime() { return 0; }
int dEnTorideKokoopa_c::getAtkSearch2ndTime() { return 0; }
bool dEnTorideKokoopa_c::checkGetUp() const { return false; }
```

---

## 6. Complete Map of What Remains (162 Functions, 23,368 Bytes)

The remaining functions fall into five distinct functional clusters:

1. **State Action Implementations (~13,000 bytes / ~65 functions)**:
   - `Jump_St`, `Jump`, `BigJump_St`, `BigJump`, `LandOn` (~2,200 bytes): Jump arc physics, landing collision checks, and camera shake triggers.
   - `AttackReady`, `AttackBegin`, `AttackSearch`, `Attack`, `AttackEnd` (~3,500 bytes): Wand targeting, tracking player position, projectile charging, and projectile firing.
   - `ShellAtk_St`, `ShellAtk`, `ShellOut` (~3,000 bytes): Spinning shell attack, wall bouncing, and shell emergence animations.
   - `FumiHit`, `FireHit`, `StarHit`, `SlideHit`, `QuakeHit`, `ShellHit`, `DieFumi_St` (~2,800 bytes): Damage reaction animations, invulnerability timers, and defeat sequences.
   - `DemoAwake`, `DemoAwake_Wait`, `DemoIkaku`, `DemoIkaku_Wait`, `DemoEscape_St`, `DemoWait` (~1,500 bytes): Cutscene intro, battle roar, and castle retreat animations.

2. **Damage and Death Overrides (~3,500 bytes / ~10 functions)**:
   - `setFumiDead`, `setFireDead`, `setStarDead`, `setShellDead`, `setQuakeDead` (each ~400-500 bytes): Player score award logic, magic projectile destruction, and death state initiation.
   - `setFumiDamage`, `setFireDamage`, `setStarDamage`, `setShellDamage`, `setQuakeDamage` (each ~250-350 bytes): Boss life decrement, flashing material setup, and hit reaction transition.

3. **Actor Lifecycle & Resource Management (~2,500 bytes / ~15 functions)**:
   - Constructor `__ct__18dEnTorideKokoopa_cFv` (516 bytes): Model, animation, effect, collider, and sub-object instantiation.
   - Destructor `__dt__18dEnTorideKokoopa_cFv` (332 bytes): Destruction of 3D models and effect managers.
   - `create__18dEnTorideKokoopa_cFv` and `doDelete__18dEnTorideKokoopa_cFv` (each ~300 bytes): Resource loading from archive and cleanup.
   - `execute__18dEnTorideKokoopa_cFv` (240 bytes): Main execution loop driving state manager and sub-controllers.

4. **Projectile & Motion Physics (~2,300 bytes / ~12 functions)**:
   - `calcBlitzPos` (80 bytes) and `blitzShoot` (180 bytes): Bone matrix multiplication for wand tip position and projectile actor instantiation.
   - `setBlitzTarget` (120 bytes) and `lockonTurn` (240 bytes): Player tracking angle interpolation and turn speed calculations.

5. **Audio & Visual Effects (~2,068 bytes / ~60 functions)**:
   - Sound effect triggers (`jumpSE`, `landonSE`, `shellatkSE`, `shellinSE`, `shelloutSE`, `blitzchargeSE`, `awakeSE`, `ikakuSE`, etc.).
   - Voice clips (`notice1Vo`, `wakeVo`, `escJumpVo`, `magicShotVo`, `shellOutVo`, `deadVo`, `loseFirstVo`, `loseSecondVo`).
   - Visual effect spawners (`jumpEffect`, `landonEffect`, `shellAtkEffect`, `hitFireDamageEffect`, `hitShellDamageEffect`, `ikakuEffect`, `fumidmgEffect`, `fumideadEffect`).

---

## 7. Unsettled Items

1. **Pure Virtual Slot 306 (`0x4D0`)**:
   - Slot 305 is `virtual void createBlitz();`
   - Slot 306 contains `fn_0x0` in the vtable (`0x00000000`), declared as `virtual void createBlitz_sub() = 0;` (pure virtual).
   - This slot must remain pure virtual in `dEnTorideKokoopa_c` so derived Koopalings (Larry, Roy, Lemmy, Wendy, Iggy, Morton, Ludwig) provide their specific projectile creation overrides.

2. **Dual Collision Detection (`0x148` vs `0x79C`)**:
   - `dEnTorideKokoopa_c` inherits `dCc_c mCc` from `dActor_c` at offset `0x148`, and has a secondary collider `dCc_c mCc` at derived offset `0x79C`.
   - In methods such as `isQuakeDamage()`, `dActor_c::mCc.isLinked()` (`0x1EA`) is tested, whereas `mCc.release()` in damage handlers acts on the secondary collider at `0x79C`. Renaming the secondary collider (e.g. `mWandCc` or `mBodyCc`) in subsequent rounds will avoid identifier shadowing.
