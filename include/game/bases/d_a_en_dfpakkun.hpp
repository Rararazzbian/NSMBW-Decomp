#pragma once
#include <game/bases/d_a_en_dpakkun_base.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>

/**
 * @brief A fire piranha plant that grows out of a pipe.
 * @details The plant waits inside the pipe, rises out of it, searches for a
 * player, spits fireballs at them and sinks back in. The direction-specific
 * behaviour is supplied by the four @p daEnDokanFPakkun*_c subclasses, which
 * add no data of their own and only override the five hook virtuals declared
 * at the end of this class.
 * @statetable
 * @unofficial
 */
class daEnDfpakkun_c : public daEnDpakkunBase_c {
public:
    /// @brief Constructs the enemy.
    /// @details Must be user-declared and defined out of line as the FIRST
    /// definition in the .cpp: nothing in this TU constructs a
    /// @p daEnDfpakkun_c, so an implicit constructor is never emitted and
    /// @p __ct__14daEnDfpakkun_cFv (0x800281C0) would go missing.
    /// The body is empty -- @p mNodeCallback.mpOwner is set by the
    /// @ref daEnDpakkunBase_c constructor.
    daEnDfpakkun_c();

    virtual ~daEnDfpakkun_c();

    // Base class overrides

    virtual void setIceAnm();
    virtual void returnAnm_Ice();

    STATE_VIRTUAL_FUNC_DECLARE(daEnDfpakkun_c, Wait);        ///< Hiding inside the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDfpakkun_c, Appear);      ///< Rising out of the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDfpakkun_c, Attack);      ///< Searching for a player and spitting fire.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDfpakkun_c, Disappear);   ///< Sinking back into the pipe.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDfpakkun_c, DieIceBreak); ///< Shattering after being frozen.
    STATE_VIRTUAL_FUNC_DECLARE(daEnDfpakkun_c, DieVanish);   ///< Withering away.

    virtual void setIceBreakAnm();
    virtual void initialize();
    virtual void createMdl();
    virtual void setVanishAnm();

    // New virtual functions (vtable 0x2E0..0x2F0)

    /// @brief Checks whether the plant may leave the pipe. @unofficial
    /// @note Inline in the class body on purpose. It is called from
    /// executeState_Wait, so the out-of-line copy is emitted lazily at that
    /// caller's flush point, which is where the original has it. Defined out of
    /// line it lands after reviveCc instead of before it.
    virtual bool checkAppear() { return true; }

    /// @brief Picks the player to aim at, or -1 for none. @unofficial
    virtual int search();

    /// @brief Computes the parameter word handed to the spawned fireball. @unofficial
    /// @note Inline in the class body, and required to be. It sits at
    /// 0x8002A1E0, inside the end-of-TU inline-flush block between
    /// hitCallback_Spin and m3d::banm_c::play. An out-of-line definition is
    /// emitted at its source position, which is always ahead of that block, so
    /// it cannot reach that address. Nothing in the DOL calls it.
    virtual int calcFirePrm() { return 0; }

    /// @brief Picks the facing direction to chase. @unofficial
    /// @note Inline in the class body, for the same lazy-flush reason as
    /// checkAppear: initializeState_Appear is what flushes it, and only the
    /// inline form puts it between initializeState_Appear and
    /// finalizeState_Appear where the original has it.
    virtual bool searchDir() { return getPl_LRflag(mPos); }

    /// @brief Bends the neck towards the target. @unofficial
    virtual bool adjustNeck();

    // Nonvirtuals

    void setMoveSpeed(int reverse);
    void setMoveAnm(float blendFrame);
    void setSearchAnm(float startFrame, float blendFrame);
    void setAttackAnm(float startFrame, float blendFrame);
    void setEatAnm(float blendFrame);
    BOOL neckChase(int target);
    void fireSet();

    m3d::anmChr_c mNeckAnm;         ///< 0x6E0 The neck bending animation. @unofficial
    m3d::anmChrBlend_c mBlendAnm;   ///< 0x718 Blends #mNeckAnm over daEnDpakkunBase_c::mAnm. @unofficial
    int mShotsLeft;                 ///< 0x740 Remaining fireballs in this attack. @unofficial
    u8 m_744[0x748 - 0x744];        ///< 0x744 @unused
    int mSearchResult;              ///< 0x748 Last value returned by #search. @unofficial
    float mNeckFrame;               ///< 0x74C Neck animation frame, chased towards the target frame. @unofficial
    u8 m_750[0x754 - 0x750];        ///< 0x750 @unused
    int mSearchDir;                 ///< 0x754 Last value returned by #searchDir. @unofficial
    u8 m_758[0x75A - 0x758];        ///< 0x758 @unused
    /// @note Named apart from daEnDpakkunBase_c::mNeckAngle deliberately. That
    /// is a different member -- an s16[9] at 0x6B4 -- and giving both the same
    /// name forces every use of the base array in this class to be written
    /// daEnDpakkunBase_c::mNeckAngle[i], which the original source cannot
    /// plausibly have done. Which of the two is misnamed is unknown; both are
    /// @unofficial.
    s16 mNeckSwingAngle;            ///< 0x75A Neck bend angle, used by the subclasses. @unofficial
    u8 m_75c[0x760 - 0x75C];        ///< 0x75C @unused
};
