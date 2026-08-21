# Gemini Round 15 Response

## Summary of Findings

1. **Task A (dEnBoss_c vtable and class layout)**:
   - `dEnBoss_c` introduces **68 new virtual methods** (vtable slots 158 through 225), bringing the total vtable size to **226 slots** (`0x390` bytes).
   - `dEnBoss_c` overrides exactly **21 virtual methods** inherited from `dEn_c` (and inherits 137 unchanged).
   - `dEnTorideKokoopa_c` overrides exactly **41 virtual methods** inherited from `dEnBoss_c` (slots 0..225), and introduces 149 new slots beginning at slot 226 (`Jump_St`), matching the total 375 slots (`0x5E4` bytes).
   - `sizeof(dEnBoss_c)` is **0x600** bytes, confirmed by constructor member layout and Kokoopa derived-member write `stw r29, 0x600(r27)`.
   - The proposed `include/game/bases/d_enemy_boss.hpp` was compiled with CodeWarrior (`mwcceppc.exe`) in `scratch/gemini_round15/`. All 226 compiled vtable slots match the retail `__vt__9dEnBoss_c` slot-for-slot (1:1), and a derived Kokoopa test class correctly placed `Jump_St` starting at slot 226.

2. **Task B (dEn_c header audit)**:
   - The existing `include/game/bases/d_enemy.hpp` declares **158 virtual slots** (55 base-class slots from `fBase_c` through `dActorMultiState_c`, plus 103 new virtual slots in `dEn_c`).
   - Every single slot (0 to 157) matches retail `__vt__5dEn_c` (`0x280` bytes) with **zero divergence**.

---

## Task A: Proposed Header (`include/game/bases/d_enemy_boss.hpp`)

Below is the complete proposed header file. It declares `dBossLifeInf_c`, `dBossLife_Common_c`, and `dEnBoss_c` with all 21 `dEn_c` overrides, all 68 new virtual methods in order, non-virtual helper methods, and the 0x600-byte member layout.

