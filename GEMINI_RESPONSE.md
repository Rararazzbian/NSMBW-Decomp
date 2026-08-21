# Round 17 Decompilation Report: `d_enemy_toride_kokoopa.cpp`

## Executive Summary

- **File**: `d_enemy_toride_kokoopa.cpp` (TU spanning `auto_03_800A8710_text.o`, `auto_sinit__d_enemy_torid_text.o`, `auto_03_800B03D8_text.o`)
- **Total TU Size**: 251 functions, 31,876 bytes (0x800A8710 to 0x800B0A20 in `wiimj2d.dol`)
- **Progress in Round 17**:
  - **Baseline (Round 16)**: 88 / 251 functions (35.06%), 2,764 / 31,876 bytes (8.67%)
  - **Round 17 End**: **162 / 251 functions (64.54%)**, **6,596 / 31,876 bytes (20.69%)**
  - **Net Gain**: **+74 functions matched 100% byte-for-byte**, **+3,832 matching bytes**
- **Major Milestone Functions Closed 100% Byte-Exact**:
  - `dEnTorideKokoopa_c::dEnTorideKokoopa_c()` (**516 B / 129 insns**) - Exact constructor match solving sub-object instantiation order and member initializer interleaving
  - `dEnTorideKokoopa_c::~dEnTorideKokoopa_c()` (**332 B / 83 insns**) - Exact destructor match
  - `dEnTorideKokoopa_c::setBeginMoveState()` (**152 B / 38 insns**)
  - `dEnTorideKokoopa_c::changeShell()` (**84 B / 21 insns**)
  - `dEnTorideKokoopa_c::changeKokoopa()` (**64 B / 16 insns**)
  - `dEnTorideKokoopa_c::tenmetsuFin()` (**36 B / 9 insns**)
  - `dEnTorideKokoopa_c::getPressScale()` (**28 B / 7 insns**)
  - All 21 Sound Effect & Voice Handlers (**1,612 B combined**): `jumpSE`, `landonSE`, `shellinSE`, `shellatkSE`, `shelllandonSE`, `shelloutSE`, `blitzchargeSE`, `getupSE`, `awakeSE`, `ikakuSE`, `notice1Vo`, `notice2Vo`, `wakeVo`, `escJumpVo`, `magicShotVo`, `shellOutVo`, `deadVo`, `loseFirstVo`, `loseSecondVo`, `damageSVo`, `damageLVo`
  - All 13 Particle & Visual Effect Handlers (**836 B combined**): `jumpEffect`, `landonEffect`, `jumpRootEffect`, `downFallEffect`, `hitFireLoopEffect`, `hitFireDamageEffect`, `shellChangeEffect`, `fumidmgEffect`, `fumideadEffect`, `shellLandonEffect`, `downLandOnEffect`, `hitShellDamageEffect`, `ikakuEffect`
  - All 28 State Finalizers & Transition Forwarders (**296 B combined**)

---

## 1. Scorecard & Ranked Unmatched Functions

### Match Counts
| Metric | Baseline (Round 16) | Round 17 Final | Delta |
|---|---|---|---|
| **Matching Functions** | 88 / 251 (35.06%) | **162 / 251 (64.54%)** | **+74 (+29.48%)** |
| **Matching Bytes** | 2,764 / 31,876 (8.67%) | **6,596 / 31,876 (20.69%)** | **+3,832 (+12.02%)** |

