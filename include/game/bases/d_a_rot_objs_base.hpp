#pragma once
#include <game/bases/d_actor_state.hpp>
#include <game/bases/d_bg_ctr.hpp>

/// @brief Base implementation of an actor that rotates around a parent actor and carries
/// its own tile collision.
/// @ingroup bases
class daRotObjsBase_c : public dActorState_c {
public:
    /// @brief The bounds of one of the actor's tile colliders. @unofficial
    struct obj_bg_data_t {
        mVec2_c mTopLeft; ///< The collider's top-left corner, relative to the actor.
        mVec2_c mBottomRight; ///< The collider's bottom-right corner, relative to the actor.
    };

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int preDraw();

    virtual int init() = 0; ///< Builds the actor's model and collision. @unofficial
    virtual bool scroll_out_check(); ///< Checks whether the actor has scrolled out of view. @unofficial
    virtual void post_execute_state(); ///< Runs after the state machine has been updated. @unofficial

    STATE_VIRTUAL_FUNC_DECLARE(daRotObjsBase_c, Search); ///< Looking for the parent actor.
    STATE_VIRTUAL_FUNC_DECLARE(daRotObjsBase_c, Move); ///< Following the parent.

    static dActor_c *searchParent_centerA(u32 id); ///< Finds the OBJ_CENTER parent with the given ID.
    static dActor_c *searchParent_centerB(u32 id); ///< Finds the OBJ_CENTER2 parent with the given ID.
    static dActor_c *searchParent(u32 id); ///< Finds the parent with the given ID.

    void setParentInfo(const dActor_c *parent); ///< Records the parent and the offset from it.
    void rotation_move(const dActor_c *parent); ///< Moves the actor to follow the parent.

    dActor_c *setParentInfo(u32 id); ///< Looks the parent up, then calls the two functions above.
    dActor_c *rotation_move(); ///< Looks the parent up by ID, then moves the actor.

    /// @brief Registers the tile collider array and its per-collider data. @unofficial
    void setBgData(dBg_ctr_c *start, dBg_ctr_c *end, obj_bg_data_t *data);

    mVec2_c mOffset; ///< The offset from the parent actor.
    fBaseID_e mParentID; ///< The parent actor's unique ID.
    dBg_ctr_c *mpBgCtrStart; ///< The first tile collider. @unofficial
    dBg_ctr_c *mpBgCtrEnd; ///< One past the last tile collider. @unofficial
    obj_bg_data_t *mpObjBgData; ///< The per-collider bounds. @unofficial
    u8 mParentNo; ///< The parent actor's ID within its kind. @unofficial

    ACTOR_PARAM_CONFIG(ParentNo, 0, 8); ///< @unofficial
};
