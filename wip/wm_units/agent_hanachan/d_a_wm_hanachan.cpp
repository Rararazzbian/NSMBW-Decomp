#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/fanm.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <nw4r/math.h>

extern "C" int R_2_1_1994D0(daWmPlayer_c *player);
extern "C" int R_2_1_1994B0(daWmPlayer_c *player);

/// @unofficial DRAFT, first-authoring round for WM_HANACHAN (.text 0x164430-0x165c70, 32
/// functions). Base class confirmed from the constructor (bl __ct__14dWmDemoActor_cFv):
/// daWmHanachan_c : public dWmDemoActor_c. sizeof(daWmHanachan_c) == 0xf00, read directly off
/// classInit (fn_2_164430: li r3, 0xf00; bl __nw__7fBase_cFUl) before authoring anything.
///
/// Member layout read directly from the constructor (fn_2_164460) and cross-checked against the
/// destructor (fn_2_1645E0)'s teardown order (exact mirror, reverse declaration order):
///   dHeapAllocator_c mAllocator      +0x188
///   int mUnk1a4                      +0x1a4  (=0)
///   m3d::mdl_c mModels[4]            +0x1a8  (element size 0x40, __construct_array count 4)
///   m3d::anmChr_c mAnimChrs[4]       +0x2a8  (element size 0x38, count 4)
///   int mUnk388                      +0x388  (=0)
///   m3d::mdl_c mModel2               +0x38c  (single, direct ctor)
///   m3d::anmChr_c mAnimChr2          +0x3cc  (single -- confirmed anmChr_c not fanm_c: the ctor
///                                     runs fanm_c's own ctor first [base sub-object], installing
///                                     fanm_c's vtable, then anmChr_c's own ctor overwrites it with
///                                     __vt__Q23m3d8anmChr_c -- standard base-then-derived ctor
///                                     codegen. The destructor calls __dt__Q23m3d6fanm_cFv
///                                     directly rather than a distinct anmChr_c dtor symbol
///                                     because anmChr_c declares no dtor of its own, so the
///                                     compiler reuses fanm_c's dtor symbol -- not a contradiction.)
///   m3d::smdl_c mSmdls[5]            +0x404  (element size 0xc, count 5)
///   custom struct mTrail[200]        +0x484  (element size 0xc, count 0xc8=200; ctor is a
///                                     REL-external helper fn_2_1D70 outside this unit, paired
///                                     with mVec3_c's real dtor -- almost certainly mVec3_c[200],
///                                     a position-history/trail buffer, not independently
///                                     confirmed)
///   int mUnk1e4 [sic, placeholder]   +0xde4  (=0)
///   HanachanState mStates[5]         +0xde8  (element size 0x38, count 5; ctor fn_2_164590 is a
///                                     bare `blr` -- trivial/empty; dtor fn_2_1645A0 is the
///                                     standard vector-deleting-destructor shape with an empty
///                                     body between prologue and the array-delete check -- POD,
///                                     no owned members. One field confirmed: a 3-float
///                                     (mVec3_c-shaped) field at struct-offset +0x10, read by
///                                     fn_2_165870 as `mPos = mStates[0].<field at +0x10>`.)
///   0xde8 + 5*0x38 == 0xf00 -- closes sizeof exactly, the strongest layout confirmation
///   available this round.
///
/// VTABLE, read directly from the class's own vtable object (lbl_2_data_44E48, 0xc8 bytes) via
/// its own relocations (no per-unit dtk data.txt exists yet since this unit is unlanded, so
/// check_vtable.py has nothing to compare against -- read `dtk rel info -r`'s Absolute
/// relocations sourced from 0x44e48-0x44f10 directly instead of manufacturing a throwaway draft
/// just to generate one). Own-address slots, converted to slot index ((offset-0x8)/4+2) and
/// matched against the recorded lifecycle-adjacency rule (each stage's pre/post hooks sit
/// directly after that stage's own action slot: 2 create/3 preCreate/4 postCreate, 5 doDelete/
/// 6 preDelete/7 postDelete, 8 execute/9 preExecute/10 postExecute, 11 draw/12 preDraw/
/// 13 postDraw):
///   slot 2  (+0x08) -> fn_2_164700 (0x64)  create
///   slot 5  (+0x14) -> fn_2_1648B0 (0x8)   doDelete -- confirmed independently: `li r3,1;blr`,
///                       the family's standard trivial `return SUCCEEDED;` body
///   slot 8  (+0x20) -> fn_2_164770 (0xb0)  execute
///   slot 11 (+0x2c) -> fn_2_164820 (0x84)  draw
///   slot 18 (+0x48) -> fn_2_1645E0 (0x11c) dtor -- confirmed by content: full member teardown
///                       via __destroy_arr on every array member in reverse declaration order,
///                       then the base-class chain (dHeapAllocator_c, smdl_c, mHeapAllocator_c,
///                       dWmActor_c) and the optional fBase_c::operator delete, matching every
///                       other landed sibling's own destructor shape exactly.
class daWmHanachan_c : public dWmDemoActor_c {
public:
    daWmHanachan_c();
    ~daWmHanachan_c();