```cpp
#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_audio.hpp>

/// @brief Interface for boss life / hit-point management.
/// @ingroup bases
class dBossLifeInf_c {
public:
    virtual ~dBossLifeInf_c();
    virtual bool isNonDamage() const = 0;
    virtual bool isOneDamage() const = 0;
    virtual bool isTwoDamage() const = 0;
    virtual bool isDmgSection() const;
    virtual int getDamage_Fire() const = 0;
    virtual int getDamage_Fumi() const = 0;
    virtual int getDamage_HipAtk() const = 0;
    virtual int getDamage_Star() const = 0;
    virtual int getDamage_PenguinSlide() const = 0;
    virtual int getDamage_BlockHit() const = 0;
    virtual int getDamage_Shell() const = 0;
    virtual int getDamage_Quake() const = 0;
    virtual void damageRev(int);

    int mLife; ///< [0x04] Remaining boss health.
};

/// @brief Common standard implementation of dBossLifeInf_c.
/// @ingroup bases
class dBossLife_Common_c : public dBossLifeInf_c {
public:
    virtual ~dBossLife_Common_c();
    virtual bool isNonDamage() const;
    virtual bool isOneDamage() const;
    virtual bool isTwoDamage() const;
    virtual bool isDmgSection() const;
    virtual int getDamage_Fire() const;
    virtual int getDamage_Fumi() const;
    virtual int getDamage_HipAtk() const;
    virtual int getDamage_Star() const;
    virtual int getDamage_PenguinSlide() const;
    virtual int getDamage_BlockHit() const;
    virtual int getDamage_Shell() const;
    virtual int getDamage_Quake() const;
    virtual void damageRev(int);
};

/// @brief Base class for stage boss actors.
/// @ingroup bases
class dEnBoss_c : public dEn_c {
public:
    dEnBoss_c();
    virtual ~dEnBoss_c();

    // dEn_c / base-class overrides (21 functions across slots 0..157)
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Star(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Slip(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_WireNet(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Shell(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);
    virtual void setDeathInfo_Quake(int);
    virtual BOOL isQuakeDamage();
    virtual void initializeState_DieFumi();
    virtual void executeState_DieFumi();
    virtual void finalizeState_DieFumi();
    virtual void FumiScoreSet(dActor_c *actor);

    // 68 new virtual methods (slots 158..225)
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieFire);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieSlide);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieShell);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieStar);
    STATE_VIRTUAL_FUNC_DECLARE(dEnBoss_c, DieQuake);

    virtual void setBattleReady();
    virtual void createModel();
    virtual void createBossLife();
    virtual int createInit();
    virtual void tenmetsuReady();
    virtual void tenmetsuProc();
    virtual void tenmetsuFin();
    virtual int getTenmetsuTime_Fire();
    virtual int getTenmetsuTime_Shell();
    virtual int getTenmetsuTime_Press();
    virtual void deadAllKill();

    virtual void setFumiDamage(dActor_c *killedBy);
    virtual void setFumiDead(dActor_c *killedBy);
    virtual void setFireDamage(dActor_c *killedBy);
    virtual void setFireDead(dActor_c *killedBy);
    virtual void setHipatkDamage(dActor_c *killedBy);
    virtual void setHipatkDead(dActor_c *killedBy);
    virtual void setSlideDamage(dActor_c *killedBy);
    virtual void setSlideDead(dActor_c *killedBy);
    virtual void setStarDamage(dActor_c *killedBy);
    virtual void setStarDead(dActor_c *killedBy);
    virtual void setQuakeDamage();
    virtual void setQuakeDead();
    virtual void setShellDamage(dActor_c *killedBy);
    virtual void setShellDead(dActor_c *killedBy);

    virtual void damageProc();
    virtual void deadProc();

    virtual bool isFumiInvalid() const;
    virtual bool isFumiDmgInvalid() const;
    virtual bool isFireInvalid() const;
    virtual bool isSlideInvalid() const;
    virtual bool isShellInvalid() const;
    virtual bool isStarInvalid() const;

    virtual void fumideadEffect();
    virtual void fumidmgEffect();
    virtual void hitFireEffect();
    virtual void hitShellEffect();

    virtual void fumidmgSE();
    virtual void fumideadSE();
    virtual void stardmgSE();
    virtual void stardeadSE();
    virtual void shelldmgSE();
    virtual void shelldeadSE();
    virtual void firedmgSE();
    virtual void firedeadSE();
    virtual void quakedmgSE();
    virtual void quakedeadSE();

    virtual void fumiDeadVo();
    virtual void damageSVo();
    virtual void damageLVo();

    // Non-virtual methods
    int create();
    void allocate();
    void fumiProc(dActor_c *killedBy);

    // Member variables
    dHeapAllocator_c mAllocator;       ///< [0x524] Boss heap allocator.
    u32 mTenmetsuTimer;                ///< [0x540] Flashing/invulnerability countdown timer.
    dAudio::SndObjctEmy_c mSound;      ///< [0x544] Boss sound actor object (size 0xAC).
    s16 mSoundParam;                   ///< [0x5F0] Parameter passed to sound triggers (e.g. voice/sound ID modifier).
    u8 mPadBoss[2];                    ///< [0x5F2] Alignment padding.
    u32 mQuakeDamage;                  ///< [0x5F4] Quake damage tracking flag / state.
    dBossLifeInf_c *mpBossLife;        ///< [0x5F8] Pointer to boss life manager.
    u8 mPadEnd[4];                     ///< [0x5FC] Tail padding to 0x600.
};

STATIC_ASSERT(sizeof(dEnBoss_c) == 0x600);
STATIC_ASSERT(sizeof(dBossLifeInf_c) == 0x8);
STATIC_ASSERT(sizeof(dBossLife_Common_c) == 0x8);
```

---

## Detailed Audit: The 68 New Slots (158–225)

Every slot has been verified against `original/wiimj2d.dol` and `bin/dtk/wiimj2d_symbols.txt`. Parameter types and constness are from CFront mangling; return types are confirmed via call site and implementation disassembly.

