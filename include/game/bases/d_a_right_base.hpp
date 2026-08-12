#pragma once
#include <game/bases/d_actor_state.hpp>
#include <game/bases/d_switch_flag_manager.hpp>


/// @brief Base implementation of a light actor that follows and rotates around a parent actor.
/// @ingroup bases
class daLightBase_c : public dActorState_c {
public:
    /// @brief The kind of actor the light is attached to. @unofficial
    enum PARENT_TYPE_e {
        PARENT_RAIL, ///< A rail polygon parent.
        PARENT_OBJ, ///< A paired object parent.
        PARENT_CENTER_A, ///< An OBJ_CENTER parent.
        PARENT_CENTER_B, ///< An OBJ_CENTER2 parent.
    };

    virtual ~daLightBase_c();

    virtual int create();
    virtual int draw();

    virtual void drawLight() = 0; ///< Draws the light itself. @unofficial

    STATE_VIRTUAL_FUNC_DECLARE(daLightBase_c, Move); ///< Following the parent.
    STATE_FUNC_DECLARE(daLightBase_c, SearchID); ///< Looking for the parent actor.

    static dActor_c *searchParent_rail(u32 id); ///< Finds the rail polygon parent with the given ID.
    static dActor_c *searchParent_obj(u32 id); ///< Finds the paired object parent with the given ID.
    static dActor_c *searchParent_centerA(u32 id); ///< Finds the OBJ_CENTER parent with the given ID.
    static dActor_c *searchParent_centerB(u32 id); ///< Finds the OBJ_CENTER2 parent with the given ID.
    static dActor_c *searchParent(u32 type, u32 id); ///< Finds the parent of the given kind and ID.

    void setParentInfo(dActor_c *parent); ///< Records the parent and the offset from it.
    void rotation_move(dActor_c *parent); ///< Moves the light to follow the parent.

    dActor_c *setParentInfo(); ///< Looks the parent up, then calls the two functions above.
    dActor_c *rotation_move(); ///< Looks the parent up by ID, then moves the light.

    void calcLightOnOff(); ///< Updates the light's on/off state from the event flags.

    fBaseID_e mParentID; ///< The parent actor's unique ID.
    mVec3_c mOffset; ///< The offset from the parent actor.
    mAng mRotation; ///< The rotation applied to the offset. @unofficial
    bool mLightOn; ///< Whether the light is currently on. @unofficial
    u8 mParentNo; ///< The parent actor's ID within its kind. @unofficial
    u8 mParentType; ///< The parent actor's kind. See PARENT_TYPE_e. @unofficial
    u8 mRotateAxis; ///< Which axis the parent moves the light along. @unofficial
    u8 mSwitchType; ///< How the light reacts to event flags. @unofficial
    s8 mPlayerNo; ///< The player who switched the light on, or @p -1 . @unofficial

    ACTOR_PARAM_CONFIG(ParentNo, 0, 8); ///< @unofficial
    ACTOR_PARAM_CONFIG(ParentType, 8, 2); ///< @unofficial
    ACTOR_PARAM_CONFIG(SwitchType, 12, 2); ///< @unofficial

    ACTOR_PARAM_CONFIG(RailParentNo, 4, 4); ///< The rail parent's own ID field. @unofficial
    ACTOR_PARAM_CONFIG(ObjParentNo, 24, 8); ///< The paired object parent's own ID field. @unofficial
};