    /// @unofficial fn_2_164700 (0x64 B). Vtable slot 2 (create). createModel(); mClipSphere.set
    /// (mPos, 250.0f); calcModel(); resetState(); return SUCCEEDED; -- the family's standard
    /// create() shape.
    virtual int create();
    /// @unofficial fn_2_164770 (0xB0 B). Vtable slot 8 (execute). processCutsceneCommand via the
    /// +0x60 double-indirection idiom, then (this->*sStateTable[mState])(), then
    /// resetPosFromState0(), fn_2_164B10(), resetState(), then fn_2_1659A0(f1,f2) with two floats
    /// read from lbl_2_data_44D60+0x54/+0x58 (an already-landed shared constant table, not yet
    /// looked up by name).
    virtual int execute();
    /// @unofficial fn_2_164820 (0x84 B). Vtable slot 11 (draw). mModel2.entry(); then
    /// mModels[i].entry() for i in 0..4; then fn_2_165AB0().
    virtual int draw();
    /// @unofficial fn_2_1648B0 (0x8 B). Vtable slot 5 (doDelete). `li r3,1;blr` -- the family's
    /// trivial default body, declared out-of-line per every landed sibling's own convention.
    virtual int doDelete();

    /// @unofficial fn_2_165090 (0xC B). mState = 1; then tail-calls dWmDemoActor_c::clearSpeedAll().
    void setState1();
    /// @unofficial fn_2_165110 (0xC B). mState = 2; no further call.
    void setState2();
    /// @unofficial fn_2_1651A0 (0xC B). mState = 3; no further call.
    void setState3();
    /// @unofficial fn_2_165870 (0x1C B). mPos = mStates[0].mSomePos (the one confirmed field of
    /// the custom 0x38-byte struct, at struct-offset +0x10).
    void resetPosFromState0();

    /// @unofficial fn_2_1648C0 (0x244 B). createModel(). NOT yet authored -- placeholder body
    /// only, referenced by create().
    void createModel();
    /// @unofficial fn_2_164D10 (0xFC B). calcModel(). NOT yet authored -- placeholder.
    void calcModel();
    /// @unofficial fn_2_164B90 (0x17C B). resetState(). NOT yet authored -- placeholder,
    /// referenced by both create() and execute().
    void resetState();
    /// @unofficial fn_2_164B10 (0x78 B). NOT yet authored -- placeholder, referenced by execute().
    void unkFn164B10();
    /// @unofficial fn_2_164E10 (0x98 B). `mState=0; clearSpeedAll(); mSpeedF=2.5f;
    /// setDirection(getPosVariant2()-mPos); mUnk480=0;`. `mSpeedF` (a `dBaseActor_c` member)
    /// identified from `clearSpeedAll()`'s own already-landed body in
    /// source/dol/bases/d_wm_demo_actor.cpp (`mSpeedF=0.0f` compiles to `stfs ...,0x10c(r3)`,
    /// the exact offset this function writes 2.5f to right after calling clearSpeedAll()).
    void state4WhenNear();
    /// @unofficial fn_2_1659A0 (0x10C B). NOT yet authored -- placeholder, takes two floats.
    void unkFn1659A0(float a, float b);
    /// @unofficial fn_2_165AB0 (0x70 B). NOT yet authored -- placeholder, referenced by draw().
    void unkFn165AB0();