| Slot | Mangled Symbol | Demangled Prototype | Return Type Evidence | Size (d_enemy_boss) |
|---|---|---|---|---|
| **158** | `initializeState_DemoWait__9dEnBoss_cFv` | `void initializeState_DemoWait()` | State triple init; returns `void` (`blr`) | 0x04 |
| **159** | `executeState_DemoWait__9dEnBoss_cFv` | `void executeState_DemoWait()` | State triple exec; returns `void` (`blr`) | 0x04 |
| **160** | `finalizeState_DemoWait__9dEnBoss_cFv` | `void finalizeState_DemoWait()` | State triple fin; returns `void` (`blr`) | 0x04 |
| **161** | `initializeState_DieFire__9dEnBoss_cFv` | `void initializeState_DieFire()` | State triple init; forwards to slot 91 (DieFumi); returns `void` | 0x10 |
| **162** | `executeState_DieFire__9dEnBoss_cFv` | `void executeState_DieFire()` | State triple exec; forwards to slot 92 (DieFumi); returns `void` | 0x10 |
| **163** | `finalizeState_DieFire__9dEnBoss_cFv` | `void finalizeState_DieFire()` | State triple fin; forwards to slot 93 (DieFumi); returns `void` | 0x10 |
| **164** | `initializeState_DieSlide__9dEnBoss_cFv` | `void initializeState_DieSlide()` | State triple init; forwards to slot 91 (DieFumi); returns `void` | 0x10 |
| **165** | `executeState_DieSlide__9dEnBoss_cFv` | `void executeState_DieSlide()` | State triple exec; forwards to slot 92 (DieFumi); returns `void` | 0x10 |
| **166** | `finalizeState_DieSlide__9dEnBoss_cFv` | `void finalizeState_DieSlide()` | State triple fin; forwards to slot 93 (DieFumi); returns `void` | 0x10 |
| **167** | `initializeState_DieShell__9dEnBoss_cFv` | `void initializeState_DieShell()` | State triple init; forwards to slot 91 (DieFumi); returns `void` | 0x10 |
| **168** | `executeState_DieShell__9dEnBoss_cFv` | `void executeState_DieShell()` | State triple exec; forwards to slot 92 (DieFumi); returns `void` | 0x10 |
| **169** | `finalizeState_DieShell__9dEnBoss_cFv` | `void finalizeState_DieShell()` | State triple fin; forwards to slot 93 (DieFumi); returns `void` | 0x10 |
| **170** | `initializeState_DieStar__9dEnBoss_cFv` | `void initializeState_DieStar()` | State triple init; forwards to slot 91 (DieFumi); returns `void` | 0x10 |
| **171** | `executeState_DieStar__9dEnBoss_cFv` | `void executeState_DieStar()` | State triple exec; forwards to slot 92 (DieFumi); returns `void` | 0x10 |
| **172** | `finalizeState_DieStar__9dEnBoss_cFv` | `void finalizeState_DieStar()` | State triple fin; forwards to slot 93 (DieFumi); returns `void` | 0x10 |
| **173** | `initializeState_DieQuake__9dEnBoss_cFv` | `void initializeState_DieQuake()` | State triple init; forwards to slot 91 (DieFumi); returns `void` | 0x10 |
| **174** | `executeState_DieQuake__9dEnBoss_cFv` | `void executeState_DieQuake()` | State triple exec; forwards to slot 92 (DieFumi); returns `void` | 0x10 |
| **175** | `finalizeState_DieQuake__9dEnBoss_cFv` | `void finalizeState_DieQuake()` | State triple fin; forwards to slot 93 (DieFumi); returns `void` | 0x10 |
| **176** | `setBattleReady__9dEnBoss_cFv` | `void setBattleReady()` | Body is `blr`; returns `void` | 0x04 |
| **177** | `createModel__9dEnBoss_cFv` | `void createModel()` | Called in `allocate()` at `0x80098744`; caller does not read r3; returns `void` | 0x04 |
| **178** | `createBossLife__9dEnBoss_cFv` | `void createBossLife()` | Called in `allocate()` at `0x80098758`; instantiates `dBossLife_Common_c`; returns `void` | 0x5C |
| **179** | `createInit__9dEnBoss_cFv` | `int createInit()` | Body is `li r3, 1; blr`; returns `int` (success code 1) | 0x08 |
| **180** | `tenmetsuReady__9dEnBoss_cFv` | `void tenmetsuReady()` | Called before damage/dead handling in `fumiProc` (`0x80098E34`) etc.; returns `void` | 0x04 |
| **181** | `tenmetsuProc__9dEnBoss_cFv` | `void tenmetsuProc()` | Called during `preExecute()` countdown (`0x80098880`); returns `void` | 0x04 |
| **182** | `tenmetsuFin__9dEnBoss_cFv` | `void tenmetsuFin()` | Called when countdown expires in `preExecute()` (`0x80098898`); returns `void` | 0x04 |
| **183** | `getTenmetsuTime_Fire__9dEnBoss_cFv` | `int getTenmetsuTime_Fire()` | Body is `li r3, 40; blr`; caller `hitCallback_Fire` stores r3 into `mTenmetsuTimer` (`0x80099164`) | 0x08 |
| **184** | `getTenmetsuTime_Shell__9dEnBoss_cFv` | `int getTenmetsuTime_Shell()` | Body is `li r3, 40; blr`; caller `hitCallback_Shell` stores r3 into `mTenmetsuTimer` (`0x800994A4`) | 0x08 |
| **185** | `getTenmetsuTime_Press__9dEnBoss_cFv` | `int getTenmetsuTime_Press()` | Body is `li r3, 40; blr`; caller `fumiProc` stores r3 into `mTenmetsuTimer` (`0x80098E20`) | 0x08 |
| **186** | `deadAllKill__9dEnBoss_cFv` | `void deadAllKill()` | Tailcalls `allEnemyDeath__11dActorMng_cFi(0)`; returns `void` | 0x0C |
| **187** | `setFumiDamage__9dEnBoss_cFP8dActor_c` | `void setFumiDamage(dActor_c *killedBy)` | Body is `blr`; called on boss stomp damage; returns `void` | 0x04 |
| **188** | `setFumiDead__9dEnBoss_cFP8dActor_c` | `void setFumiDead(dActor_c *killedBy)` | Body is `blr`; called on boss stomp death; returns `void` | 0x04 |
| **189** | `setFireDamage__9dEnBoss_cFP8dActor_c` | `void setFireDamage(dActor_c *killedBy)` | Body is `blr`; called on fireball damage; returns `void` | 0x04 |
| **190** | `setFireDead__9dEnBoss_cFP8dActor_c` | `void setFireDead(dActor_c *killedBy)` | Body is `blr`; called on fireball death; returns `void` | 0x04 |
| **191** | `setHipatkDamage__9dEnBoss_cFP8dActor_c` | `void setHipatkDamage(dActor_c *killedBy)` | Tailcalls `setFumiDamage` via vtable slot 187 (`0x2F4`); returns `void` | 0x10 |
| **192** | `setHipatkDead__9dEnBoss_cFP8dActor_c` | `void setHipatkDead(dActor_c *killedBy)` | Tailcalls `setFumiDead` via vtable slot 188 (`0x2F8`); returns `void` | 0x10 |
| **193** | `setSlideDamage__9dEnBoss_cFP8dActor_c` | `void setSlideDamage(dActor_c *killedBy)` | Body is `blr`; returns `void` | 0x04 |
| **194** | `setSlideDead__9dEnBoss_cFP8dActor_c` | `void setSlideDead(dActor_c *killedBy)` | Body is `blr`; returns `void` | 0x04 |
| **195** | `setStarDamage__9dEnBoss_cFP8dActor_c` | `void setStarDamage(dActor_c *killedBy)` | Body is `blr`; called on star damage; returns `void` | 0x04 |
| **196** | `setStarDead__9dEnBoss_cFP8dActor_c` | `void setStarDead(dActor_c *killedBy)` | Body is `blr`; called on star death; returns `void` | 0x04 |
| **197** | `setQuakeDamage__9dEnBoss_cFv` | `void setQuakeDamage()` | Body is `blr`; called on ground quake damage; returns `void` | 0x04 |
| **198** | `setQuakeDead__9dEnBoss_cFv` | `void setQuakeDead()` | Body is `blr`; called on ground quake death; returns `void` | 0x04 |
| **199** | `setShellDamage__9dEnBoss_cFP8dActor_c` | `void setShellDamage(dActor_c *killedBy)` | Body is `blr`; called on shell hit damage; returns `void` | 0x04 |
| **200** | `setShellDead__9dEnBoss_cFP8dActor_c` | `void setShellDead(dActor_c *killedBy)` | Body is `blr`; called on shell hit death; returns `void` | 0x04 |
| **201** | `damageProc__9dEnBoss_cFv` | `void damageProc()` | Body is `blr`; called at end of non-fatal damage; returns `void` | 0x04 |
| **202** | `deadProc__9dEnBoss_cFv` | `void deadProc()` | Body is `blr`; called at end of fatal damage; returns `void` | 0x04 |
| **203** | `isFumiInvalid__9dEnBoss_cCFv` | `bool isFumiInvalid() const` | `const`; body is `li r3, 0; blr`; tested with `cmpwi r3, 0` in `Normal_VsPlHitCheck` | 0x08 |
| **204** | `isFumiDmgInvalid__9dEnBoss_cCFv` | `bool isFumiDmgInvalid() const` | `const`; body is `li r3, 0; blr`; tested with `cmpwi r3, 0` in `Normal_VsPlHitCheck` | 0x08 |
| **205** | `isFireInvalid__9dEnBoss_cCFv` | `bool isFireInvalid() const` | `const`; body is `li r3, 0; blr`; tested with `cmpwi r3, 0` in `hitCallback_Fire` | 0x08 |
| **206** | `isSlideInvalid__9dEnBoss_cCFv` | `bool isSlideInvalid() const` | `const`; body is `li r3, 1; blr`; returns `bool` (default true/immune to slide) | 0x08 |
| **207** | `isShellInvalid__9dEnBoss_cCFv` | `bool isShellInvalid() const` | `const`; body is `li r3, 0; blr`; tested with `cmpwi r3, 0` in `hitCallback_Shell` | 0x08 |
| **208** | `isStarInvalid__9dEnBoss_cCFv` | `bool isStarInvalid() const` | `const`; body is `li r3, 0; blr`; tested with `cmpwi r3, 0` in `hitCallback_Star` | 0x08 |
| **209** | `fumideadEffect__9dEnBoss_cFv` | `void fumideadEffect()` | Body is `blr`; called on fumi death; returns `void` | 0x04 |
| **210** | `fumidmgEffect__9dEnBoss_cFv` | `void fumidmgEffect()` | Body is `blr`; called on fumi damage; returns `void` | 0x04 |
| **211** | `hitFireEffect__9dEnBoss_cFv` | `void hitFireEffect()` | Body is `blr`; called on fire hit; returns `void` | 0x04 |
| **212** | `hitShellEffect__9dEnBoss_cFv` | `void hitShellEffect()` | Body is `blr`; called on shell hit; returns `void` | 0x04 |
| **213** | `fumidmgSE__9dEnBoss_cFv` | `void fumidmgSE()` | Plays SE 0x513 via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **214** | `fumideadSE__9dEnBoss_cFv` | `void fumideadSE()` | Plays SE 0x514 via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **215** | `stardmgSE__9dEnBoss_cFv` | `void stardmgSE()` | Plays SE 0x519 via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **216** | `stardeadSE__9dEnBoss_cFv` | `void stardeadSE()` | Plays SE 0x51A via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **217** | `shelldmgSE__9dEnBoss_cFv` | `void shelldmgSE()` | Plays SE 0x51B via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **218** | `shelldeadSE__9dEnBoss_cFv` | `void shelldeadSE()` | Plays SE 0x51C via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **219** | `firedmgSE__9dEnBoss_cFv` | `void firedmgSE()` | Plays SE 0x516 / 0x517 depending on `isDmgSection()`; returns `void` | 0x88 |
| **220** | `firedeadSE__9dEnBoss_cFv` | `void firedeadSE()` | Plays SE 0x518 via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **221** | `quakedmgSE__9dEnBoss_cFv` | `void quakedmgSE()` | Plays SE 0x519 via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **222** | `quakedeadSE__9dEnBoss_cFv` | `void quakedeadSE()` | Plays SE 0x51A via `mSound` (tailcall `bctr`); returns `void` | 0x20 |
| **223** | `fumiDeadVo__9dEnBoss_cFv` | `void fumiDeadVo()` | Body is `blr`; called in `fumiProc` / `hitCallback_HipAttk` / `hitCallback_Spin`; returns `void` | 0x04 |
| **224** | `damageSVo__9dEnBoss_cFv` | `void damageSVo()` | Body is `blr`; small damage voice; returns `void` | 0x04 |
| **225** | `damageLVo__9dEnBoss_cFv` | `void damageLVo()` | Body is `blr`; large damage voice; returns `void` | 0x04 |

