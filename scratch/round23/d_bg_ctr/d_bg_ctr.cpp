// d_bg_ctr.cpp -- FIRST DRAFT (scratch/round22/d_bg_ctr)
//
// Unit: wiimj2d.dol .text 0x8007F7A0..0x80081070 (dBg_ctr_c).
// Class layout derived from the target disassembly; shadow header in
// shadow/game/bases/d_bg_ctr.hpp.
#include <game/bases/d_bg_ctr.hpp>
#include <game/bases/d_bc.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_base_actor.hpp>

// statics (.sbss)
dBg_ctr_c *dBg_ctr_c::mEntryN;
dBg_ctr_c *dBg_ctr_c::mEntryB;
dActor_c *dBg_ctr_c::mGroupCtrlActor;
int dBg_ctr_c::mGroupCtrlNo;

// helper: read a raw float from an object (dActor_c layout not fully named)
static inline f32 rawF32(const void *p, int off) {
    return *(const f32 *)((const u8 *)p + off);
}

static inline signed char rawS8(const void *p, int off) {
    return *(const signed char *)((const u8 *)p + off);
}

// lookup tables for fn_80080E40 and fn_80080880
extern const u32 lbl_802EFBE0[];
extern const u32 lbl_802EFBF0[];
extern const u32 lbl_802EFBC0[];
extern const u32 lbl_802EFBD0[];

// ---------------------------------------------------------------------------
// ctor / dtor
dBg_ctr_c::dBg_ctr_c() : mScratch() {
    init();
}

dBg_ctr_c::~dBg_ctr_c() {
    release();
}

void dBg_ctr_c::reset() {
    mEntryN = nullptr;
    mEntryB = nullptr;
    mGroupCtrlActor = nullptr;
    mGroupCtrlNo = 0;
}

void dBg_ctr_c::init() {
    mEntryPrev = nullptr;
    mEntryNext = nullptr;
    m_0c = 0;
    m_10 = 0;
    m_14 = 0;
    mLinkNetPlayer[0] = nullptr;
    mWallSlidPlayer[0] = nullptr;
    mLinkNetPlayer[1] = nullptr;
    mWallSlidPlayer[1] = nullptr;
    mLinkNetPlayer[2] = nullptr;
    mWallSlidPlayer[2] = nullptr;
    mLinkNetPlayer[3] = nullptr;
    mWallSlidPlayer[3] = nullptr;
    mEntryFlag = false;
    mGroupNo = -1;
    m_ac.x = 0.0f;
    m_ac.y = 0.0f;
}

void dBg_ctr_c::entry() {
    if (mEntryFlag) {
        return;
    }
    mEntryPrev = mEntryN;
    mEntryN = this;
    if (mEntryPrev == nullptr) {
        mEntryNext = mEntryB;
        mEntryB = this;
    } else {
        mEntryNext = mEntryPrev->mEntryNext;
        mEntryPrev->mEntryNext = this;
    }
    mEntryFlag = true;
}

void dBg_ctr_c::release() {
    if (!mEntryFlag) {
        return;
    }
    if (mEntryPrev != nullptr) {
        mEntryPrev->mEntryNext = mEntryNext;
    } else {
        mEntryB = mEntryNext;
    }
    if (mEntryNext != nullptr) {
        mEntryNext->mEntryPrev = mEntryPrev;
    } else {
        mEntryN = mEntryPrev;
    }
    init();
}

// ---------------------------------------------------------------------------
// set / offset / angle
void dBg_ctr_c::set_common(dActor_c *actor, CallbackF *f, CallbackH *h,
                           CallbackW *w, u8 a, u8 b) {
    mpActor = actor;
    mRotation = (short *)((u8 *)actor + 0x104);
    m_c0 = *(short *)((u8 *)actor + 0x104);
    m_ac.x = 0.0f;
    m_ac.y = 0.0f;
    mCallbackF = f;
    mCallbackH = h;
    mCallbackW = w;
    m_dd = a;
    m_de = b;
    mCheckRevUpper = &dBg_ctr_c::CheckRevUpperSpeed;
    mCheckRevUnder = &dBg_ctr_c::CheckRevUnderSpeed;
    mCheckRevSide = &dBg_ctr_c::CheckRevSideSpeed;
    if (mGroupCtrlActor != actor) {
        mGroupCtrlNo++;
        mGroupCtrlActor = actor;
    }
    mGroupNo = mGroupCtrlNo;
}

