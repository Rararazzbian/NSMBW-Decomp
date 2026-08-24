#pragma once
#include <types.h>
#include <game/bases/d_base_actor.hpp>

/// @brief A fountain/geyser position actor helper.
/// @unofficial The only retail symbol in wiimj2d.dol is ::posMove, which moves
/// a player riding the fountain and reports to an embedded controller object.
/// Nothing in this module constructs the class, so the member roles below are
/// inferred from posMove's accesses alone.
class dFunsuiAct_c {
public:
    /// @unofficial Embedded polymorphic object at +0x1C. Only its vtable slot
    /// at +0xA0 is evidenced (called from posMove with
    /// (player, 0, mVec3_c &, 0, 0)); everything above it is padding so the
    /// callee lands on the right slot. No instance or vtable is emitted here.
    class Obj {
    public:
        virtual void unk00(); virtual void unk01(); virtual void unk02();
        virtual void unk03(); virtual void unk04(); virtual void unk05();
        virtual void unk06(); virtual void unk07(); virtual void unk08();
        virtual void unk09(); virtual void unk10(); virtual void unk11();
        virtual void unk12(); virtual void unk13(); virtual void unk14();
        virtual void unk15(); virtual void unk16(); virtual void unk17();
        virtual void unk18(); virtual void unk19(); virtual void unk20();
        virtual void unk21(); virtual void unk22(); virtual void unk23();
        virtual void unk24(); virtual void unk25(); virtual void unk26();
        virtual void unk27(); virtual void unk28(); virtual void unk29();
        virtual void unk30(); virtual void unk31(); virtual void unk32();
        virtual void unk33(); virtual void unk34(); virtual void unk35();
        virtual void unk36(); virtual void unk37();
        virtual void exec(dBaseActor_c *actor, int, mVec3_c &, int, int);
    };

    void posMove();

    fBaseID_e mBaseId;   ///< 0x00 base ID of the player actor this tracks. @unofficial
    float mTargetY;      ///< 0x04 Y position handed to updateFunsuiPos. @unofficial
    float mCurrentY;     ///< 0x08 accumulated Y written to the player's mPos.z. @unofficial
    float mSpeed;        ///< 0x0C per-frame increment, zeroed while revolving. @unofficial
    u32 mPad10;          ///< 0x10 never read by posMove. @unofficial
    int mRevIndex;       ///< 0x14 selects cs_rev_speed[mRevIndex] (+3/-3). @unofficial
    int mTimer;          ///< 0x18 countdown gating the rev-speed branch. @unofficial
    Obj mObj;            ///< 0x1C embedded controller object (vtable slot +0xA0 used). @unofficial
};