    /// @unofficial fn_2_1657E0 (0x90 B). Returns `daWmMap_c::m_instance->GetPos("W502") +
    /// kOffsetA` (kOffsetA read as (0,0,0) from lbl_2_data_44D60+0x4 in the retail .data --
    /// confirmed by direct file read of original/d_basesNP.rel, not inferred). Computed via
    /// PSVECAdd (psq_l/ps_add/psq_st in the target), not mVec3_c::operator+. Does not read
    /// `this` at all (the incoming this-pointer register is loaded then never touched) --
    /// still an ordinary non-static member in the source, just one whose body happens not to
    /// touch object state.
    mVec3_c getBasePos();
    /// @unofficial fn_2_165770 (0x68 B). getBasePos() + kOffsetB (also (0,0,0) in retail data,
    /// at lbl_2_data_44D60+0x10 -- a DIFFERENT data object from kOffsetA/kOffsetC even though
    /// all three currently hold the same value, evidenced by the three call sites needing
    /// three separate 0xc-byte slots in the target's .data).
    mVec3_c getPosVariant2();
    /// @unofficial fn_2_165700 (0x68 B). getBasePos() + kOffsetC ((0,0,0), lbl_2_data_44D60+0x1c).
    mVec3_c getPosVariant3();
    /// @unofficial fn_2_1655C0 (0xB8 B). Recomputes mPos (+0xac, confirmed against create()'s
    /// own already-matching `lfs f3, 0xac(r31)` read of mPos) from getBasePos(), then mUnk460
    /// from getPosVariant2(), then mUnk46c from
    /// getPosVariant3(), then mUnk454 from a second getBasePos() call (the target really does
    /// call the 1657E0 helper twice from here). Referenced by nothing authored yet this
    /// round -- likely called from resetState() or state0(), not yet confirmed.
    void resetTargetPositions();
    /// @unofficial fn_2_165680 (0x74 B). `for (i = 0; i < 200; i++) mTrail[i] = getPosVariant3();`
    /// -- floods the whole 200-entry trail buffer with the same computed position. Referenced
    /// by nothing authored yet this round.
    void resetTrail();

    /// @unofficial fn_2_164EB0 (0x1D4 B). sStateTable[0]. NOT yet authored -- placeholder.
    void state0();
    /// @unofficial fn_2_1650A0 (0x6C B). sStateTable[1]. NOT yet authored -- placeholder.
    void state1();
    /// @unofficial fn_2_165120 (0x78 B). sStateTable[2]. NOT yet authored -- placeholder.
    void state2();
    /// @unofficial fn_2_1651B0 (0x404 B, largest in the unit). sStateTable[3]. Deliberately left
    /// for last -- placeholder only.
    void state3();

    typedef void (daWmHanachan_c::*StateFunc_t)();
    /// @unofficial lbl_2_rodata_88CC, decoded via the REL's own relocation stream: 4 entries
    /// (0x88CC/0x88D8/0x88E4/0x88F0, each 0xC bytes) resolving to fn_2_164EB0/0x1650A0/
    /// 0x165120/0x1651B0 -- indexed by mState (0..3) in execute().
    static const StateFunc_t sStateTable[4];

    /// @unofficial +0x184. dWmDemoActor_c's own base ends at 0x184 (established fact from
    /// every landed sibling this session); this class's own first member sits there.
    int mUnk184;
    dHeapAllocator_c mAllocator;
    int mUnk1a4;
    m3d::mdl_c mModels[4];
    m3d::anmChr_c mAnimChrs[4];
    int mUnk388;
    m3d::mdl_c mModel2;
    m3d::anmChr_c mAnimChr2;
    m3d::smdl_c mSmdls[5];
    /// @unofficial +0x440. Five RAW POINTERS, not padding -- read off unkFn165AB0's own loop
    /// this round: `lwz r3, 0x440(r31)` with r31 incrementing by 4 each of 5 iterations, so the
    /// field is `T *[5]`, not bytes. Never written by the constructor (the ctor's array-
    /// construction calls jump straight from mSmdls[5] at +0x404 to mTrail[200] at +0x484,
    /// nothing touches +0x440-0x454), so it is zero-initialised only via
    /// `fBase_c::operator new`'s blanket zero, same convention as +0x184's mUnk184. Declared as
    /// `m3d::scnLeaf_c *` -- the loop calls the non-virtual `setPriorityDraw(int,int)` on it
    /// directly (unconditionally, matching `m3d::scnLeaf_c::setPriorityDraw`'s own signature
    /// exactly) and then, only if non-null, dispatches vtable slot 0x14 -- `entry()` -- which is
    /// `scnLeaf_c`'s own slot per the `m3d::mdl_c` vtable dump from earlier this round, so the
    /// base class pointer is sufficient; no more specific derived type is required to reproduce
    /// either call.
    m3d::scnLeaf_c *mUnk440[5];
    /// @unofficial +0x454. THREE consecutive Vec3-shaped fields (0x454, 0x460, 0x46c), read off
    /// fn_2_1655C0's own stores this round -- corrects the previous round's single-float guess
    /// for +0x454/+0x46c (still byte-compatible: `.x` sits at the same offset a bare float did).
    /// state2() only ever reads the `.x` component of the first and third (as a boundary/fence
    /// check), which is why the single-float guess still matched there.
    mVec3_c mUnk454;
    /// @unofficial +0x460. Written by resetTargetPositions() from getPosVariant2().
    mVec3_c mUnk460;
    /// @unofficial +0x46c. Written by resetTargetPositions() from getPosVariant3(). `.x` is the
    /// field state2() compares against mUnk454.x.
    mVec3_c mUnk46c;
    u8 mPad478[0x47c - 0x478];
    /// @unofficial +0x47c. Written by fn_2_165090 (=1, then clearSpeedAll()), fn_2_165110 (=2),
    /// fn_2_1651A0 (=3). A small state machine, name inferred from usage not confirmed.
    int mState;
    /// @unofficial +0x480. Written `=0` by state4WhenNear() only, this round -- not yet known
    /// to be read anywhere. Ctor does not touch it (zero via `fBase_c::operator new`).
    int mUnk480;
    /// @unofficial +0x484. 200-element array, element ctor is a REL-external helper
    /// (fn_2_1D70, outside this unit) paired with mVec3_c's real destructor. Represented here as
    /// a raw mVec3_c array pending independent confirmation of the ctor's role.
    mVec3_c mTrail[200];
    int mUnk1e4;

