#pragma once
#include <game/bases/d_actor_state.hpp>

/// @brief Base implementation of an object that rides on a spinning parent object.
/// @details The parent is an @ref fProfile::OBJ_SPIN_PARENT actor, which publishes a
/// single displacement value. Each child copies that displacement onto one axis of its
/// own spawn position, in one of four directions.
/// @unofficial Reconstructed from the vtable, the static initialiser and the code.
/// @ingroup bases
class daSpinChildBase_c : public dActorState_c {
public:
    /// @brief The axis and sign the parent's displacement is applied with. @unofficial
    enum MOVE_DIR_e {
        MOVE_RIGHT, ///< The child moves along +X.
        MOVE_LEFT, ///< The child moves along -X.
        MOVE_UP, ///< The child moves along +Y.
        MOVE_DOWN, ///< The child moves along -Y.
    };

    virtual ~daSpinChildBase_c() {}

    // Base class overrides

    virtual int create();
    virtual int execute();
    virtual int preDraw();

    // New virtual functions

    virtual int init(); ///< Sets up the derived actor. Returning @p 0 aborts creation. @unofficial
    virtual void init_move(); ///< Called when the child starts following the parent. @unofficial
    virtual void post_execute_state(); ///< Called after the state machine has run. @unofficial

    STATE_FUNC_DECLARE(daSpinChildBase_c, SearchID); ///< Looking for the parent actor.
    STATE_FUNC_DECLARE(daSpinChildBase_c, Move); ///< Following the parent actor.

    /// @brief Finds the spin parent with the given ID.
    static dActor_c *searchParent(u32 no);

    void setParentInfo(dActor_c *parent); ///< Records the parent's unique ID.
    void move_with_parent(dActor_c *parent); ///< Applies the parent's displacement.

    dActor_c *setParentInfo(u32 no); ///< Looks the parent up by ID, then binds to it.
    dActor_c *move_with_parent(); ///< Looks the parent up, then moves with it.

    fBaseID_e mParentID; ///< The parent actor's unique ID.
    float mMoveScale; ///< Scales the parent's displacement. @unofficial
    float mBasePos; ///< The spawn coordinate the displacement is added to. @unofficial
    u8 mParentNo; ///< The parent actor's ID within its profile. @unofficial
    u8 mMoveDir; ///< The direction the child moves in. See MOVE_DIR_e. @unofficial

    ACTOR_PARAM_CONFIG(ParentNo, 28, 4); ///< @unofficial
    ACTOR_PARAM_CONFIG(MoveScale, 8, 1); ///< @unofficial
};
