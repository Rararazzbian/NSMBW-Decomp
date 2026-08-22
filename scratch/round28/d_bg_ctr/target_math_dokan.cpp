// d_bg_ctr.cpp -- ROUND 25 DRAFT
//
// Unit: wiimj2d.dol .text 0x8007F7A0..0x80081070 (dBg_ctr_c).
// Class layout derived from the target disassembly; shadow header in
// shadow/game/bases/d_bg_ctr.hpp.
#include <game/bases/d_bg_ctr.hpp>
#include <game/bases/d_bc.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_base_actor.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/math/eggMath.h>
#include <lib/revolution/MTX/vec.h>
#include <lib/revolution/MTX/mtx.h>
#include <nw4r/math/math_triangular.h>

// Proposed nw4r geometry declarations (not yet in shared headers).
// See scratch/round24/proposed_nw4r_geometry.hpp for the full proposal.
namespace nw4r { namespace math {
struct SEGMENT3 { VEC3 start; VEC3 end; };
struct SPHERE { VEC3 center; f32 radius; };
bool IntersectionSegment3Sphere(const SEGMENT3 *, const SPHERE *, f32 *, f32 *);
f32 DistSqSegment3ToSegment3(const SEGMENT3 *, const SEGMENT3 *, f32 *, f32 *);
extern s16 Atan2Idx(f32 y, f32 x);
}}

// getActorKind is not declared in the dBc_c header.
// It returns a u32 (consumed as clrlwi + cmplwi in the target).
namespace { u32 getActorKind(dBc_c *bc); }

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
static const u32 lbl_802EFBC0[4] = {2, 1, 0, 3};
static const u32 lbl_802EFBD0[4] = {0, 3, 2, 1};
static const u32 lbl_802EFBE0[4] = {0x80000000, 0x80000000, 0x40000000, 0x20000000};
static const u32 lbl_802EFBF0[4] = {0x00000000, 0x00000000, 0x10000000, 0x08000000};

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