    /// @unofficial +0xde8. Custom POD struct, element size 0x38, count 5. Trivial default ctor
    /// (`blr`), standard empty vector-deleting destructor. Only one field confirmed: a
    /// 3-float (mVec3_c-shaped) value at struct-offset +0x10.
    struct HanachanState_t {
        /// @unofficial +0x0. Written `=0` by calcModel()'s per-state loop.
        int mUnk0;
        /// @unofficial +0x4. Written from `mUnk460` by calcModel()'s per-state loop.
        mVec3_c mUnk4;
        mVec3_c mSomePos;
        /// @unofficial +0x1c. Written `=true` by calcModel()'s per-state loop.
        bool mUnk1c;
        /// @unofficial +0x20 (natural alignment padding 0x1d-0x20 matches the target's own gap
        /// exactly). Written `=0` by calcModel().
        int mUnk20;
        int mUnk24;
        int mUnk28;
        u8 mPad2c[0x38 - 0x2c];
    };
    HanachanState_t mStates[5];
};

daWmHanachan_c::daWmHanachan_c() : mUnk1a4(0), mUnk388(0), mUnk1e4(0) {}

daWmHanachan_c::~daWmHanachan_c() {}

int daWmHanachan_c::create() {
    createModel();
    mClipSphere.set(mPos, 250.0f);
    calcModel();
    resetState();
    return SUCCEEDED;
}

int daWmHanachan_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    (this->*sStateTable[mState])();
    resetPosFromState0();
    unkFn164B10();
    resetState();
    unkFn1659A0(1.0f, 1.25f);
    return SUCCEEDED;
}

int daWmHanachan_c::draw() {
    mModel2.entry();
    for (int i = 0; i < 4; i++) {
        mModels[i].entry();
    }
    unkFn165AB0();
    return SUCCEEDED;
}

int daWmHanachan_c::doDelete() {
    return SUCCEEDED;
}

void daWmHanachan_c::createModel() {
}

void daWmHanachan_c::unkFn164B10() {
    for (int i = 0; i < 4; i++) {
        mModels[i].play();
    }
    mModel2.play();
}

void daWmHanachan_c::resetState() {
}

void daWmHanachan_c::calcModel() {
    mAngle.y = -0x4000;
    resetTrail();
    resetTargetPositions();
    for (int i = 0; i < 5; i++) {
        mStates[i].mUnk0 = 0;
        mStates[i].mUnk4 = mUnk460;
        mStates[i].mSomePos = mPos;
        mStates[i].mSomePos.x += (float) i * 45.0f;
        mStates[i].mUnk1c = true;
        mStates[i].mUnk20 = 0;
        mStates[i].mUnk24 = 0;
        mStates[i].mUnk28 = 0;
    }
    mScale.x = 4.0f;
    mScale.y = 4.0f;
    mScale.z = 4.0f;
    setState3();
}

void daWmHanachan_c::state4WhenNear() {
    mState = 0;
    clearSpeedAll();
    mSpeedF = 2.5f;
    setDirection(getPosVariant2() - mPos);
    mUnk480 = 0;
}

void daWmHanachan_c::state0() {
}

void daWmHanachan_c::setState1() {
    mState = 1;
    clearSpeedAll();
}

