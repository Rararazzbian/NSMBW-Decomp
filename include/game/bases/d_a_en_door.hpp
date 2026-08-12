#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>

/// @brief Placeholder for the undecompiled rotation center actor (::OBJ_CENTER).
/// @unofficial
class daObjCenter_c : public dActor_c {
public:
    u8 m_392[0x39c - 0x392];
    u8 mNo; ///< The center's identifier, matched against a door's ::PARAM_ParentNo.
};

/// @brief Placeholder for the undecompiled rotation center actor (::OBJ_CENTER2).
/// @unofficial
class daObjCenter2_c : public dActor_c {
public:
    u8 m_392[0x3e2 - 0x392];
    u8 mNo; ///< The center's identifier, matched against a door's ::PARAM_ParentNo.
};

/**
 * @brief Base implementation of a door.
 * @details Doors may be attached to a rotation center actor, in which case they
 * orbit it. Derived actors supply the model, the animations and the sounds.
 * @statetable
 * @paramtable
 */
class daEnDoor_c : public dEn_c {
public:
    virtual ~daEnDoor_c();

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual bool ActorDrawCullCheck();

    STATE_FUNC_DECLARE(daEnDoor_c, Search); ///< Looking for the door's rotation center.
    STATE_FUNC_DECLARE(daEnDoor_c, Open); ///< Playing the opening animation.
    STATE_FUNC_DECLARE(daEnDoor_c, Close); ///< Playing the closing animation.
    STATE_FUNC_DECLARE(daEnDoor_c, Wait); ///< Idling, waiting for a player.
    STATE_FUNC_DECLARE(daEnDoor_c, Dummy); ///< The door was a fake and is vanishing.

    // New virtual functions

    virtual bool isClosed(); ///< Returns whether the door is idle (fully closed).
    virtual bool isDummyOpen(); ///< Returns whether the door is a fake one.
    virtual void createMdl(); ///< Builds the door's model.
    virtual void initAnm(); ///< Initializes the door's animations.
    virtual void initCcData(); ///< Initializes the door's collider.
    virtual void initialize(); ///< Performs any remaining setup.
    virtual void waitProc(); ///< Per-frame processing while idling.
    virtual void setOpenAnm(); ///< Starts the opening animation.
    virtual void setOpenSE(); ///< Plays the opening sound.
    virtual void setCloseAnm(); ///< Starts the closing animation.
    virtual void setCloseSE(); ///< Plays the closing sound.
    virtual void setCloseMoveSE(); ///< Plays the sound for the closing movement.
    virtual void setWaitAnm(); ///< Starts the idle animation.

    // Nonvirtuals

    void allocate(); ///< Creates the actor's heap and its model.
    void calcMdl(); ///< Updates the model's transform.
    bool checkOpenOk(); ///< Returns whether the door may be opened right now.

    /// @brief Records the given actor as this door's rotation center.
    void setParentInfo(dActor_c *parent);

    /// @brief Looks up the rotation center with the given identifier and attaches to it.
    /// @return The rotation center, or @p nullptr if not found.
    dActor_c *setParentInfo(u32 no);

    /// @brief Places the door relative to the given rotation center.
    void rotation_move(dActor_c *parent);

    /// @brief Places the door relative to its rotation center.
    /// @return The rotation center, or @p nullptr if it no longer exists.
    dActor_c *rotation_move();

    /// @brief Searches for the rotation center with the given identifier.
    static dActor_c *searchParent(u32 no);
    static dActor_c *searchParent_centerA(u32 no); ///< @copydoc searchParent
    static dActor_c *searchParent_centerB(u32 no); ///< @copydoc searchParent

    /// @brief The door's collision callback.
    static void ccCallback(dCc_c *self, dCc_c *other);

    dHeapAllocator_c mAllocator; ///< The actor's allocator.
    nw4r::g3d::ResFile mResFile; ///< The actor's resource file.
    m3d::mdl_c mModel; ///< The door's model.
    m3d::anmChr_c mAnmChr; ///< The door's bone animation.
    fBaseID_e mParentID; ///< The @ref fBase_c::mUniqueID "unique identifier" of the rotation center.
    mVec3_c mParentOffset; ///< The offset from the rotation center to the door.
    int mOpenType; ///< How the door was triggered. @unofficial
    int mIsOpen; ///< Whether the opening animation has finished. @unofficial
    int mDemoActive; ///< Whether the door cutscene is running. @unofficial
    s16 mBaseAngle; ///< The door's rotation offset from the rotation center. @unofficial
    bool mHasParent; ///< Whether the door orbits a rotation center. @unofficial
    u8 mParentNo; ///< The identifier of the door's rotation center. @unofficial

    ACTOR_PARAM_CONFIG(ParentNo, 0, 8); ///< @unofficial
    ACTOR_PARAM_CONFIG(HasParent, 8, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(Direction, 12, 2); ///< @unofficial
};