void dBg_ctr_c::set(dActor_c *actor, float f1, float f2, float f3, float f4,
                    CallbackF *f, CallbackH *h, CallbackW *w, u8 a, u8 b,
                    mVec3_c *pos) {
    set_common(actor, f, h, w, a, b);
    setOfs(f1, f2, f3, f4, pos);
    mMode = 0;
    calc();
}

void dBg_ctr_c::set(dActor_c *actor, mVec2_c v1, mVec2_c v2, CallbackF *f,
                    CallbackH *h, CallbackW *w, u8 a, u8 b, mVec3_c *pos) {
    set(actor, v1.x, v1.y, v2.x, v2.y, f, h, w, a, b, pos);
}

void dBg_ctr_c::set(dActor_c *actor, const sBgSetInfo *info, u8 a, u8 b,
                    mVec3_c *pos) {
    f32 f4 = info->f4;
    f32 f0 = info->f0;
    f32 fC = info->fC;
    f32 f8 = info->f8;
    set(actor, mVec2_c(f0, f4), mVec2_c(f8, fC),
        (CallbackF *)info->cbF, (CallbackH *)info->cbH, (CallbackW *)info->cbW,
        a, b, pos);
}

void dBg_ctr_c::set_circle(dActor_c *actor, float x, float y, float r,                           CallbackF *f, CallbackH *h, CallbackW *w, u8 a,
                           u8 b) {
    set_common(actor, f, h, w, a, b);
    mCenter.x = x;
    mCenter.y = y;
    mRadius = r;
    mMode = 1;
    calc();
}

void dBg_ctr_c::setOfs(float f1, float f2, float f3, float f4, mVec3_c *pos) {
    mVec3_c v(1.0f, 1.0f, 1.0f);
    if (pos != nullptr) {
        v.x = pos->x;
        v.y = pos->y;
    }
    setOfsX1(f1 * v.x);
    setOfsY1(f2 * v.y);
    setOfsX2(f3 * v.x);
    setOfsY2(f4 * v.y);
}

void dBg_ctr_c::setOfs(mVec2_c v1, mVec2_c v2, mVec3_c *pos) {
    setOfs(v1.x, v1.y, v2.x, v2.y, pos);
}

void dBg_ctr_c::setOfsX1(float f) {
    f32 prev = mCenter.x;
    mCenter.x = f - rawF32(mpActor, 0xd0);
    if (mFlags & 1) {
        m_ac.x = mCenter.x - prev;
    }
}

void dBg_ctr_c::setOfsY1(float f) {
    f32 prev = mCenter.y;
    mCenter.y = f - rawF32(mpActor, 0xd4);
    if (mFlags & 1) {
        m_ac.y = mCenter.y - prev;
    }
}

void dBg_ctr_c::setOfsX2(float f) {
    mOffset2.x = f - rawF32(mpActor, 0xd0);
}

void dBg_ctr_c::setOfsY2(float f) {
    mOffset2.y = f - rawF32(mpActor, 0xd4);
}

void dBg_ctr_c::setAngleY3(short *rot) {
    if (mRotation == rot) {
        return;
    }
    mRotation = rot;
    m_c0 = *rot;
}