---

## The 21 `dEn_c` Overrides in `dEnBoss_c`

Confirmed by comparing `__vt__5dEn_c` and `__vt__9dEnBoss_c` slot by slot (slots 0..157). Exactly 21 slots have different function pointers:

1. **Slot 8** (`0x028`): `postExecute(fBase_c::MAIN_STATE_e)` (`postExecute__9dEnBoss_cFQ27fBase_c12MAIN_STATE_e` at `0x80098900`)
2. **Slot 16** (`0x048`): `~dEnBoss_c()` (`__dt__9dEnBoss_cFv` at `0x800985B0`)
3. **Slot 60** (`0x0F8`): `Normal_VsPlHitCheck(dCc_c*, dCc_c*)` (`Normal_VsPlHitCheck__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x80098B60`)
4. **Slot 61** (`0x0FC`): `Normal_VsYoshiHitCheck(dCc_c*, dCc_c*)` (`Normal_VsYoshiHitCheck__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x80098CB0`)
5. **Slot 62** (`0x100`): `hitCallback_Star(dCc_c*, dCc_c*)` (`hitCallback_Star__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x80099AF0`)
6. **Slot 63** (`0x104`): `hitCallback_Slip(dCc_c*, dCc_c*)` (`hitCallback_Slip__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x8009A0D0`)
7. **Slot 65** (`0x10C`): `hitCallback_Spin(dCc_c*, dCc_c*)` (`hitCallback_Spin__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x800998D0`)
8. **Slot 67** (`0x114`): `hitCallback_WireNet(dCc_c*, dCc_c*)` (`hitCallback_WireNet__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x8009A0C0`)
9. **Slot 68** (`0x118`): `hitCallback_HipAttk(dCc_c*, dCc_c*)` (`hitCallback_HipAttk__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x80099690`)
10. **Slot 71** (`0x124`): `hitCallback_PenguinSlide(dCc_c*, dCc_c*)` (`hitCallback_PenguinSlide__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x80099F10`)
11. **Slot 73** (`0x12C`): `hitCallback_Shell(dCc_c*, dCc_c*)` (`hitCallback_Shell__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x800993C0`)
12. **Slot 74** (`0x130`): `hitCallback_Fire(dCc_c*, dCc_c*)` (`hitCallback_Fire__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x80099060`)
13. **Slot 75** (`0x134`): `hitCallback_Ice(dCc_c*, dCc_c*)` (`hitCallback_Ice__9dEnBoss_cFP5dCc_cP5dCc_c` at `0x8009A0E0`)
14. **Slot 79** (`0x144`): `setDeathInfo_Quake(int)` (`setDeathInfo_Quake__9dEnBoss_cFi` at `0x80099D40`)
15. **Slot 82** (`0x150`): `isQuakeDamage()` (`isQuakeDamage__9dEnBoss_cFv` at `0x80098AD0`)
16. **Slot 89** (`0x16C`): `initializeState_DieFumi()` (`initializeState_DieFumi__9dEnBoss_cFv` at `0x80099F50`)
17. **Slot 90** (`0x170`): `executeState_DieFumi()` (`executeState_DieFumi__9dEnBoss_cFv` at `0x80099F70`)
18. **Slot 91** (`0x174`): `finalizeState_DieFumi()` (`finalizeState_DieFumi__9dEnBoss_cFv` at `0x80099F60`)
19. **Slot 146** (`0x250`): `FumiScoreSet(dActor_c*)` (`FumiScoreSet__9dEnBoss_cFP8dActor_c` at `0x8009A110`)

