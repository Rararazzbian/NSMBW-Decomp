
#include <game/bases/d_enemy_boss.hpp>

class dEnTorideKokoopa_c : public dEnBoss_c {
public:
    dEnTorideKokoopa_c();
    virtual ~dEnTorideKokoopa_c();

    // 41 overrides of dEnBoss_c
    virtual int preExecute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual void finalUpdate();
    virtual mVec2_c getLookatPos() const;
    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other);
    virtual BOOL isQuakeDamage();
    virtual void initializeState_DemoWait();
    virtual void executeState_DemoWait();
    virtual void finalizeState_DemoWait();
    virtual void initializeState_DieFire();
    virtual void executeState_DieFire();
    virtual void finalizeState_DieFire();
    virtual void initializeState_DieShell();
    virtual void executeState_DieShell();
    virtual void finalizeState_DieShell();
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
    virtual bool isFumiInvalid() const;
    virtual bool isFireInvalid() const;
    virtual bool isStarInvalid() const;
    virtual void fumideadEffect();
    virtual void fumidmgEffect();
    virtual void damageSVo();
    virtual void damageLVo();

    // 149 new virtual functions in Kokoopa (Jump_St, etc.)
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Jump_St);
};

dEnTorideKokoopa_c::dEnTorideKokoopa_c() {}
dEnTorideKokoopa_c::~dEnTorideKokoopa_c() {}