// ---------------------------------------------------------------------------
// update / link players
void dBg_ctr_c::update() {
    m_0c = 0;
    m_10 = 0;
    m_14 = 0;
    mLinkNetPlayer[0] = nullptr;
    mWallSlidPlayer[0] = nullptr;
    mLinkNetPlayer[1] = nullptr;
    mWallSlidPlayer[1] = nullptr;
    mLinkNetPlayer[2] = nullptr;
    mWallSlidPlayer[2] = nullptr;
    mLinkNetPlayer[3] = nullptr;
    mWallSlidPlayer[3] = nullptr;
    m_d8 = m_d8 & 1;
    if (mUpdateFlag) {
        mUpdateFlag = 0;
    } else {
        m_c4 = 0;
    }
}

void dBg_ctr_c::updateObjBg() {
    for (dBg_ctr_c *cur = mEntryN; cur != nullptr; cur = cur->mEntryPrev) {
        cur->update();
    }
}

bool dBg_ctr_c::setLinkNetPlayer(dBc_c *player) {
    for (int i = 0; i < 4; i++) {
        if (mLinkNetPlayer[i] == player) {
            return true;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (mLinkNetPlayer[i] == nullptr) {
            mLinkNetPlayer[i] = player;
            return true;
        }
    }
    return false;
}

dBc_c *dBg_ctr_c::getLinkNetPlayer(signed char no) {
    for (int i = 0; i < 4; i++) {
        dBc_c *p = mLinkNetPlayer[i];
        if (p != nullptr && rawS8(p, 0x98) == no) {
            return p;
        }
    }
    return nullptr;
}

bool dBg_ctr_c::setLinkWallSlidPlayer(dBc_c *player) {
    for (int i = 0; i < 4; i++) {
        if (mWallSlidPlayer[i] == player) {
            return true;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (mWallSlidPlayer[i] == nullptr) {
            mWallSlidPlayer[i] = player;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// rev checks
bool dBg_ctr_c::upperRevCheck(dActor_c *other) {
    if (mFlags & (1 << 19)) {
        return true;
    }
    if (mCheckRevUpper == nullptr) {
        return false;
    }
    return mCheckRevUpper(mpActor, other);
}

bool dBg_ctr_c::underRevCheck(dActor_c *other) {
    if (mFlags & (1 << 20)) {
        return true;
    }
    if (mCheckRevUnder == nullptr) {
        return false;
    }
    return mCheckRevUnder(mpActor, other);
}

bool dBg_ctr_c::sideRevCheck(dActor_c *other, u8 dir) {
    if (mFlags & (1 << 21)) {
        return true;
    }
    if (mCheckRevSide == nullptr) {
        return false;
    }
    return mCheckRevSide(mpActor, other, dir);
}

bool dBg_ctr_c::CheckRevUpperSpeed(dActor_c *a, dActor_c *b) {
    (void)b;
    return rawF32(a, 0xec) > 0.0f;
}

bool dBg_ctr_c::CheckRevUnderSpeed(dActor_c *a, dActor_c *b) {
    (void)b;
    return rawF32(a, 0xec) < 0.0f;
}

bool dBg_ctr_c::CheckRevSideSpeed(dActor_c *a, dActor_c *b, u8 dir) {
    (void)b;
    if (dir == 0) {
        if (rawF32(a, 0xe8) > 0.0f) {
            return true;
        }
    } else if (rawF32(a, 0xe8) < 0.0f) {
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// large functions
void dBg_ctr_c::calc() {}
void dBg_ctr_c::revisePos() {}
void dBg_ctr_c::addDokanMoveDiff(mVec3_c *) {}
bool dBg_ctr_c::fn_80080E40(dBc_c *, u8, u8) { return false; }
bool dBg_ctr_c::fn_80080880(f32, f32, f32) { return false; }
bool dBg_ctr_c::fn_80080900(mVec3_c *, short *, int) { return false; }
bool dBg_ctr_c::fn_80080670(mVec3_c *, f32) { return false; }
void fn_8007FFA0(dBg_ctr_c *, dActor_c *, mVec3_c *, int) {}

int dBg_ctr_c::checkRevisionState(ulong flags) {
    if (flags & 0xA0000000) {
        return 0;
    }
    if ((flags & 0x40000000) && (mFlags & 0x800)) {
        return 0;
    }
    return 1;
}