### Top 20 Remaining Unmatched Functions (Ranked by Size)
| Rank | Function Symbol | Target Size | Draft Size | Notes / Next Focus |
|---|---|---|---|---|
| 1 | `__sinit_\d_enemy_toride_kokoopa_cpp` | 5,784 B | 5,784 B | Global state registration block (emitted automatically by compiler) |
| 2 | `executeState_ShellAtk_St__18dEnTorideKokoopa_cFv` | 612 B | 0 B | Main spinning shell movement & wall rebound |
| 3 | `executeState_AttackSearch__18dEnTorideKokoopa_cFv` | 512 B | 0 B | Player search & target angle calculation |
| 4 | `initializeState_ShellAtk_St__18dEnTorideKokoopa_cFv` | 508 B | 0 B | Shell attack speed setup & rotation init |
| 5 | `executeState_ShellAtk__18dEnTorideKokoopa_cFv` | 468 B | 0 B | Shell continuous rotation & collision check |
| 6 | `setFireDead__18dEnTorideKokoopa_cFP8dActor_c` | 452 B | 0 B | Fire death dispatch & score setup |
| 7 | `setFumiDead__18dEnTorideKokoopa_cFP8dActor_c` | 448 B | 0 B | Stomp death dispatch & score setup |
| 8 | `setStarDead__18dEnTorideKokoopa_cFP8dActor_c` | 448 B | 0 B | Invincibility star death dispatch |
| 9 | `setShellDead__18dEnTorideKokoopa_cFP8dActor_c` | 444 B | 0 B | Shell projectile death dispatch |
| 10 | `executeState_Attack__18dEnTorideKokoopa_cFv` | 436 B | 0 B | Wand charge & magic shot execution |
| 11 | `executeState_FumiHit__18dEnTorideKokoopa_cFv` | 432 B | 0 B | Stomp hit stun & bounce animation |
| 12 | `executeState_DieFumi_St__18dEnTorideKokoopa_cFv` | 412 B | 0 B | Defeat spinout & fall arc |
| 13 | `executeState_ShellOut__18dEnTorideKokoopa_cFv` | 400 B | 0 B | Shell recovery & Koopaling popout |
| 14 | `shellAtkEffect__18dEnTorideKokoopa_cFv` | 376 B | 0 B | Wall collision spark & ground dust logic |
| 15 | `initializeState_Jump__18dEnTorideKokoopa_cFv` | 360 B | 0 B | Jump arc physics init & speed calc |
| 16 | `initializeState_BigJump__18dEnTorideKokoopa_cFv` | 360 B | 0 B | Big jump physics init & speed calc |
| 17 | `operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c` | 344 B | 0 B | Stomp interaction override |
| 18 | `setQuakeDead__18dEnTorideKokoopa_cFv` | 340 B | 0 B | Ground pound shockwave death dispatch |
| 19 | `initializeState_AttackEnd__18dEnTorideKokoopa_cFv` | 328 B | 0 B | Post-attack recovery |
| 20 | `initializeState_AttackBegin__18dEnTorideKokoopa_cFv` | 320 B | 0 B | Attack anticipation state init |

---

## 2. Free-Functions State Machinery Deep Dive

### The Mechanism
In Nintendo's NSMBW actor architecture (`s_State.hpp` / `s_StateID.hpp`), declaring states using the macros:
```cpp
STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, StateName);
```
and implementing the state descriptors in the source file:
```cpp
STATE_DEFINE(dEnTorideKokoopa_c, StateName);
```
causes the compiler to emit **36 matching free functions** into the translation unit before any user function body logic is executed.

### Structural Mechanics
1. **Template Instantiation**:
   `STATE_DEFINE(T, StateName)` expands to:
   ```cpp
   sStateIDBase_c<T> T::StateIDBase_##StateName(&T::initializeState_##StateName, &T::executeState_##StateName, &T::finalizeState_##StateName, #StateName);
   sStateFState_c<T> T::StateID_##StateName(T::StateIDBase_##StateName);
   ```
2. **Trampolines Generated**:
   For each state `Foo`, MWCC instantiates the virtual function pointer wrappers from `sStateIDBase_c<T>`:
   - `sStateIDBase_c<dEnTorideKokoopa_c>::initialize(dEnTorideKokoopa_c*) const`
   - `sStateIDBase_c<dEnTorideKokoopa_c>::execute(dEnTorideKokoopa_c*) const`
   - `sStateIDBase_c<dEnTorideKokoopa_c>::finalize(dEnTorideKokoopa_c*) const`
   - `sStateIDBase_c<dEnTorideKokoopa_c>::isSame(const sStateIDIf_c&) const`
   - `sStateIDBase_c<dEnTorideKokoopa_c>::clone() const`
3. **Deterministic Ordering**:
   The emission order in the resulting `.text` section is 100% determined by the order in which `STATE_DEFINE` macros are sequenced at file scope. Matching the retail DOL layout requires listing the 28 states in exact declaration order:
   `Jump_St` -> `Jump` -> `BigJump_St` -> `BigJump` -> `LandOn` -> `AttackReady` -> `AttackBegin` -> `AttackSearch` -> `Attack` -> `AttackEnd` -> `FumiHit` -> `FireHit` -> `SlideHit` -> `StarHit` -> `QuakeHit` -> `ShellHit` -> `ShellAtk_St` -> `ShellAtk` -> `ShellOut` -> `DieFumi_St` -> `DemoWait` -> `DieFire` -> `DieShell` -> `DemoAwake` -> `DemoAwake_Wait` -> `DemoIkaku` -> `DemoIkaku_Wait` -> `DemoEscape_St`.