void dBg_ctr_c::set_circle(dActor_c *actor, float x, float y, float r,
                           CallbackF *f, CallbackH *h, CallbackW *w, u8 a,
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
// fn_80080880 (32w) -- range check with rotation-based scratch indexing
bool dBg_ctr_c::fn_80080880(f32 f1, f32 f2, f32 f3) {
    if (f2 < f1) {
        f32 tmp = f2;
        f2 = f1;
        f1 = tmp;
    }

    int idx = ((u32)*mRotation >> 14) & 3;

    if (f3 + mScratch[lbl_802EFBC0[idx]].x < f1) {
        return false;
    }
    return !(f3 + mScratch[lbl_802EFBD0[idx]].x > f2);
}

// ---------------------------------------------------------------------------
// fn_8007FFA0 (115w) -- static helper called by revisePos
static void fn_8007FFA0(dBg_ctr_c *ctr, dActor_c *actor, mVec3_c *vec, int mode) {
    f32 stack_x = 0.0f;
    f32 stack_y = 0.0f;

    mVec3_c *pos = *(mVec3_c **)((u8 *)actor + 0x14);

    if (!(ctr->mFlags & 0x40000000)) {
        f32 f2 = vec->x;
        if (mode == 0) {
            f2 *= rawF32(actor, 0xD8);
        }
        stack_x += f2;
        *(f32 *)((u8 *)actor + 0xDC) = vec->x;
    }

    stack_y += vec->y;

    if (ctr->m_c2 != 0) {
        mVec3_c *src;
        if (mode == 2) {
            src = (mVec3_c *)((u8 *)actor + 0x3C);
        } else {
            src = (mVec3_c *)((u8 *)actor + 0x34);
        }

        f32 sqrtVal = EGG::Math<f32>::sqrt(src->x * src->x + src->y * src->y);
        s16 angle = (s16)(nw4r::math::Atan2Idx(src->y, src->x) + ctr->m_c2);

        f32 cos = nw4r::math::CosIdx(angle);
        stack_x += sqrtVal * cos - src->x;

        f32 sin = nw4r::math::SinIdx(angle);
        stack_y += sqrtVal * sin - src->y;
    }

    if (mode == 2 && stack_x == 0.0f) {
        stack_y = 0.0f;
    }

    pos->x = dScStage_c::getLoopPosX(pos->x + stack_x);
    pos->y += stack_y;

    if (mode == 0) {
        dBase_c *parent = *(dBase_c **)((u8 *)actor + 0x04);
        if (parent != nullptr) {
            *(f32 *)((u8 *)parent + 0x310) += stack_x;
            *(f32 *)((u8 *)parent + 0x314) += stack_y;
        }
    }
}

// ---------------------------------------------------------------------------
// revisePos (72w) -- iterates linked list and player arrays, calls fn_8007FFA0
void dBg_ctr_c::revisePos() {
    dActor_c *actor = mpActor;

    f32 f0 = rawF32(this, 0x9C);
    f32 f1 = rawF32(actor, 0xB4);
    f32 f3 = rawF32(actor, 0xB0);
    f32 f4 = f1 - f0;
    f32 f2 = rawF32(this, 0x98);
    f1 = rawF32(actor, 0xAC);
    f0 = rawF32(this, 0x94);
    f2 = f3 - f2;
    f1 = f1 - f0;

    mVec3_c delta;
    delta.y = f2;
    delta.x = f1;
    delta.z = f4;

    delta.x += m_ac.x;
    delta.y += m_ac.y;

    for (u8 *node = (u8 *)m_0c; node != nullptr; node = *(u8 **)(node + 0x6C)) {
        fn_8007FFA0(this, (dActor_c *)node, &delta, 0);
    }

    for (int i = 0; i < 4; i++) {
        dBc_c *player = mLinkNetPlayer[i];
        if (player != nullptr) {
            fn_8007FFA0(this, (dActor_c *)player, &delta, 1);
        }
    }

    for (int i = 0; i < 4; i++) {
        dBc_c *player = mWallSlidPlayer[i];
        if (player != nullptr) {
            fn_8007FFA0(this, (dActor_c *)player, &delta, 2);
        }
    }

    m_ac.x = 0.0f;
    m_ac.y = 0.0f;
}

// ---------------------------------------------------------------------------
// calc (125w) -- main update: sets mUpdateFlag, computes rotated rect corners
void dBg_ctr_c::calc() {
    mUpdateFlag = 1;

    mVec3_c centerPos = mpActor->getCenterPos();
    dScStage_c::getLoopPosX(centerPos.x);

    mPos.x = centerPos.x;
    mPos.y = centerPos.y;

    s16 rot = *mRotation;
    s16 diff = rot - m_c0;
    m_c2 = diff;
    m_c4 = diff;

    m_a8 = m_a0;

    if (mMode == 1) {
        mPos.x += mCenter.x;
        mPos.y += mCenter.y;

        m_a0 = mPos;
    } else {
        f32 cos = nw4r::math::CosIdx(rot);
        f32 sin = nw4r::math::SinIdx(rot);

        f32 cx = mCenter.x;
        f32 cy = mCenter.y;
        f32 ox = mOffset2.x;
        f32 oy = mOffset2.y;
        f32 px = mPos.x;
        f32 py = mPos.y;

        f32 cxc = cx * cos;
        f32 cxs = cx * sin;
        f32 cyc = cy * cos;
        f32 cys = cy * sin;
        f32 oxc = ox * cos;
        f32 oxs = ox * sin;
        f32 oyc = oy * cos;
        f32 oys = oy * sin;

        mScratch[0].x = px + (cxc - cys);
        mScratch[0].y = py + (cyc + cxs);

        mScratch[1].x = px + (cxc - oys);
        mScratch[1].y = py + (oyc + cxs);

        mScratch[2].x = px + (oxc - oys);
        mScratch[2].y = py + (oyc + oxs);

        mScratch[3].x = px + (oxc - cys);
        mScratch[3].y = py + (cyc + oxs);

        m_a0.x = (mScratch[0].x + mScratch[2].x) * 0.5f;
        m_a0.y = (mScratch[0].y + mScratch[2].y) * 0.5f;
    }

    revisePos();

    *(f32 *)((u8 *)this + 0x94) = *(const f32 *)((const u8 *)mpActor + 0xAC);
    *(f32 *)((u8 *)this + 0x98) = *(const f32 *)((const u8 *)mpActor + 0xB0);
    *(f32 *)((u8 *)this + 0x9C) = *(const f32 *)((const u8 *)mpActor + 0xB4);

    m_c0 = rot;
}

// ---------------------------------------------------------------------------
// addDokanMoveDiff (87w) -- computes dokan lift movement
void dBg_ctr_c::addDokanMoveDiff(mVec3_c *out) {
    if (m_c4 == 0) return;

    f32 dy = out->y - mPos.y;
    f32 dx = out->x - mPos.x;
    f32 len = EGG::Math<f32>::sqrt(dx * dx + dy * dy);
    s16 angle = (s16)(nw4r::math::Atan2Idx(dy, dx) + m_c4);
    f32 scale = 0.00390625f;
    f32 cos = nw4r::math::CosFIdx((f32)angle * scale);
    f32 sin = nw4r::math::SinFIdx((f32)angle * scale);
    f32 corrected_y = len * sin - dy;
    f32 corrected_x = len * cos - dx;
    out->x = dScStage_c::getLoopPosX(mPos.x + len * nw4r::math::CosFIdx((f32)angle * scale));
    out->y = mPos.y + len * nw4r::math::SinFIdx((f32)angle * scale);
}

// ---------------------------------------------------------------------------
// fn_80080E40 (121w) -- collision filter with lookup tables
bool dBg_ctr_c::fn_80080E40(dBc_c *bc, u8 dir, u8 idx) {
    if (m_d4 != 0) {
        return false;
    }

    if (!(*(const u8 *)((const u8 *)bc + 0xE5) & m_dd)) {
        return false;
    }

    if (bc->mpOwner == mpActor) {
        return false;
    }
    if (bc->mpOwner == (dActor_c *)m_38) {
        return false;
    }

    if (bc->mpNoHitActor == mpActor) {
        return false;
    }

    u8 *typePtr = *(u8 **)((const u8 *)bc + 0xE8);
    if (*typePtr != m_de) {
        return false;
    }

    if (mFlags & lbl_802EFBE0[idx]) {
        return false;
    }

    u32 kind = getActorKind(bc);
    if (kind == 1 || kind == 2) {
        if (mFlags & lbl_802EFBF0[idx]) {
            return false;
        }
    } else {
        if (mFlags & (1 << 26)) {
            return false;
        }
    }

    switch (mFlags2) {
    case 1:
        if (dir & (1 << 6)) {
            return false;
        }
        break;
    case 2:
        if (dir & (1 << 7)) {
            return false;
        }
        break;
    case 3:
        if ((dir & (1 << 28)) || (dir & (1 << 8))) {
            return false;
        }
        break;
    case 4:
        if (dir & (1 << 9)) {
            return false;
        }
        break;
    }

    if ((mFlags & (1 << 11)) && (dir & (1 << 10))) {
        return false;
    }
    if ((mFlags & (1 << 12)) && (dir & (1 << 11))) {
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// fn_80080900 (256w) -- segment-vs-shape intersection test
bool dBg_ctr_c::fn_80080900(mVec3_c *segment, short *angleOut, int flags) {
    if (!mpActor) {
        return false;
    }

    if (mMode == 1) {
        nw4r::math::SPHERE sphere;
        sphere.center.x = mPos.x;
        sphere.center.y = mPos.y;
        sphere.center.z = segment[0].z;
        sphere.radius = mRadius;

        f32 t;
        f32 unused;

        if (!nw4r::math::IntersectionSegment3Sphere(
                reinterpret_cast<const nw4r::math::SEGMENT3 *>(segment),
                &sphere, &t, &unused)) {
            return false;
        }

        mVec3_c dir(
            segment[1].x - segment[0].x,
            segment[1].y - segment[0].y,
            segment[1].z - segment[0].z);

        segment[1].x = segment[0].x + dir.x * t;
        segment[1].y = segment[0].y + dir.y * t;
        segment[1].z = segment[0].z + dir.z * t;

        if (angleOut) {
            mVec3_c toHit(
                segment[1].x - sphere.center.x,
                segment[1].y - sphere.center.y,
                segment[1].z - sphere.center.z);

            *angleOut = cM::atan2s(toHit.x, -toHit.y) + 0x8000;
        }

        return true;
    }

    // Rect mode
    mVec3_c *seg = segment;
    short *angle = angleOut;
    int flg = flags;

    f32 segStartX = seg[0].x;
    f32 segEndX = seg[1].x;
    f32 segZ = seg[0].z;

    // 4 corners copied to stack — the compiler keeps a pointer (like r27)
    mVec3_c corners[4];
    mVec3_c *pCorners = corners;
    for (int i = 0; i < 4; i++) {
        corners[i].x = mScratch[i].x;
        corners[i].y = mScratch[i].y;
        corners[i].z = segZ;
    }

    if (segStartX != segEndX) {
        flg = 0;
    }

    int cornerA = 0;
    int cornerB = 0;
    if (flg == 1) {
        s16 rot = *mRotation;
        int base = ((u32)(-rot) >> 14) & 3;

        if (seg[0].y < seg[1].y) {
            base = (base + 2) & 3;
        }

        cornerA = base;
        cornerB = (base - 1) & 3;
    }

    // edge segment and result on stack
    nw4r::math::SEGMENT3 edgeSeg;
    f32 distSqResult;
    f32 *retDistSq = &distSqResult;
    int found = 0;

    for (int i = 0; i < 4; i++) {
        if (flg != 0) {
            u8 fi = (u8)i;
            if (fi != (u8)cornerA && fi != (u8)cornerB) {
                continue;
            }
        }

        int next = (i + 1) & 3;
        edgeSeg.start = pCorners[i];
        edgeSeg.end = pCorners[next];

        f32 d = nw4r::math::DistSqSegment3ToSegment3(
                reinterpret_cast<const nw4r::math::SEGMENT3 *>(seg),
                &edgeSeg, retDistSq, NULL);

        if (d >= 0.0f) {
            continue;
        }

        f32 t = *retDistSq;

        mVec3_c dir(
            seg[1].x - seg[0].x,
            seg[1].y - seg[0].y,
            seg[1].z - seg[0].z);

        seg[1].x = seg[0].x + dir.x * t;
        seg[1].y = seg[0].y + dir.y * t;
        seg[1].z = seg[0].z + dir.z * t;

        if (angle) {
            f32 edgeDirX = edgeSeg.end.x - edgeSeg.start.x;
            f32 edgeDirY = edgeSeg.end.y - edgeSeg.start.y;

            *angle = cM::atan2s(edgeDirX, -edgeDirY) + 0x4000;
        }

        found = 1;
    }

    return found != 0;
}

// ---------------------------------------------------------------------------
// fn_80080670 (130w) -- point-vs-shape test (circle or rect)
bool dBg_ctr_c::fn_80080670(mVec3_c *pos, float f1) {
    if (!mpActor) {
        return false;
    }

    if (mMode == 1) {
        f32 cx = f1 + mPos.x;
        f32 cy = mPos.y;

        Vec diff;
        diff.x = cx - pos->x;
        diff.y = cy - pos->y;
        diff.z = -pos->z;

        f32 mag = PSVECMag(&diff);
        if (mag <= mRadius) {
            return true;
        }
        return false;
    }

    // Rect mode
    short rot = *mRotation;
    if (rot == 0) {
        if (pos->y > mScratch[0].y) return false;
        if (pos->y < mScratch[2].y) return false;
        if (pos->x > f1 + mScratch[2].x) return false;
        if (pos->x < f1 + mScratch[0].x) return false;
        return true;
    }

    // Rotated rect
    f32 centerX = f1 + mScratch[0].x;
    f32 centerY = mScratch[0].y;

    mMtx_c rotMtx;
    rotMtx.ZrotS(mAng(-rot));

    mMtx_c transMtx;
    PSMTXTrans(transMtx, pos->x - centerX, pos->y - centerY, pos->z);

    PSMTXConcat(rotMtx, transMtx, rotMtx);

    nw4r::math::VEC3 result;
    rotMtx.multVecZero(result);

    if (result.x < 0.0f) return false;
    if (result.x > mOffset2.x - mCenter.x) return false;
    if (result.y > 0.0f) return false;
    if (result.y < mOffset2.y - mCenter.y) return false;

    return true;
}

// ---------------------------------------------------------------------------
// checkRevisionState
int dBg_ctr_c::checkRevisionState(ulong flags) {
    if (flags & 0xA0000000) {
        return 0;
    }
    if ((flags & 0x40000000) && (mFlags & 0x800)) {
        return 0;
    }
    return 1;
}