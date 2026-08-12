#pragma once
#include <game/bases/d_actor.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>

/// @brief Base implementation of a sinking pipe ("sink dokan").
/// @details The pipe is built out of a top unit, a variable number of body units and a root unit,
/// all placed along the actor's rotated axis. Derived actors supply the movement logic.
/// @ingroup bases
class daSinkDokan_c : public dActor_c {
public:
    /// @brief The pipe's unit set. @unofficial
    enum DOKAN_TYPE_e {
        DOKAN_NORMAL, ///< The regular pipe.
        DOKAN_BREAK, ///< The breakable pipe.
        DOKAN_TWIN ///< The double-ended pipe.
    };

    virtual ~daSinkDokan_c();

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void deleteReady();

    // New virtual functions

    virtual bool vfd4() = 0; ///< Checks whether the pipe must be deleted. @unofficial
    virtual void vfd8() = 0; ///< Registers the pipe's tile collision. @unofficial

    virtual void calcMoveDistMax(); ///< Computes @ref mMoveDistMax. @unofficial
    virtual void setMoveDist(); ///< Computes @ref mMoveDist, clamped to @ref mMoveDistMax. @unofficial
    virtual void adjustPos(); ///< Adjusts the pipe's spawn position. @unofficial

    virtual void vfe8() = 0; ///< Builds the pipe's tile collision. @unofficial

    virtual void setDokanUnit_Normal(); ///< Configures the pipe as a regular pipe. @unofficial
    virtual void setDokanUnit_Break(); ///< Configures the pipe as a breakable pipe. @unofficial
    virtual void setDokanUnit_Twin(); ///< Configures the pipe as a double-ended pipe. @unofficial

    // Nonvirtuals

    void createMdl(); ///< Builds the pipe's models. @unofficial
    void setDokanUnit(); ///< Dispatches to the setDokanUnit_* function for @ref mDokanType. @unofficial

    dHeapAllocator_c mAllocator; ///< The allocator for the pipe's models.
    nw4r::g3d::ResFile mResFile; ///< The pipe's model archive.
    m3d::smdl_c mTopModel; ///< The model for the pipe's mouth. @unofficial
    m3d::smdl_c mUnitModels[0x20]; ///< The models for the pipe's body segments. @unofficial
    m3d::smdl_c mRootModel; ///< The model for the pipe's base. @unofficial
    mVec3_c mStartPos; ///< The position the pipe was spawned at. @unofficial
    dBg_ctr_c mBgCtr; ///< The pipe's tile collider. @unofficial
    int m_63c; ///< @todo Figure out the purpose of this field.
    int mDokanType; ///< The pipe's unit set. Value is a DOKAN_TYPE_e. @unofficial
    float mLength; ///< The pipe's total length, in pixels. @unofficial
    float mMoveDist; ///< How far the pipe has come out of the ground. @unofficial
    float mMoveDistMax; ///< The maximum value for @ref mMoveDist. @unofficial
    mAng mTopAngle; ///< The rotation of the pipe's mouth. @unofficial
    mAng mRootAngle; ///< The rotation of the pipe's base. @unofficial

    static const float smc_MAX_SPEED; ///< The pipe's maximum movement speed. @unofficial

    ACTOR_PARAM_CONFIG(PlayerNo, 0, 2); ///< @unofficial
    ACTOR_PARAM_CONFIG(DokanType, 4, 2); ///< @unofficial
    ACTOR_PARAM_CONFIG(Unk63c, 8, 2); ///< @unofficial
    ACTOR_PARAM_CONFIG(UnitCount, 16, 8); ///< @unofficial
};