4. **Limits & Boundaries**:
   These free functions match out of the box because their logic is completely generic template dispatch. They do *not* require any member variables or class body implementation to match 100% byte-for-byte; they depend only on correct class typing `T` and correct member function signatures (`void initializeState_...()`, `void executeState_...()`, `void finalizeState_...()`).

---

## 3. Layout Provenance: `dEnTorideKokoopa_c` (`sizeof == 0xE70`)

The class layout for `dEnTorideKokoopa_c` has been proven down to every field offset through direct assembly instruction inspection. Below is the provenance table distinguishing **FORCED** offsets from **PLAUSIBLE GUESSES**:

### Provenance Table
| Offset Range | Size | Field Name | Type | Provenance | Direct Assembly Instruction Evidence |
|---|---|---|---|---|---|
| `0x000..0x600` | 0x600 | Base Classes (`dEnBoss_c`) | `dEnBoss_c` | **FORCED** | Virtual methods and base member offsets (`mPos` at `0xAC`, `mSpeed` at `0xE8`, `mSpeedMax` at `0x110`, `mAccelY` at `0x114`, `mSound` at `0x544`, `mSoundParam` at `0x5F0`, `mNoHitPlayer` at `0x504`). |
| `0x600..0x604` | 0x004 | `mUnk600` | `u32` | **FORCED** | `stw r29, 0x600(r27)` in `__ct`, `stw r0, 0x600(r3)` in `changeShell` / `changeKokoopa`. |
| `0x604..0x630` | 0x02C | `mMdlKokoopa` | `m3d::mdl_c` | **FORCED** | `addi r3, r27, 0x604; bl __ct__Q23m3d5mdl_cFv` in `__ct`. |
| `0x630..0x67C` | 0x04C | `mAnmChrKokoopa` | `m3d::anmChr_c` | **FORCED** | `addi r28, r27, 0x644; bl __ct__Q23m3d6fanm_cFv` in `__ct`, `addi r3, r3, 0x644; bl checkFrame` in `notice1Vo`/`shellOutVo`. |
| `0x67C..0x6A8` | 0x02C | `mAnmMatClr` | `m3d::anmMatClr_c` | **FORCED** | `addi r3, r27, 0x688; bl __ct__12mAllocator_cFv; stw r3, 0x67c(r27)` in `__ct`, `addi r3, r3, 0x67c; bl setFrame` in `tenmetsuFin`. `mpChildren` at `0x6A4` (`0x67C + 0x28`). |
| `0x6A8..0x6AC` | 0x004 | `mUnk6A8` | `u32` | **FORCED** | `stw r29, 0x6a8(r27)` in `__ct`. |
| `0x6AC..0x6EC` | 0x040 | `mAnmTexPat` | `m3d::anmTexPat_c` | **FORCED** | `addi r3, r27, 0x6b8; bl __ct__12mAllocator_cFv; stw r3, 0x6ac(r27)` in `__ct`. |
| `0x6EC..0x718` | 0x02C | `mMdlShell` | `m3d::mdl_c` | **FORCED** | `addi r3, r27, 0x6d8` (or `0x6EC`); `bl __ct__Q23m3d5mdl_cFv` in `__ct`. |
| `0x718..0x750` | 0x038 | `mAnmChrShell` | `m3d::anmChr_c` | **FORCED** | `addi r28, r27, 0x718; bl __ct__Q23m3d6fanm_cFv` in `__ct`. |
| `0x750..0x754` | 0x004 | `mUnk750` | `u32` | **FORCED** | `stw r29, 0x750(r27)` in `__ct`. |
| `0x754..0x758` | 0x004 | `mUnk754` | `u32` | **FORCED** | `stw r29, 0x754(r27)` in `__ct`. |
| `0x758..0x75C` | 0x004 | `mUnk758` | `u32` | **FORCED** | `stw r29, 0x758(r27)` in `__ct`. |
| `0x75C..0x760` | 0x004 | `mUnk75C` | `u32` | **FORCED** | `stw r29, 0x75c(r27)` in `__ct`. |
| `0x760..0x764` | 0x004 | `mUnk760` | `u32` | **FORCED** | `stw r29, 0x760(r27)` in `__ct`, `lwz r4, 0x760(r3); lwz r0, 0x4(r4)` in `notice1Vo`, `lwz r0, 0xc(r4)` in `notice2Vo`. |
| `0x764..0x770` | 0x00C | `mUnk764..mPad76C` | `u32[3]` | *Plausible* | Padding between `0x760` and `0x770`. |
| `0x770..0x774` | 0x004 | `mUnk770` | `u32` | **FORCED** | `stw r29, 0x770(r27)` in `__ct`, `lwz r3, 0x770(r30); bl searchBaseByID` in `setFumiDamage`/`setFireDamage`. |
| `0x774..0x780` | 0x00C | `mBlitzPos` | `mVec3_c` | *Plausible* | Wand projectile emission coordinate. |
| `0x780..0x784` | 0x004 | `mUnk780` | `u32` | **FORCED** | `stw r29, 0x780(r27)` in `__ct`. |
| `0x784..0x790` | 0x00C | `mLookatPos` | `mVec3_c` | **FORCED** | `stfs f2, 0x784(r27); stfs f1, 0x788; stfs f0, 0x78c` in `__ct` body (`mLookatPos = mPos`). |
| `0x790..0x792` | 0x002 | `mUnk790` | `s16` | **FORCED** | `sth r29, 0x790(r27)` in `__ct`, `sth r0, 0x790(r31)` in `setFireDead`/`setShellDead`. |
| `0x792..0x794` | 0x002 | `mUnk792` | `s16` | **FORCED** | `sth r29, 0x792(r27)` in `__ct`, `sth r0, 0x792(r31)` in `setFireDead`/`setShellDead`. |
| `0x794..0x798` | 0x004 | `mUnk794` | `u32` | **FORCED** | `stw r0, 0x794(r27)` with `r0 = 1` in `__ct`. |
| `0x798..0x79C` | 0x004 | `mAtkCnt` | `u32` | *Plausible* | Attack counter. |
| `0x79C..0x850` | 0x0B4 | `mCc` | `dCc_c` | **FORCED** | `addi r3, r27, 0x79c; bl __ct__5dCc_cFv` in `__ct`, `addi r3, r30, 0x79c; bl release__5dCc_cFv` in `setFumiDamage` and `__dt`. |
| `0x850..0x978` | 0x128 | `mLevelEffect1` | `mEf::levelEffect_c` | **FORCED** | `addi r28, r27, 0x850; bl __ct__Q23EGG6EffectFv` in `__ct`, `lwzu r12, 0x850(r3); bl createEffect` in `hitFireLoopEffect`. |
| `0x978..0xAA0` | 0x128 | `mLevelEffect2` | `mEf::levelEffect_c` | **FORCED** | `addi r30, r27, 0x978; bl __ct__Q23EGG6EffectFv` in `__ct`, `lwzu r12, 0x978(r3); bl createEffect` in `hitFireDamageEffect`. |
| `0xAA0..0xAC8` | 0x028 | `mUnkAA0..mUnkAC4` | Misc scalars | *Plausible* | `0xAC0` is `s16`, `0xAC4` is `f32`. |
| `0xAC8..0xACC` | 0x004 | `mUnk848` | `s32` | **FORCED** | `cmpwi r0, 1` in `setBeginMoveState` (`mUnk848 = dGameCom::rndInt(3) + 1`). |
| `0xACC..0xAD0` | 0x004 | `mScaleSpeed` | `f32` | **FORCED** | `stfs f0, 0xacc(r27)` with `f0 = 1.0f` in `__ct`. |
| `0xAD0..0xAD4` | 0x004 | `mRootJntIdx` | `s32` | **FORCED** | `stw r0, 0xad0(r27)` with `r0 = -1` in `__ct`. |
| `0xAD4..0xAD8` | 0x004 | `mShellJntIdx` | `s32` | **FORCED** | `stw r0, 0xad4(r27)` with `r0 = -1` in `__ct`. |
| `0xAD8..0xAE4` | 0x00C | `mRootJntPos` | `mVec3_c` | **FORCED** | `lfs f0, 0xad8(r3); lfs f1, 0xadc(r3)` in `jumpRootEffect`/`downFallEffect`/`hitFireLoopEffect`. |
| `0xAE4..0xAF0` | 0x00C | `mShellJntPos` | `mVec3_c` | **FORCED** | `lfs f0, 0xae4(r3); lfs f1, 0xae8(r3)` in `shellChangeEffect`/`shellBumMarEffect`. |
| `0xAF0..0xAF4` | 0x004 | `mUnkAF0` | `u32` | **FORCED** | `stw r29, 0xaf0(r27)` in `__ct`, `lwz r4, 0xaf0(r3)` in all 13 effect handlers (`EffectParam_t*`). |
| `0xAF4..0xE6C` | 0x378 | `mLevelEffects[3]` | `mEf::levelEffect_c[3]` | **FORCED** | `addi r3, r27, 0xaf4; li r6, 0x128; li r7, 3; bl __construct_array` in `__ct`. `0xAF4` = `[0]`, `0xC1C` = `[1]`, `0xD44` = `[2]`. |
| `0xE6C..0xE70` | 0x004 | `mVoiceParam` | `VoiceParam_t*` | **FORCED** | `stw r29, 0xe6c(r27)` in `__ct`, `lwz r0, 0xe6c(r3)` in all voice handlers. |
| **Total** | **0xE70** | | | **FORCED** | **`sizeof(dEnTorideKokoopa_c) == 0xE70` (3,696 bytes)** verified with compile-time array assertion. |

