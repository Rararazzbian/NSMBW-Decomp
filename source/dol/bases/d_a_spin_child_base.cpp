#include <game/bases/d_a_spin_child_base.hpp>
#include <game/framework/f_manager.hpp>

STATE_DEFINE(daSpinChildBase_c, SearchID);
STATE_DEFINE(daSpinChildBase_c, Move);

dActor_c *daSpinChildBase_c::searchParent(u32 no) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_SPIN_PARENT, actor)) != nullptr) {
        // [daObjSpinParent_c has not been decompiled yet; read its ID field by offset.]
        if (no == ((u8 *) actor)[0x438]) {
            return actor;
        }
    }
    return nullptr;
}

void daSpinChildBase_c::setParentInfo(dActor_c *parent) {
    mParentID = parent->mUniqueID;
}

void daSpinChildBase_c::move_with_parent(dActor_c *parent) {
    // [daObjSpinParent_c has not been decompiled yet; read its offset field by offset.]
    float move = *(float *) ((u8 *) parent + 0x434) * mMoveScale;

    switch (mMoveDir) {
        case MOVE_RIGHT:
            mPos.x = mBasePos + move;
            break;
        case MOVE_LEFT:
            mPos.x = mBasePos - move;
            break;
        case MOVE_UP:
            mPos.y = mBasePos + move;
            break;
        default:
            mPos.y = mBasePos - move;
            break;
    }
}

dActor_c *daSpinChildBase_c::move_with_parent() {
    dActor_c *parent = (dActor_c *) fManager_c::searchBaseByID(mParentID);
    if (parent != nullptr) {
        move_with_parent(parent);
    }
    return parent;
}

dActor_c *daSpinChildBase_c::setParentInfo(u32 no) {
    dActor_c *parent = searchParent(no);
    if (parent != nullptr) {
        setParentInfo(parent);
        move_with_parent(parent);
        mStateMgr.changeState(StateID_Move);
    }
    return parent;
}

int daSpinChildBase_c::create() {
    u32 param = mParam;
    u32 no = ACTOR_PARAM_LOCAL(param, ParentNo);

    mParentID = (fBaseID_e) 0;
    mMoveScale = ACTOR_PARAM_LOCAL(param, MoveScale);
    mParentNo = no;
    mMoveDir = MOVE_DOWN;

    if (init() == 0) {
        return NOT_READY;
    }

    if (mMoveDir <= MOVE_LEFT) {
        mBasePos = mPos.x;
    } else {
        mBasePos = mPos.y;
    }

    if (setParentInfo(no) == nullptr) {
        mStateMgr.changeState(StateID_SearchID);
    }

    return SUCCEEDED;
}

int daSpinChildBase_c::execute() {
    if (ActorScrOutCheck(0)) {
        return SUCCEEDED;
    }

    if (mParentID != 0 && move_with_parent() == nullptr) {
        deleteActor(1);
        return SUCCEEDED;
    }

    mStateMgr.executeState();
    post_execute_state();

    return SUCCEEDED;
}

int daSpinChildBase_c::preDraw() {
    if (dActor_c::preDraw() == 0) {
        return NOT_READY;
    }
    return mParentID != 0;
}

int daSpinChildBase_c::init() {
    return 1;
}

void daSpinChildBase_c::init_move() {}

void daSpinChildBase_c::post_execute_state() {}

void daSpinChildBase_c::initializeState_SearchID() {}

void daSpinChildBase_c::finalizeState_SearchID() {}

void daSpinChildBase_c::executeState_SearchID() {
    setParentInfo(mParentNo);
}

void daSpinChildBase_c::initializeState_Move() {
    init_move();
}

void daSpinChildBase_c::finalizeState_Move() {}

void daSpinChildBase_c::executeState_Move() {}
