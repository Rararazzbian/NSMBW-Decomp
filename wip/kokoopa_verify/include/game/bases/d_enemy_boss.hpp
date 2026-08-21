#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_audio.hpp>

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

    int mLife;
};

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

class dEnBoss_c : public dEn_c {
public:
    dEnBoss_c();
    virtual ~dEnBoss_c();

    // dEn_c overrides (21 functions across 19 slot groups)
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

    virtual BOOL isFumiInvalid() const;
    virtual bool isFumiDmgInvalid() const;
    virtual BOOL isFireInvalid() const;
    virtual bool isSlideInvalid() const;
    virtual bool isShellInvalid() const;
    virtual BOOL isStarInvalid() const;

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
    dHeapAllocator_c mAllocator; // 0x524
    u32 mTenmetsuTimer; // 0x540
    dAudio::SndObjctEmy_c mSound; // 0x544
    s16 mSoundParam; // 0x5F0
    u8 mPadBoss[2]; // 0x5F2
    u32 mQuakeDamage; // 0x5F4
    dBossLifeInf_c *mpBossLife; // 0x5F8
    u8 mPadEnd[4]; // 0x5FC
};

STATIC_ASSERT(sizeof(dEnBoss_c) == 0x600);
STATIC_ASSERT(sizeof(dBossLifeInf_c) == 0x8);
STATIC_ASSERT(sizeof(dBossLife_Common_c) == 0x8);