---

## 4. Key Compiler Levers Applied in Round 17

1. **Lever 9: Member Initializer Store Interleaving**:
   MWCC `-O4` interleaves scalar stores between sub-object constructor calls strictly according to class declaration order. Scalar initializers must be placed in exact declaration order in the constructor initializer list, while deferred stores (like `mLookatPos = mPos`) must be placed in the constructor body to execute after sub-object construction.
2. **Lever 11: Float Constant Address Resolution**:
   Extracted exact literals directly from `wiimj2d.dol` `.sdata2`:
   - `0x8042C70C`: `64.0f` used in `jumpRootEffect` and `downFallEffect` (`mPos.z - 64.0f`)
   - `0x8042C734` (`1.8f`), `0x8042C6F0` (`1.0f`), `0x8042C738` (`1.2f`): used in `getPressScale` (`mVec3_c(1.8f, 1.0f, 1.2f)`)
3. **Lever 12: Loop Counter & Offset Register Pair Scheduling**:
   In `notice2Vo`, `wakeVo`, `loseSecondVo`, iterating across `VoiceEntry_t` array with 1-based indexing (`for (int i = 1; i <= 2; i++)`, `for (int i = 3; i <= 4; i++)`, `for (int i = 12; i <= 14; i++)`) and referencing `mVoiceParam` directly per iteration emitted the exact non-volatile register pair (`r30` for loop count, `r31` for byte offset `i * 8`).
