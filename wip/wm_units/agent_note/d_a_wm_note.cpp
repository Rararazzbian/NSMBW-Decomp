#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_w_camera.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/cLib/c_lib.hpp>

/// @unofficial fn_2_172AE0 is a still-undecompiled, REL-local, non-virtual daWmMap_c member
/// (no exported name) living in a not-yet-landed TU. Called as daWmMap_c::m_instance->fn_2_172AE0()
/// in the original (register allocation: r3 = daWmMap_c::m_instance, no other args, int
/// return tested only by storing it and later passing it straight to GetPos(int)), so most likely a
/// "current event/target node index" accessor. Takes two more int args (r4=1, r5=-1 in the
/// target, both set several instructions before the call and never touched again before it --
/// genuinely live, not dead code) whose meaning is unresolved; passed through unchanged.
/// Linked via the REL-internal R_2_1_<offset> convention established by
/// d_a_wm_antlion_mng.cpp / d_a_wm_course.cpp -- a member-call spelling compiles but does not
/// link into an un-landed region of the same REL.
extern "C" int R_2_1_172AE0(daWmMap_c *, int, int);

/// @brief The world map actor for the floating music-note cutscene marker.
/// @details A near-twin of daWmSinkShip_c's minimal shape (see that unit's own doc comment for
/// the shared idioms: the +0x60 secondary vtable pointer, the private per-TU dWmLib::sc_ForceList
/// copy, the staged-locals calcModel()). Unlike sinkship, this class overrides
/// processCutsceneCommand and derives from dWmDemoActor_c DIRECTLY (no dWmObjActor_c in
/// between) -- confirmed from the target vtable (lbl_2_data_467C8, read out of
/// bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.o): GetActorType resolves to the imported
/// GetActorType__14dWmDemoActor_cFv (no dWmObjActor_c-only slot anywhere, unlike sinkship's
/// vf74/vf78), and the vtable is exactly 0x70 bytes / 26 slots (two fewer than sinkship's 0x78 /
/// 28) -- the two dWmObjActor_c-only slots are simply absent. sizeof(daWmNote_c) == 0x1c8, read
/// directly off the allocator wrapper (fn_2_175F90).
///
/// Member offsets read directly from the constructor/destructor (fn_2_175FC0/fn_2_176010,
/// identical shape to sinkship's): dHeapAllocator_c mAllocator @ 0x184 (immediately after
/// dWmDemoActor_c, no intermediate mResNodeIdx), m3d::smdl_c mModel @ 0x1a0. The five new
/// scalar members below run from 0x1ac to 0x1c8 (0x1c8 - 0x1ac == the sum of their sizes with the
/// natural 3-byte tail-alignment after the leading bool -- no unexplained gap).
class daWmNote_c : public dWmDemoActor_c {
public:
    daWmNote_c();
    ~daWmNote_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    /// @unofficial fn_2_176350 (0x224 B, by far this unit's largest function). Dispatches on
    /// cutsceneCommandId (0x1f/0x20/other) with a first-frame-gated setup half and an
    /// every-frame half. See the definition for the per-case breakdown.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    /// @unofficial fn_2_1761C0. Same shape as sinkship's createModel(): fixed archive/model name
    /// "note" (read directly out of the target's own .data, lbl_2_data_467C0), no chr animation.
    void createModel();
    /// @unofficial fn_2_176270. Sets mClipSphere from mPos + a fixed 50.0f radius (read directly
    /// out of the target's own merged rodata pool, lbl_2_rodata_8E80 offset 0) and clears
    /// mIsShown. Called once from create(), right after createModel() and before any calcModel().
    void initClip();
    /// @unofficial fn_2_1762A0. Identical shape to sinkship's calcModel() (staged mVec3_c/mAng3_c
    /// locals, same call sequence).
    void calcModel();

