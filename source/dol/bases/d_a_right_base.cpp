#include <game/bases/d_a_right_base.hpp>
#include <game/framework/f_manager.hpp>

STATE_DEFINE(daLightBase_c, SearchID);
STATE_VIRTUAL_DEFINE(daLightBase_c, Move);

dActor_c *daLightBase_c::searchParent_rail(u32 id) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::EN_RAIL_POLY_PARENT, actor)) != nullptr) {
        if (id == ACTOR_PARAM_LOCAL(actor->mParam, RailParentNo)) {
            return actor;
        }
    }
    return nullptr;
}

dActor_c *daLightBase_c::searchParent_obj(u32 id) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::EN_PAIR_OBJ_PARENT, actor)) != nullptr) {
        if (id == ACTOR_PARAM_LOCAL(actor->mParam, ObjParentNo)) {
            return actor;
        }
    }
    return nullptr;
}

dActor_c *daLightBase_c::searchParent_centerA(u32 id) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER, actor)) != nullptr) {
        // [daObjCenter_c has not been decompiled yet; read its ID field by offset.]
        if (id == ((u8 *) actor)[0x39c]) {
            return actor;
        }
    }
    return nullptr;
}

dActor_c *daLightBase_c::searchParent_centerB(u32 id) {
    dActor_c *actor = nullptr;
    while ((actor = (dActor_c *) fManager_c::searchBaseByProfName(fProfile::OBJ_CENTER2, actor)) != nullptr) {
        // [daObjCenter2_c has not been decompiled yet; read its ID field by offset.]
        if (id == ((u8 *) actor)[0x3e2]) {
            return actor;
        }
    }
    return nullptr;
}

dActor_c *daLightBase_c::searchParent(u32 type, u32 id) {
    switch (type) {
        case PARENT_RAIL:
            return searchParent_rail(id);
        case PARENT_OBJ:
            return searchParent_obj(id);
        case PARENT_CENTER_A:
            return searchParent_centerA(id);
        case PARENT_CENTER_B:
            return searchParent_centerB(id);
    }
    return nullptr;
}

void daLightBase_c::setParentInfo(dActor_c *parent) {
    u32 type = mParentType;

    mParentID = parent->mUniqueID;

    if (type == PARENT_RAIL) {
        // [daEnRailPolyParent_c has not been decompiled yet; read its position by offset.]
        mOffset = mPos - *(mVec3_c *) ((u8 *) parent + 0x3ec);

    } else if (type == PARENT_OBJ) {
        mOffset.x = mPos.x;
        mOffset.y = mPos.y;
        mOffset.z = mPos.z;
        // [daEnPairObjParent_c has not been decompiled yet; read its fields by offset.]
        mRotateAxis = ((u8 *) parent)[0x5a1];

    } else if (type == PARENT_CENTER_A || type == PARENT_CENTER_B) {
        mVec3_c center = getCenterPos();
        mOffset = center - parent->mPos;
    }
}

void daLightBase_c::rotation_move(dActor_c *parent) {
    u32 type = mParentType;

    if (type == PARENT_RAIL) {
        mPos = parent->mPos + mOffset;

    } else if (type == PARENT_OBJ) {
        f32 move = *(f32 *) ((u8 *) parent + 0x524); // daEnPairObjParent_c, see above
        if (mRotateAxis == 0) {
            mPos.x = mOffset.x + move;
        } else {
            mPos.y = mOffset.y + move;
        }

    } else if (type == PARENT_CENTER_A || type == PARENT_CENTER_B) {
        mVec3_c pos = parent->mPos;
        mAng angle = parent->mAngle.z;
        angle += 0x4000;
        mRotation = angle;
        f32 cos = nw4r::math::CosIdx(angle);
        f32 sin = nw4r::math::SinIdx(angle);
        f32 ox = mOffset.x;
        f32 oy = mOffset.y;
        mPos.x = pos.x + (ox * cos - oy * sin);
        mPos.y = pos.y + (ox * sin + oy * cos);
        // [Only X and Y are recomputed above, but the center offset is then subtracted
        // from the whole vector: Z is decremented again on every call, so the light
        // drifts along Z whenever mCenterOffs.z is non-zero. Reproduced as-is.]
        mPos -= mCenterOffs;
    }
}

int daLightBase_c::create() {
    u32 param = mParam;
    u32 type = ACTOR_PARAM_LOCAL(param, SwitchType);

    mParentID = (fBaseID_e) 0;
    mSwitchType = type;
    mPlayerNo = -1;

    if (type == 1 || type == 2) {
        if (dActor_c::m_flag_keep[1] != 0 &&
            (dSwitchFlagMng_c::m_instance->mFlags & dActor_c::m_flagbit_keep) != 0) {
            mLightOn = true;
            mPlayerNo = dSwitchFlagMng_c::m_instance->mPlayerNo;
        }
    } else {
        mLightOn = true;
    }

    mParentNo = ACTOR_PARAM_LOCAL(param, ParentNo);
    mParentType = ACTOR_PARAM_LOCAL(param, ParentType);

    return SUCCEEDED;
}

int daLightBase_c::draw() {
    if (mLightOn) {
        drawLight();
    }
    return SUCCEEDED;
}

dActor_c *daLightBase_c::rotation_move() {
    dActor_c *parent = (dActor_c *) fManager_c::searchBaseByID(mParentID);
    if (parent != nullptr) {
        rotation_move(parent);
    }
    return parent;
}

dActor_c *daLightBase_c::setParentInfo() {
    dActor_c *parent = searchParent(mParentType, mParentNo);
    if (parent != nullptr) {
        setParentInfo(parent);
        rotation_move(parent);
    }
    return parent;
}

void daLightBase_c::calcLightOnOff() {
    if (mSwitchType == 1) {
        if ((dSwitchFlagMng_c::m_instance->mFlags & mEventMask) != 0) {
            mLightOn = true;
            mPlayerNo = dSwitchFlagMng_c::m_instance->mPlayerNo;
        }
    } else if (mSwitchType == 2) {
        if ((dSwitchFlagMng_c::m_instance->mFlags & mEventMask) != 0) {
            mLightOn = true;
            mPlayerNo = dSwitchFlagMng_c::m_instance->mPlayerNo;
        } else {
            mLightOn = false;
            mPlayerNo = -1;
        }
    }
}

void daLightBase_c::initializeState_SearchID() {}

void daLightBase_c::finalizeState_SearchID() {}

void daLightBase_c::executeState_SearchID() {
    if (setParentInfo() != nullptr) {
        mStateMgr.changeState(StateID_Move);
    }
}

void daLightBase_c::initializeState_Move() {}

void daLightBase_c::finalizeState_Move() {}

void daLightBase_c::executeState_Move() {
    calcLightOnOff();
}

daLightBase_c::~daLightBase_c() {}