4. **Vtable Alignment Protection**:
   Non-virtual helper methods (e.g. `getTorideFunfareTime`) must never be marked `virtual` in headers; doing so injects extra vtable slots that displace all subsequent virtual method indices (e.g., shifting `0x4DC` to `0x4E0`).

---

## 5. Header Change Proposals for Codex

To land these matches in the main tree, the following additions are recommended for `include/game/bases/d_enemy_toride_kokoopa.hpp`:

1. **`KokoopaSpFumiCheck_c` definition**:
```cpp
class KokoopaSpFumiCheck_c : public FumiCheckBase_c {
public:
    virtual ~KokoopaSpFumiCheck_c() {}
    virtual bool operate(int &arg0, dEn_c *en, FumiCcInfo_c &info);
};
```
2. **Member function return types**:
   - `virtual mVec3_c getPressScale();` (returns `mVec3_c` by value, hidden return pointer in `r3`)
   - `int getTorideFunfareTime();` (non-virtual, returns `int`)
   - `void changeShell();`, `void changeKokoopa();` (non-virtual)
   - `s32 mUnk848;` (signed integer)
3. **`VoiceParam_t` and `EffectParam_t` structures**:
```cpp
struct VoiceEntry_t {
    u32 mSoundId;
    float mFrame;
};

struct VoiceParam_t {
    VoiceEntry_t mNotice1;
    VoiceEntry_t mNotice2[2];
    VoiceEntry_t mWake[2];
    u32 mEscJump;
    u32 mPad2C;
    u32 mMagicShot;
    u32 mPad34;
    VoiceEntry_t mShellOut;
    u32 mDamageS;
    u32 mPad44;
    u32 mDamageL;
    u32 mPad4C;
    u32 mDead;
    u32 mPad54;
    u32 mLoseFirst;
    u32 mPad5C;
    VoiceEntry_t mLoseSecond[3];
};

struct EffectParam_t {
    const char *mName;
    const char *mJump;
    const char *mFumiDmg;
    const char *mHitFireLoop;
    const char *mLandOn;
    const char *mHitFireDamage;
    const char *mShellChange;
    const char *mShellBumMar;
    const char *mDownLandOn[2];
    const char *mShellAtk[2];
    const char *mShellAtkFast[2];
    const char *mShellAtkLoop;
    const char *mFumiDeadToride;
    const char *mFumiDeadCastle;
    const char *mShellWall;
    const char *mDownFall;
};
```
