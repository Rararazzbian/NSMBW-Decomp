#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_actor_manager.hpp>

ACTOR_PROFILE(EN_HATENA_BALLOON, daEnHatenaBalloon_c, 0x46);

// ---------------------------------------------------------------- preDraw
int daEnHatenaBalloon_c::preDraw() {
    if (m_822 != 0) {
        if (mBalloonType == 0) {
            dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
            if (player == nullptr || !player->isStatus(0x53)) {
                deleteActor(1);
                return dActor_c::preDraw();
            }
            mVec3_c pos(mPos);
            pos.y += 6.0f;
            pos.z -= 64.0f;
            player->setDrawBalloonInPlayer(pos);
        } else {
            mVec3_c pos(mPos);
            pos.y += 8.0f;
            pos.z -= 64.0f;
            item_draw_calc(&pos);
        }
    }
    return dActor_c::preDraw();
}

// ---------------------------------------------------------------- draw
int daEnHatenaBalloon_c::draw() {
    if (mBalloonType == 1) {
        mItemModel.entry();
    }

    mMtx_c nodeMtx;
    mMtx_c mtx;
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;

    // The original compares against a float that is NOT a bare literal -- a bare
    // 0.0f makes CodeWarrior canonicalise the constant into fcmpu's FIRST operand,
    // and the target has it second. Binding a const reference keeps the source
    // operand order without allocating an extra FPR (a plain `float zero = 0.0f;`
    // local matches instruction-for-instruction but rotates f0/f1/f2 in the mPos
    // copy above). The `!(a != b)` double negation, materialised through `isZero`,
    // is what produces the mfcr/extrwi/xori/cntlzw/srwi. sequence; folding it to
    // `if (m_7c4 == zero)` collapses to a bare fcmpu/beq and loses 5 instructions.
    float shake = m_7c4;
    const f32 &zero = 0.0f;
    bool isZero = !(m_7c4 != zero);
    if (isZero) {
        shake = m_7c8;
    }
    pos.x += shake;
    pos.y += 16.0f;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);
    mMatrix.ZrotM(angle.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
    mModel.getNodeWorldMtx(0, &nodeMtx);
    // set() -- not three field assignments: the argument list is evaluated
    // right-to-left, which hoists all three loads above all three stores.
    m_78c.set(nodeMtx.m[0][3], nodeMtx.m[1][3], nodeMtx.m[2][3]);
    mModel.entry();

    pos.z = m_7d0 - 64.0f;
    mtx.trans(pos.x, pos.y, pos.z);
    mtx.YrotM(angle.y);
    mtx.XrotM(angle.x);
    mtx.ZrotM(angle.z);
    mBalloonModel.setLocalMtx(&mtx);
    mBalloonModel.setScale(mScale);
    mBalloonModel.calc(false);
    mBalloonModel.entry();

    mLightMask.draw();

    return 1;
}

