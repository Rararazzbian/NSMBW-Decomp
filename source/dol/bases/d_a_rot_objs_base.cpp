#include <game/bases/d_a_rot_objs_base.hpp>
#include <game/framework/f_manager.hpp>

STATE_VIRTUAL_DEFINE(daRotObjsBase_c, Search);
STATE_VIRTUAL_DEFINE(daRotObjsBase_c, Move);

dActor_c *daRotObjsBase_c::searchParent_centerA(u32 id) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER, actor)) != nullptr) {
        // [daObjCenter_c has not been decompiled yet; read its ID field by offset.]
        if (id == ((u8 *) actor)[0x39c]) {
            return actor;
        }
    }
    return nullptr;
}

dActor_c *daRotObjsBase_c::searchParent_centerB(u32 id) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER2, actor)) != nullptr) {
        // [daObjCenter2_c has not been decompiled yet; read its ID field by offset.]
        if (id == ((u8 *) actor)[0x3e2]) {
            return actor;
        }
    }
    return nullptr;
}

dActor_c *daRotObjsBase_c::searchParent(u32 id) {
    dActor_c *parent = searchParent_centerA(id);
    if (parent != nullptr) {
        return parent;
    }
    return searchParent_centerB(id);
}

void daRotObjsBase_c::setParentInfo(const dActor_c *parent) {
    mParentID = parent->mUniqueID;
    mOffset.set(mPos.x - parent->mPos.x, mPos.y - parent->mPos.y);
}

void daRotObjsBase_c::rotation_move(const dActor_c *parent) {
    mAng angle = parent->mAngle.z;
    angle += 0x4000;
    mAngle.z = angle;

    f32 cos = nw4r::math::CosIdx(angle);
    f32 sin = nw4r::math::SinIdx(angle);
    f32 ox = mOffset.x;
    f32 oy = mOffset.y;

    mPos.x = parent->mPos.x + (ox * cos - oy * sin);
    mPos.y = parent->mPos.y + (ox * sin + oy * cos);
}

dActor_c *daRotObjsBase_c::setParentInfo(u32 id) {
    dActor_c *parent = searchParent(id);
    if (parent != nullptr) {
        setParentInfo(parent);
        rotation_move(parent);
        mStateMgr.changeState(StateID_Move);
    }
    return parent;
}

dActor_c *daRotObjsBase_c::rotation_move() {
    dActor_c *parent = (dActor_c *) fManager_c::searchBaseByID(mParentID);
    if (parent != nullptr) {
        rotation_move(parent);
    }
    return parent;
}

void daRotObjsBase_c::setBgData(dBg_ctr_c *start, dBg_ctr_c *end, obj_bg_data_t *data) {
    mpBgCtrStart = start;
    mpBgCtrEnd = end;
    mpObjBgData = data;
}

int daRotObjsBase_c::create() {
    if (!init()) {
        return NOT_READY;
    }

    u32 param = mParam;
    mParentID = (fBaseID_e) 0;
    mParentNo = ACTOR_PARAM_LOCAL(param, ParentNo);

    if (setParentInfo(ACTOR_PARAM_LOCAL(param, ParentNo)) == nullptr) {
        mStateMgr.changeState(StateID_Search);
    }

    return SUCCEEDED;
}

int daRotObjsBase_c::execute() {
    if (scroll_out_check()) {
        return SUCCEEDED;
    }

    if (mParentID != 0 && rotation_move() == nullptr) {
        deleteActor(1);
        return SUCCEEDED;
    }

    mStateMgr.executeState();
    post_execute_state();

    return SUCCEEDED;
}

int daRotObjsBase_c::preDraw() {
    if (dActor_c::preDraw() == NOT_READY) {
        return NOT_READY;
    }
    return mParentID != 0;
}

int daRotObjsBase_c::doDelete() {
    if (mParentID != 0) {
        dBg_ctr_c *ctr = mpBgCtrStart;
        dBg_ctr_c *end = mpBgCtrEnd;
        do {
            ctr->release();
            ctr++;
        } while (ctr < end);
    }
    return SUCCEEDED;
}

bool daRotObjsBase_c::scroll_out_check() {
    return ActorScrOutCheck(0);
}

void daRotObjsBase_c::post_execute_state() {}

void daRotObjsBase_c::initializeState_Search() {}

void daRotObjsBase_c::finalizeState_Search() {}

void daRotObjsBase_c::executeState_Search() {
    setParentInfo(mParentNo);
}

void daRotObjsBase_c::initializeState_Move() {
    if (mParentID != 0) {
        obj_bg_data_t *data = mpObjBgData;
        dBg_ctr_c *ctr = mpBgCtrStart;
        dBg_ctr_c *end = mpBgCtrEnd;
        do {
            ctr->set(this, data->mTopLeft, data->mBottomRight, nullptr, nullptr, nullptr, 3, 0, nullptr);
            ctr->mFlags = 1;
            ctr->entry();
            ctr++;
            data++;
        } while (ctr < end);
    }
}

void daRotObjsBase_c::finalizeState_Move() {}

void daRotObjsBase_c::executeState_Move() {
    if (mParentID != 0) {
        dBg_ctr_c *ctr = mpBgCtrStart;
        dBg_ctr_c *end = mpBgCtrEnd;
        do {
            ctr->calc();
            ctr++;
        } while (ctr < end);
    }
}
