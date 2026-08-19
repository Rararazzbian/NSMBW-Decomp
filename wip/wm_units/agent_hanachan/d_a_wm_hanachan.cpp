#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/fanm.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_player.hpp>

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
    /// @unofficial fn_2_1659A0 (0x10C B). NOT yet authored -- placeholder, takes two floats.
    void unkFn1659A0(float a, float b);
    /// @unofficial fn_2_165AB0 (0x70 B). NOT yet authored -- placeholder, referenced by draw().
    void unkFn165AB0();

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
    /// @unofficial +0x440-0x47c. Gap between mSmdls[5]'s end (0x404+0xc*5=0x440) and mState
    /// (+0x47c, confirmed by fn_2_165090/165110/1651A0's `stw rX, 0x47c(r3)`) and mTrail's start
    /// (+0x484). Not yet identified field-by-field.
    u8 mPad440[0x47c - 0x440];
    /// @unofficial +0x47c. Written by fn_2_165090 (=1, then clearSpeedAll()), fn_2_165110 (=2),
    /// fn_2_1651A0 (=3). A small state machine, name inferred from usage not confirmed.
    int mState;
    u8 mPad480[0x484 - 0x480];
    /// @unofficial +0x484. 200-element array, element ctor is a REL-external helper
    /// (fn_2_1D70, outside this unit) paired with mVec3_c's real destructor. Represented here as
    /// a raw mVec3_c array pending independent confirmation of the ctor's role.
    mVec3_c mTrail[200];
    int mUnk1e4;

    /// @unofficial +0xde8. Custom POD struct, element size 0x38, count 5. Trivial default ctor
    /// (`blr`), standard empty vector-deleting destructor. Only one field confirmed: a
    /// 3-float (mVec3_c-shaped) value at struct-offset +0x10.
    struct HanachanState_t {
        u8 mPad0[0x10];
        mVec3_c mSomePos;
        u8 mPad1[0x38 - 0x10 - 0xc];
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
}

void daWmHanachan_c::resetState() {
}

void daWmHanachan_c::calcModel() {
}

void daWmHanachan_c::state0() {
}

void daWmHanachan_c::setState1() {
    mState = 1;
    clearSpeedAll();
}

void daWmHanachan_c::state1() {
}

void daWmHanachan_c::setState2() {
    mState = 2;
}

void daWmHanachan_c::state2() {
}

void daWmHanachan_c::setState3() {
    mState = 3;
}

void daWmHanachan_c::state3() {
}

void daWmHanachan_c::resetPosFromState0() {
    mPos = mStates[0].mSomePos;
}

void daWmHanachan_c::unkFn1659A0(float a, float b) {
}

void daWmHanachan_c::unkFn165AB0() {
}

const daWmHanachan_c::StateFunc_t daWmHanachan_c::sStateTable[4] = {
    &daWmHanachan_c::state0,
    &daWmHanachan_c::state1,
    &daWmHanachan_c::state2,
    &daWmHanachan_c::state3,
};

ACTOR_PROFILE(WM_HANACHAN, daWmHanachan_c, 0);