    dHeapAllocator_c mAllocator; ///< This class's OWN allocator, @ 0x184.
    m3d::smdl_c mModel; ///< This class's OWN model, @ 0x1a0.
    bool mIsShown; ///< @unofficial @ 0x1ac. Gates draw()'s mModel.entry() call; cleared by
                   ///< initClip(), set by processCutsceneCommand()'s cutsceneCommandId==0x1f case.
    int mState; ///< @unofficial @ 0x1b0. processCutsceneCommand()'s cutsceneCommandId==0x20
                ///< sub-state: 0 while easing toward mTargetPos, 1 while the post-arrival timer
                ///< counts down.
    int mNodeIdx; ///< @unofficial @ 0x1b4. Result of R_2_1_172AE0(daWmMap_c::m_instance), fed
                  ///< straight into GetPos() to resolve mTargetPos.
    mVec3_c mTargetPos; ///< @unofficial @ 0x1b8. The position addCalcPos() eases mPos toward.
    int mTimer; ///< @unofficial @ 0x1c4. Countdown, in frames; both the 0x1f and 0x20 cases set it
                ///< to 0x3c (60) and set mIsCutEnd directly once it reaches 0 (dWmDemoActor_c::setCutEnd() is NOT called; a plain field write matches the target byte-for-byte, virtual dispatch does not).
    // Total size 0x1c8 (measured, classInit's operand).
};

ACTOR_PROFILE(WM_NOTE, daWmNote_c, 0);

daWmNote_c::daWmNote_c() {}
daWmNote_c::~daWmNote_c() {}

int daWmNote_c::create() {
    createModel();
    initClip();
    return SUCCEEDED;
}

int daWmNote_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmNote_c::draw() {
    if (mIsShown) {
        mModel.entry();
    }
    return SUCCEEDED;
}

int daWmNote_c::doDelete() {
    return SUCCEEDED;
}

void daWmNote_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("note", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("note");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmNote_c::initClip() {
    mIsShown = false;
    mClipSphere.set(mPos, 50.0f);
}

void daWmNote_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmNote_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }

    dWCamera_c *camera = dWCamera_c::m_instance;

    if (isFirstFrame) {
        daWmMap_c *map = daWmMap_c::m_instance;
        daWmPlayer_c *player = daWmPlayer_c::ms_instance;

        switch (cutsceneCommandId) {
        case 0x1f:
            mIsShown = true;
            mPos = player->mPos;
            mPos.y += 50.0f;
            mAngle.y = -0x4000;

            mNodeIdx = R_2_1_172AE0(map, 1, -1);
            mTargetPos = map->GetPos(mNodeIdx);
            mTargetPos.y += 10.0f;
            mTimer = 0x3c;
            break;
        case 0x20: {
            u32 id = mUniqueID;
            u8 *cam = (u8 *) camera;
            *(u32 *) (cam + 0x604) = 1;
            *(u32 *) (cam + 0x5f4) = 0;
            *(u32 *) (cam + 0x5f0) = id;
            *(bool *) (cam + 0x624) = false;
            *(u32 *) (cam + 0x608) = 0;
            mState = 0;
            break;
        }
        }
    }

    switch (cutsceneCommandId) {
    case 0x1f:
        if (mTimer > 0) {
            mTimer--;
        }
        if (mTimer == 0) {
            mIsCutEnd = true;
        }
        break;
    case 0x20:
        switch (mState) {
        case 0: {
            float delta = cLib::addCalcPos(&mPos, mVec3_c(mTargetPos), 0.00800000038f, 10.0f, 2.0f);
            bool arrived = std::fabs(delta) <= 1.19209290e-07f;
            if (arrived) {
                mTimer = 0x3c;
                mState = 1;
            }
            break;
        }
        case 1:
            if (mTimer > 0) {
                mTimer--;
            }
            if (mTimer == 0) {
                mIsCutEnd = true;
            }
            break;
        }
        break;
    default:
        mIsCutEnd = true;
        break;
    }
}