void daWmHanachan_c::state1() {
    daWmPlayer_c *player = daWmPlayer_c::ms_instance;
    if (R_2_1_1994B0(player) == 1 || (player->m_18c && R_2_1_1994D0(player) == 1)) {
        state4WhenNear();
    }
}

void daWmHanachan_c::setState2() {
    mState = 2;
}

void daWmHanachan_c::state2() {
    calcSpeed();
    posMove();
    if (!(mUnk454.x >= mUnk46c.x)) {
        if (mPos.x > mUnk454.x) {
            setState1();
            return;
        }
    }
    if (mUnk454.x >= mUnk46c.x) {
        return;
    }
    if (mPos.x >= mUnk454.x) {
        return;
    }
    setState1();
}

void daWmHanachan_c::setState3() {
    mState = 3;
}

void daWmHanachan_c::state3() {
}

/// @unofficial Three separate named zero-vectors, not one shared constant -- the target's
/// .data has three distinct 0xc-byte slots (lbl_2_data_44D60+0x4/+0x10/+0x1c, confirmed by a
/// direct byte read of original/d_basesNP.rel), even though all three currently hold
/// (0.0f, 0.0f, 0.0f). Declared here, ahead of the whole group, so they pool in this ascending
/// order independent of which of the three functions below is defined first. Typed as
/// nw4r::math::VEC3 (NOT mVec3_c/revolution Vec) so `operator+` resolves to VEC3's own inline
/// paired-single VEC3Add (math_types.h:315) instead of mVec3_c's scalar operator+ (m_vec.hpp)
/// or a real `bl PSVECAdd` -- the lead traced the target's exact psq_l/ps_add/psq_st shape to
/// that inline, confirmed by a landed PSVECAdd caller (source/dol/cLib/c_m3d.cpp) genuinely
/// calling out via `bl` in the matched retail DOL, which rules PSVECAdd itself out.
static const nw4r::math::VEC3 kOffsetA(0.0f, 0.0f, 0.0f);
static const nw4r::math::VEC3 kOffsetB(0.0f, 0.0f, 0.0f);
static const nw4r::math::VEC3 kOffsetC(0.0f, 0.0f, 0.0f);

void daWmHanachan_c::resetTargetPositions() {
    mVec3_c pos1 = getBasePos();
    mPos = pos1;
    mVec3_c pos2 = getPosVariant2();
    mUnk460 = pos2;
    mVec3_c pos3 = getPosVariant3();
    mUnk46c = pos3;
    mVec3_c pos4 = getBasePos();
    mUnk454 = pos4;
}

void daWmHanachan_c::resetTrail() {
    for (int i = 0; i < 200; i++) {
        mTrail[i] = getPosVariant3();
    }
}

mVec3_c daWmHanachan_c::getPosVariant3() {
    mVec3_c base = getBasePos();
    const nw4r::math::VEC3 &baseRef = base;
    return baseRef + kOffsetC;
}

mVec3_c daWmHanachan_c::getPosVariant2() {
    mVec3_c base = getBasePos();
    const nw4r::math::VEC3 &baseRef = base;
    return baseRef + kOffsetB;
}

mVec3_c daWmHanachan_c::getBasePos() {
    mVec3_c pos = daWmMap_c::m_instance->GetPos("W502");
    const nw4r::math::VEC3 &posRef = pos;
    return posRef + kOffsetA;
}

void daWmHanachan_c::resetPosFromState0() {
    mPos = mStates[0].mSomePos;
}

void daWmHanachan_c::unkFn1659A0(float a, float b) {
    for (int i = 0; i < 5; i++) {
        mMatrix.trans(mVec3_c(mStates[i].mSomePos.x, a + mStates[i].mSomePos.y, mStates[i].mSomePos.z));
        mMatrix.ZXYrotM(0, mStates[i].mUnk20, 0);
        mSmdls[i].setLocalMtx(&mMatrix);
        mSmdls[i].setScale(b, 1.0f, b);
        mSmdls[i].calc(false);
    }
}

void daWmHanachan_c::unkFn165AB0() {
    for (int i = 0; i < 5; i++) {
        mUnk440[i]->setPriorityDraw(0x81, 0x7f);
        if (mUnk440[i] != nullptr) {
            mUnk440[i]->entry();
        }
    }
}

const daWmHanachan_c::StateFunc_t daWmHanachan_c::sStateTable[4] = {
    &daWmHanachan_c::state0,
    &daWmHanachan_c::state1,
    &daWmHanachan_c::state2,
    &daWmHanachan_c::state3,
};

ACTOR_PROFILE(WM_HANACHAN, daWmHanachan_c, 0);