// ---------------------------------------------------------------- model_set
void daEnHatenaBalloon_c::model_set() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("balloon", "g3d/balloon.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("balloon");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);
    mModel.setPriorityDraw(-1, 0x98);

    nw4r::g3d::ResAnmChr anmChr = mResFile.GetResAnmChr("float");
    mAnmChr.create(mdl, anmChr, &mAllocator);
    mAnmChr.setAnm(mModel, anmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 1.0f);

    nw4r::g3d::ResAnmTexSrt anmTexSrt = mResFile.GetResAnmTexSrt("float");
    mAnmTexSrt.create(mdl, anmTexSrt, &mAllocator);
    mAnmTexSrt.setAnm(mModel, anmTexSrt, 0, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmTexSrt, 1.0f);

    nw4r::g3d::ResAnmTexPat anmTexPat = mResFile.GetResAnmTexPat("balloon");
    mAnmTexPat.create(mdl, anmTexPat, &mAllocator);
    mAnmTexPat.setAnm(mModel, anmTexPat, 0, m3d::FORWARD_ONCE);
    mModel.setAnm(mAnmTexPat);

    float frame = 0.0f;
    if (dActorMng_c::m_instance->envAllWaterCheck()) {
        frame = 1.0f;
    }
    mAnmTexPat.setFrame(frame, 0);
    mAnmTexPat.setRate(0.0f, 0);

    nw4r::g3d::ResMdl backMdl = mResFile.GetResMdl("balloon_back");
    mBalloonModel.create(backMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);

    nw4r::g3d::ResAnmChr backAnmChr = mResFile.GetResAnmChr("float_back");
    mBalloonAnmChr.create(backMdl, backAnmChr, &mAllocator);
    mBalloonAnmChr.setAnm(mBalloonModel, backAnmChr, m3d::FORWARD_LOOP);
    mBalloonModel.setAnm(mBalloonAnmChr, 1.0f);

    nw4r::g3d::ResAnmTexPat backAnmTexPat = mResFile.GetResAnmTexPat("balloon_back");
    mBalloonAnmTexPat.create(backMdl, backAnmTexPat, &mAllocator);
    mBalloonAnmTexPat.setAnm(mBalloonModel, backAnmTexPat, 0, m3d::FORWARD_ONCE);
    mBalloonModel.setAnm(mBalloonAnmTexPat);

    float backFrame = 0.0f;
    if (dActorMng_c::m_instance->envAllWaterCheck()) {
        backFrame = 1.0f;
    }
    mBalloonAnmTexPat.setFrame(backFrame, 0);
    mBalloonAnmTexPat.setRate(0.0f, 0);

    if (mBalloonType == 1) {
        nw4r::g3d::ResFile itemRes = dResMng_c::m_instance->getRes("I_kinoko", "g3d/I_kinoko.brres");
        nw4r::g3d::ResMdl itemMdl = itemRes.GetResMdl("I_kinoko");
        mItemModel.create(itemMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);
        dActor_c::setSoftLight_Item(mItemModel);

        nw4r::g3d::ResAnmChr itemAnmChr = itemRes.GetResAnmChr("wait");
        mItemAnmChr.create(itemMdl, itemAnmChr, &mAllocator);
        mItemAnmChr.setAnm(mItemModel, itemAnmChr, m3d::FORWARD_LOOP);
        mItemModel.setAnm(mItemAnmChr, 1.0f);
        mItemAnmChr.setRate(0.5f);
    }

    mAllocator.adjustFrmHeap();
}

// ---------------------------------------------------------------- item_draw_calc
void daEnHatenaBalloon_c::item_draw_calc(mVec3_c *inPos) {
    mVec3_c pos = *inPos;
    mVec3_c scale = mItemDrawPos;
    mAng3_c angle(0, 0, 0);

    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.concat(mMtx_c::createTrans(0.0f, 8.0f, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -8.0f, 0.0f));

    mItemModel.setLocalMtx(&mMatrix);
    mItemModel.setScale(scale);
    mItemModel.calc(false);
}

// ---------------------------------------------------------------- anm_set
static const char *l_anm_name[] = {"float", "vibrate"};
static const char *l_anm_name_back[] = {"float_back", "vibrate_back"};

void daEnHatenaBalloon_c::anm_set(int anmNo) {
    nw4r::g3d::ResAnmChr anmChr = mResFile.GetResAnmChr(l_anm_name[anmNo]);
    mAnmChr.setAnm(mModel, anmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 1.0f);
    mAnmChr.setRate(1.0f);

    nw4r::g3d::ResAnmChr backAnmChr = mResFile.GetResAnmChr(l_anm_name_back[anmNo]);
    mBalloonAnmChr.setAnm(mBalloonModel, backAnmChr, m3d::FORWARD_LOOP);
    mBalloonModel.setAnm(mBalloonAnmChr, 1.0f);
    mBalloonAnmChr.setRate(1.0f);

    mAnmNo = anmNo;
}