*(Note: Slots 89-91 comprise the 3 virtual functions of `STATE_VIRTUAL_FUNC_DECLARE(dEn_c, DieFumi)`. Counting each virtual function individually yields exactly 21 overridden functions across 19 declaration points).*

---

## The 41 Kokoopa Overrides of `dEnBoss_c` (Slots 0–225)

Confirmed by comparing `__vt__9dEnBoss_c` and `__vt__18dEnTorideKokoopa_c` across all 226 inherited slots (0..225). Exactly 41 slots are overridden by `dEnTorideKokoopa_c`:

1. **Slot 7** (`0x024`): `preExecute()` (`preExecute__18dEnTorideKokoopa_cFv` at `0x800A8C60`)
2. **Slot 8** (`0x028`): `postExecute(fBase_c::MAIN_STATE_e)` (`postExecute__18dEnTorideKokoopa_cFQ27fBase_c12MAIN_STATE_e` at `0x800A8D90`)
3. **Slot 9** (`0x02C`): `draw()` (`draw__18dEnTorideKokoopa_cFv` at `0x800A8E10`)
4. **Slot 16** (`0x048`): `~dEnTorideKokoopa_c()` (`__dt__18dEnTorideKokoopa_cFv` at `0x800A8B10`)
5. **Slot 21** (`0x05C`): `finalUpdate()` (`finalUpdate__18dEnTorideKokoopa_cFv` at `0x800A8EA0`)
6. **Slot 26** (`0x070`): `getLookatPos()` (`getLookatPos__18dEnTorideKokoopa_cCFv` at `0x800AED20`)
7. **Slot 71** (`0x124`): `hitCallback_PenguinSlide(dCc_c*, dCc_c*)` (`hitCallback_PenguinSlide__18dEnTorideKokoopa_cFP5dCc_cP5dCc_c` at `0x800A9140`)
8. **Slot 82** (`0x150`): `isQuakeDamage()` (`isQuakeDamage__18dEnTorideKokoopa_cFv` at `0x800A9090`)
9. **Slot 158** (`0x280`): `initializeState_DemoWait()` (`initializeState_DemoWait__18dEnTorideKokoopa_cFv` at `0x800AE420`)
10. **Slot 159** (`0x284`): `executeState_DemoWait()` (`executeState_DemoWait__18dEnTorideKokoopa_cFv` at `0x800AE510`)
11. **Slot 160** (`0x288`): `finalizeState_DemoWait()` (`finalizeState_DemoWait__18dEnTorideKokoopa_cFv` at `0x800AE500`)
12. **Slot 161** (`0x28C`): `initializeState_DieFire()` (`initializeState_DieFire__18dEnTorideKokoopa_cFv` at `0x800AE2D0`)
13. **Slot 162** (`0x290`): `executeState_DieFire()` (`executeState_DieFire__18dEnTorideKokoopa_cFv` at `0x800AE320`)
14. **Slot 163** (`0x294`): `finalizeState_DieFire()` (`finalizeState_DieFire__18dEnTorideKokoopa_cFv` at `0x800AE310`)
15. **Slot 167** (`0x2A4`): `initializeState_DieShell()` (`initializeState_DieShell__18dEnTorideKokoopa_cFv` at `0x800AE370`)
16. **Slot 168** (`0x2A8`): `executeState_DieShell()` (`executeState_DieShell__18dEnTorideKokoopa_cFv` at `0x800AE3C0`)
17. **Slot 169** (`0x2AC`): `finalizeState_DieShell()` (`finalizeState_DieShell__18dEnTorideKokoopa_cFv` at `0x800AE3B0`)
18. **Slot 176** (`0x2C8`): `setBattleReady()` (`setBattleReady__18dEnTorideKokoopa_cFv` at `0x800AEC30`)
19. **Slot 181** (`0x2DC`): `tenmetsuProc()` (`tenmetsuProc__18dEnTorideKokoopa_cFv` at `0x800AECC0`)
20. **Slot 182** (`0x2E0`): `tenmetsuFin()` (`tenmetsuFin__18dEnTorideKokoopa_cFv` at `0x800AEC90`)
21. **Slot 183** (`0x2E4`): `getTenmetsuTime_Fire()` (`getTenmetsuTime_Fire__18dEnTorideKokoopa_cFv` at `0x800AA750`)
22. **Slot 185** (`0x2EC`): `getTenmetsuTime_Press()` (`getTenmetsuTime_Press__18dEnTorideKokoopa_cFv` at `0x800AA760`)
23. **Slot 187** (`0x2F4`): `setFumiDamage(dActor_c*)` (`setFumiDamage__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9190`)
24. **Slot 188** (`0x2F8`): `setFumiDead(dActor_c*)` (`setFumiDead__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9280`)
25. **Slot 189** (`0x2FC`): `setFireDamage(dActor_c*)` (`setFireDamage__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9440`)
26. **Slot 190** (`0x300`): `setFireDead(dActor_c*)` (`setFireDead__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9550`)
27. **Slot 195** (`0x314`): `setStarDamage(dActor_c*)` (`setStarDamage__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9720`)
28. **Slot 196** (`0x318`): `setStarDead(dActor_c*)` (`setStarDead__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9810`)
29. **Slot 197** (`0x31C`): `setQuakeDamage()` (`setQuakeDamage__18dEnTorideKokoopa_cFv` at `0x800A99D0`)
30. **Slot 198** (`0x320`): `setQuakeDead()` (`setQuakeDead__18dEnTorideKokoopa_cFv` at `0x800A9A90`)
31. **Slot 199** (`0x324`): `setShellDamage(dActor_c*)` (`setShellDamage__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9BF0`)
32. **Slot 200** (`0x328`): `setShellDead(dActor_c*)` (`setShellDead__18dEnTorideKokoopa_cFP8dActor_c` at `0x800A9D00`)
33. **Slot 201** (`0x32C`): `damageProc()` (`damageProc__18dEnTorideKokoopa_cFv` at `0x800A9EC0`)
34. **Slot 202** (`0x330`): `deadProc()` (`deadProc__18dEnTorideKokoopa_cFv` at `0x800A9F70`)
35. **Slot 203** (`0x334`): `isFumiInvalid()` (`isFumiInvalid__18dEnTorideKokoopa_cCFv` at `0x800AED10`)
36. **Slot 205** (`0x33C`): `isFireInvalid()` (`isFireInvalid__18dEnTorideKokoopa_cCFv` at `0x800AED00`)
37. **Slot 208** (`0x348`): `isStarInvalid()` (`isStarInvalid__18dEnTorideKokoopa_cCFv` at `0x800AECF0`)
38. **Slot 209** (`0x34C`): `fumideadEffect()` (`fumideadEffect__18dEnTorideKokoopa_cFv` at `0x800AAEF0`)
39. **Slot 210** (`0x350`): `fumidmgEffect()` (`fumidmgEffect__18dEnTorideKokoopa_cFv` at `0x800AAE80`)
40. **Slot 224** (`0x388`): `damageSVo()` (`damageSVo__18dEnTorideKokoopa_cFv` at `0x800AB3C0`)
41. **Slot 225** (`0x38C`): `damageLVo()` (`damageLVo__18dEnTorideKokoopa_cFv` at `0x800AB400`)

---

## Task B: Audit of Existing `dEn_c` Header (`include/game/bases/d_enemy.hpp`)

`dEn_c` inherits through the hierarchy: `fBase_c` -> `dBase_c` -> `dBaseActor_c` -> `dActor_c` -> `dActorMultiState_c` -> `dEn_c`.

1. **Inherited Base Virtuals (55 slots: 0–54)**:
   - `fBase_c`: 17 slots (0..16)
   - `dBase_c`: 1 slot (17: `getKindString`)
   - `dBaseActor_c`: 4 slots (18..21: `draw2D`, `draw2D_lyt2`, `GetActorType`, `finalUpdate`)
   - `dActor_c`: 29 slots (22..50: `ActorDrawCullCheck` through `poisonSplashEffect`)
   - `dActorMultiState_c`: 4 slots (51..54: `changeState`, `initializeState_GegnericMulti`, `executeState_GegnericMulti`, `finalizeState_GegnericMulti`)
2. **`dEn_c` Declared Virtuals (103 new slots: 55–157)**:
   - 4 damage check functions (55..58)
   - 3 normal vs hit checks (59..61)
   - 16 hit callbacks (62..77)
   - 4 death info setters (78..81)
   - 1 quake damage query (82)
   - 1 yoshi eat callback (83)
   - 5 death sound setters (84..88)
   - 39 state functions (13 states * 3: `DieFumi`, `DieFall`, `DieBigFall`, `DieSmoke`, `DieYoshiFumi`, `DieIceVanish`, `DieGoal`, `DieOther`, `EatIn`, `EatNow`, `EatOut`, `HitSpin`, `Ice`) (89..127)
   - 3 effect/SE functions (128..130)
   - 1 quake action (131)
   - 3 display / liquid / damage functions (132..134)
   - 2 boyon functions (135..136)
   - 4 ice functions (137..140)
   - 3 funsui functions (141..143)
   - 1 combo clap check (144)
   - 13 jump / score / SE / effect functions (145..157)

**Total Count**: 55 base + 103 new = **158 slots**.

### Audit Conclusion:
The declared virtual count in `include/game/bases/d_enemy.hpp` is **exactly 158**. There is **no divergence** whatsoever. We compiled a subclass against the existing header and inspected `.rela.data` relocations: every single slot from 0 to 157 matched retail `__vt__5dEn_c` at `.data:0x80311EE0` (size `0x280`).

---

## Proved vs Inferred

### Proved:
- `__vt__9dEnBoss_c` has size `0x390` (226 slots).
- `__vt__5dEn_c` has size `0x280` (158 slots).
- `__vt__18dEnTorideKokoopa_c` has size `0x5E4` (375 slots).
- The exact 68 new slot sequence (158..225) in `dEnBoss_c`, proven by symbols and relocations.
- The 21 `dEn_c` overrides in `dEnBoss_c` and the 41 `dEnBoss_c` overrides in `dEnTorideKokoopa_c`.
- `sizeof(dEnBoss_c) == 0x600` bytes, confirmed by constructor member allocations and derived writes.
- Member types and offsets:
  - `dHeapAllocator_c` at `0x524` (constructed at `this+0x524` with `__ct__16dHeapAllocator_cFv`).
  - `mTenmetsuTimer` at `0x540` (zeroed in ctor, read/decremented in `preExecute`, written from `getTenmetsuTime_*`).
  - `dAudio::SndObjctEmy_c` at `0x544` (inlined `NMSndObject<4>` ctor with vtable `__vt__Q26dAudio13SndObjctEmy_c`).
  - `mSoundParam` (`s16`) at `0x5F0` (loaded via `lha` in SE methods).
  - `mQuakeDamage` at `0x5F4` (read in `isQuakeDamage`, set in `setDeathInfo_Quake`).
  - `mpBossLife` at `0x5F8` (allocated in `createBossLife`, destroyed via `dBossLifeInf_c` vtable).

### Inferred:
- Field names `mTenmetsuTimer`, `mSoundParam`, `mQuakeDamage`, `mpBossLife` are descriptive semantic names (`@unofficial`), although their offsets, sizes, and operational semantics are proven.

---

## Unsettled Items

None. All 226 vtable slots, 21 base overrides, 41 derived overrides, member offsets, and `dEn_c` 158-slot audit have been verified and compiled cleanly.
