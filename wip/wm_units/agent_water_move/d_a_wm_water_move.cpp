#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/framework/f_manager.hpp>
#include <game/bases/d_actor_state.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_water_entry_manager.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/sLib/s_State.hpp>
#include <lib/nw4r/math/math_triangular.h>
#include <lib/egg/core/eggHeap.h>
#include <game/bases/d_game_com.hpp>

// AC_WATER_MOVE + AC_WATER_MOVE_REGULAR -- ONE class, two classInit entry points.
// .text 0x152010-0x1530E0 (0x10D0 bytes), .ctors 0x394 -> __sinit at 0x152CE0.
//
// ONE-CLASS-OR-TWO, settled two independent ways:
// 1. DIRECT BYTES: fn_2_152010 (AC_WATER_MOVE's classInit) and fn_2_1520B0
//    (AC_WATER_MOVE_REGULAR's) are byte-identical apart from their own addresses --
//    same allocation size (0x4c0), same base-ctor call, same vtable patched in
//    (lbl_2_data_421C0), same dHeapAllocator_c/m3d::mdl_c/m3d::anmTexSrt_c construction
//    sequence at the same offsets. Two DISTINCT compiled functions constructing the
//    SAME class, matching the ACTOR_PROFILE idiom seen on the AC_* switch family.
// 2. COMPILE PROBE (probe_oneclass.cpp, this directory): a placeholder class shared by
//    two classInit stubs compiles to ONE local vtable object, `__vt__11daWmProbe_c`,
//    referenced by both -- confirms the shape independent of trusting the coordinator.
// The class's own vtable (lbl_2_data_421C0, dumped fresh in
// target_auto_04_0003A960_data.txt:9398-9506) carries THREE overrides cross-checked
// directly against include/game/framework/f_base.hpp's own declaration order:
//   create()  slot2/offset 0x08 -> fn_2_152150 (0x244 bytes)
//   execute() slot8/offset 0x20 -> fn_2_1523A0 (0xD8 bytes)  [doDelete/draw ALSO overridden,
//                                                              see class body below]
//   ~daWaterMove_c() slot18/offset 0x48 -> fn_2_152C60 (one-slot flag-arg shape, same ABI
//                                           d_a_dummy_door.cpp already established)
// plus doDelete() (fn_2_1524B0) and draw() (fn_2_152480), both real overrides at their
// own f_base.hpp-declared slots. Everything else in the 56-entry vtable proper resolves to
// plain inherited dActor_c/dBaseActor_c/fBase_c names -- confirmed by reading the vtable
// dump directly, not by slot arithmetic alone.
//
// THREE states, non-virtual (STATE_FUNC_DECLARE, matching source/d_basesNP/bases/
// d_a_wm_sandpillar.cpp's own idiom, the smallest already-landed STATE_DEFINE user).
// Names read directly off the vtable object's own trailing string-literal table (decoded
// byte-for-byte from the raw `.4byte` words, not inferred): "daWaterMove_c::StateID_Wait",
// "...StateID_Udmove", "...StateID_Lrmove". The 9 preceding (0,0xFFFFFFFF,fn) triples are
// NOT part of the vtable proper -- they are the compiler's own PowerPC "member function
// pointer, non-virtual" 3-word encoding for STATE_DEFINE's three constructor arguments
// (initialize/execute/finalize), one per state; __sinit (fn_2_152CE0) reads them straight
// out of this same pool to construct the three sFStateID_c<daWaterMove_c> statics living in
// lbl_2_bss_F5C0 (+0x10/+0x50/+0x90 -- the exact addresses create()'s own changeState-style
// dispatch passes to mStateMgr). sFStateID_c<daWaterMove_c> itself (lbl_2_data_42368, the
// second referenced .data object) is the compiler's OWN template instantiation of
// include/game/sLib/s_FStateID.hpp -- not a hand-written class: fn_2_152F60/152FC0 are its
// destructor/isSameName (both declared right there in the template), and fn_2_153050/
// 153080/1530B0 are its initializeState/executeState/finalizeState, each a bare
// `__ptmf_scall` trampoline calling through the stored `stateFunc` pointer-to-member. Using
// STATE_FUNC_DECLARE/STATE_DEFINE exactly as sandpillar does reproduces ALL of this via
// ordinary template instantiation; nothing here is hand-authored.
//
// Member layout: sizeof(daWaterMove_c) == 0x4c0, read directly off both classInits' own
// `li r3, 0x4c0`. dActorState_c itself is 0x3d0 (established empirically on the AC_switch
// unit: an empty class deriving it with no added members allocates exactly 0x3d0). The
// first 0x3d0 bytes are therefore entirely inherited (dActor_c/dBaseActor_c's own mPos
// (0xac), mScale (0xdc), mAngle (0x100) match create()/calcModel()'s own offsets exactly,
// confirmed against include/game/bases/d_base_actor.hpp -- no need to add placeholder
// fields for any of them). daWaterMove_c's OWN added members start at +0x3d4 (a 4-byte gap
// after dActorState_c, unidentified -- reserved padding, not a guess dressed up as a name):
// a dHeapAllocator_c, a zero-initialised nw4r::g3d::ResFile (matches ResCommon<T>'s own
// pointer-only layout, no ctor call needed, just zeroed -- see
// include/lib/nw4r/g3d/res/g3d_rescommon.h), an m3d::mdl_c, and an m3d::anmTexSrt_c
// immediately followed by an mAllocator_c -- same idiom, same construction order, as
// source/d_basesNP/bases/d_a_wm_sandpillar.cpp's own mModel/mAnimTexSrt pair. The
// intermediate `__vt__Q23m3d6banm_c` store inside classInit is NOT a separate member: it is
// the ordinary base-then-derived vtable-pointer sequence for constructing m3d::anmTexSrt_c
// (banm_c's own vtable during base construction, overwritten by anmTexSrt_c's own moments
// later) -- confirmed against include/game/mLib/m_3d/anm_tex_srt.hpp
// (`class anmTexSrt_c : public banm_c`).
//
// Fields past the sub-objects (0x464 onward) are raw data with no further constructor
// calls in either classInit, so their types come only from their access instructions
// (lfs/stfs = f32, lha/sth = s16, lbz/stb = u8) -- named where a single, unambiguous use
// makes the role clear, left as `mUnk<offset>` otherwise. Not fully attributed: this
// unit's own scope is the class SHAPE and its overridden virtuals/states, not a complete
// semantic reconstruction of every field.
class daWaterMove_c : public dActorState_c {
public:
    // Explicit (mResFile has no default ctor -- ResCommon<T> only takes a void*),
    // but defined INLINE with a trivial body so MWCC inlines it into `new` at the
    // call site -- matching classInit's own observed shape (no separate
    // `bl __ct__13daWaterMove_cFv`, just the base ctor call then these two explicit
    // zero-stores alongside the sub-object constructions).
    daWaterMove_c() : mResFile(nullptr), mUnk434(0) {}

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual ~daWaterMove_c();

    STATE_FUNC_DECLARE(daWaterMove_c, Wait);
    STATE_FUNC_DECLARE(daWaterMove_c, Udmove);
    STATE_FUNC_DECLARE(daWaterMove_c, Lrmove);

    /// @unofficial fn_2_1524F0, called first from create(). Loads the water-float model
    /// resource, builds mModel/mAnimTexSrt from it.
    void createMdl();
    /// @unofficial fn_2_152640, called from execute(). Builds the world matrix from
    /// mPos/mScale/mAngle plus a computed wobble offset, then calc()s the model.
    void calcModel();
    /// @unofficial fn_2_1528C0, called from execute(). Searches players via
    /// fManager_c::searchBaseByGroupType and dWaterEntryMng_c, folding a per-player result
    /// bitmask into mUnk4B8/sets mUnk4A8 on a change.
    void checkPlayers();
    /// @unofficial fn_2_1529D0, called from executeState_Udmove/executeState_Lrmove.
    /// "Approach" utility: drives value toward a target derived from mUnk47C/mUnk480,
    /// clamping against field_348's direction sign.
    float approach(float value);
    /// @unofficial fn_2_152AA0, called from execute(). Advances a running angle
    /// (mUnk4BA) and writes its cos/sin (scaled) into mUnk498/mUnk49C.
    void calcWave();

    u8 mUnk3D0[4]; ///< @unofficial unidentified 4-byte gap before mAllocator.
    dHeapAllocator_c mAllocator; ///< +0x3d4
    nw4r::g3d::ResFile mResFile; ///< +0x3f0, zero-initialised only (ResCommon<T> is one ptr)
    m3d::mdl_c mModel; ///< +0x3f4
    u32 mUnk434; ///< @unofficial unidentified 4-byte field between mModel and
                 ///< mAnimTexSrt -- found by diffing classInit against target, which
                 ///< explicitly zeroes it (a real field, not filler -- see the class's
                 ///< own explicit inline ctor below, required anyway since mResFile has
                 ///< no default constructor).
    m3d::anmTexSrt_c mAnimTexSrt; ///< +0x438 -- confirmed by experiment: an EARLIER
                                 ///< draft added a separate `mAllocator_c mAnimAllocator`
                                 ///< member here, believing anmTexSrt_c's own create()
                                 ///< needed its own allocator distinct from mAllocator.
                                 ///< Removing it made classInit's own `li r3, 0x4c0` match
                                 ///< target EXACTLY and collapsed a spurious SECOND
                                 ///< `bl __ct__12mAllocator_cFv` down to the genuine single
                                 ///< one -- createMdl()'s own two create() calls both pass
                                 ///< `&mAllocator` (the dHeapAllocator_c at +0x3d4, which
                                 ///< IS-A mAllocator_c), never a second allocator.

    mVec3_c mHomePos; ///< +0x464, create()'s own copy of (mPos.x, mPos.y, 3000.0f)
    f32 mUnk470; ///< +0x470
    f32 mUnk474; ///< +0x474
    f32 mUnk478; ///< +0x478
    f32 mUnk47C; ///< +0x47c, approach()'s own low clamp
    f32 mUnk480; ///< +0x480, approach()'s own high clamp
    f32 mUnk484; ///< +0x484, approach()'s own "current vs target" comparison value
    f32 mUnk488; ///< +0x488
    f32 mUnk48C; ///< +0x48c
    f32 mUnk490; ///< +0x490
    f32 mUnk494; ///< +0x494
    f32 mUnk498; ///< +0x498, calcWave()'s own cos() output
    f32 mUnk49C; ///< +0x49c, calcWave()'s own sin() output
    f32 mUnk4A0; ///< +0x4a0
    u32 mUnk4A4; ///< +0x4a4, an index into dWaterEntryMng_c's own per-actor table
    s32 mUnk4A8; ///< +0x4a8, a countdown, decremented in execute()
    s32 mUnk4AC; ///< +0x4ac, bitfield-derived (create()'s own extrwi/clrlwi on the
                 ///< actor's own parameter word)
    s32 mUnk4B0; ///< +0x4b0, likewise bitfield-derived; also the "which of 3 changeState
                 ///< targets" selector read back by create()
    u32 mUnk4B4; ///< +0x4b4, an index (<<2) into lbl_2_rodata_81C8's own trailing floats
    u8 mUnk4B8; ///< +0x4b8
    s16 mUnk4BA; ///< +0x4ba, calcWave()'s own running angle counter
    u8 mUnk4BC_pad[4]; ///< @unofficial trailing padding to reach sizeof == 0x4c0
};

/// @unofficial dActorState_c/dActor_c's own field at +0x348 -- read/written by create(),
/// execute() and approach() as a plain byte. Not named in include/game/bases/d_actor.hpp
/// or d_base_actor.hpp under any obviously-matching identifier; left as a raw offset
/// accessor, the same idiom source/dol/bases/d_a_player_demo_manager.cpp already uses for
/// unnamed fields in an already-decompiled-elsewhere class hierarchy (see its own
/// `field_38c_ref`/`field_438_ref` helpers).
static inline u8 &field_348_ref(daWaterMove_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x348);
}

// DOL-absolute helpers on dWaterEntryMng_c -- not yet named/landed there (its own
// header, include/game/bases/d_water_entry_manager.hpp, is still just `mPad[0x8c0]`).
// Declared extern "C" at file scope, matching the FUN_<addr> convention
// include/game/bases/d_cs_seq_manager.hpp already uses for un-landed DOL members --
// dtk's own full symbol map already names these (bin/dtk/wiimj2d_symbols.txt), so no
// R_ synthetic symbol is needed (that technique is for un-landed regions of the SAME
// REL; these are DOL-absolute and always resolvable).
extern "C" u32 fn_800EB6D0(dWaterEntryMng_c *, void *, u32);
extern "C" void fn_800EBC80(dWaterEntryMng_c *, mVec3_c *, u32);
extern "C" void fn_800EBC40(dWaterEntryMng_c *, u32);
extern "C" u32 fn_800EBBC0(daPlBase_c *, f32 *, f32 *, u32, u8);

// Shared, file-scope constants pool. All FIVE of these `lbl_2_rodata_*` labels
// (81C8/8210/8228/8240/824C/8250) are read by the various member functions below via
// ADDRESSES RELATIVE TO lbl_2_rodata_81C8 ITSELF, well past that first object's own
// 0x48-byte size (e.g. approach() reads lbl_2_rodata_81C8+0x7c, landing inside what
// dtk separately labels lbl_2_rodata_8240) -- MWCC pools adjacent rodata into one
// contiguous blob addressed off its FIRST symbol, the same anchor-relative pattern
// already confirmed for g_profile_AC_WATER_MOVE's own STATE_DEFINE arguments. Declared
// as raw bits (not `f32`) because not every slot is a float -- idx13 (0x00030000) and
// idx30 (0x01020408, really 4 packed bytes) are not.
static const u32 sWaterMoveConsts[] = {
    0xFFFFFFFFu, 0x3F000000u, 0x41800000u, 0x3E000000u, 0x40000000u, 0x3F88F5C3u,
    0x437A0000u, 0x3F83D70Au, 0x43160000u, 0x3F88F5C3u, 0x451F6000u, 0x3F83D70Au,
    0x451F6000u, 0x00030000u, 0x43000000u, 0x43200000u, 0x43200000u, 0x453B8000u,
    // lbl_2_rodata_8210 (+0x48)
    0x00000000u, 0x43B40000u, 0x41800000u, 0x00000000u, 0x43300000u, 0x00000000u,
    // lbl_2_rodata_8228 (+0x60)
    0x3F800000u, 0x3CCCCCCDu, 0x3C23D70Au, 0x322BCC77u, 0x3F000000u, 0x3DCCCCCDu,
    // lbl_2_rodata_8240 (+0x78)
    0x01020408u, 0x3D800000u, 0x3E000000u,
    // lbl_2_rodata_824C (+0x84)
    0x3B800000u,
    // lbl_2_rodata_8250 (+0x88)
    0x40000000u,
};
static inline f32 wmConstF(int wordIdx) {
    return *reinterpret_cast<const f32 *>(&sWaterMoveConsts[wordIdx]);
}

STATE_DEFINE(daWaterMove_c, Wait);
STATE_DEFINE(daWaterMove_c, Udmove);
STATE_DEFINE(daWaterMove_c, Lrmove);

// fn_2_152010. classInit for AC_WATER_MOVE.
static void *classInit_AC_WATER_MOVE() { return new daWaterMove_c(); }
// fn_2_1520B0. classInit for AC_WATER_MOVE_REGULAR.
static void *classInit_AC_WATER_MOVE_REGULAR() { return new daWaterMove_c(); }

fProfile::fActorProfile_c g_profile_AC_WATER_MOVE = {
    &classInit_AC_WATER_MOVE, fProfile::AC_WATER_MOVE, fProfile::DRAW_ORDER::AC_WATER_MOVE, 0
};
fProfile::fActorProfile_c g_profile_AC_WATER_MOVE_REGULAR = {
    &classInit_AC_WATER_MOVE_REGULAR, fProfile::AC_WATER_MOVE_REGULAR,
    fProfile::DRAW_ORDER::AC_WATER_MOVE_REGULAR, 0
};

// ORDER (ground-truthed against bin/dtk/d_basesNP_symbols.txt): draw(), doDelete(), the
// nine state functions, THEN the destructor -- not destructor-first as an earlier draft
// had it. Fixed by reordering only.

// FULL FUNCTION ORDER, ground-truthed against bin/dtk/d_basesNP_symbols.txt's own
// addresses: create, execute, draw, doDelete, createMdl, calcModel, checkPlayers,
// approach, calcWave, then the nine state functions (each state's own finalize
// before its init/execute pair), and the destructor LAST -- not grouped by kind, and
// not destructor-first. An earlier draft had several of these badly out of order;
// fixed here by reordering only, no content change.

// fn_2_152150. UNVERIFIED past the classInit-level shape -- best-effort literal
// transcription of the target's own instructions, not yet round-tripped through
// verify_anon.py to a clean match. Parked here rather than omitted: every field and
// call target is real (cross-checked against landed headers), but the bitfield
// extraction (mParam's own sub-fields, exact boundaries unconfirmed) and the
// mStateMgr changeState dispatch (three raw vtable calls through
// lbl_2_bss_F5C0+0x10/0x50/0x90 -- the three sFStateID_c statics -- reproduced here as
// changeState() calls against the STATE_DEFINE-generated StateID_* objects) are
// transcribed, not independently re-derived.
int daWaterMove_c::create() {
    u32 param = mParam;
    u32 sel2 = (param >> 12) & 0x3;
    u32 sel4a = (param >> 8) & 0xf;
    u32 sel4b = (param) & 0xf;
    u32 sel8 = (param >> 24) & 0xff;
    field_348_ref(this) = (u8)sel4a;
    mUnk4B0 = sel4b;
    mUnk4AC = sel8;
    mUnk4A0 = 0.0f;
    mUnk4B4 = (sel2 == 3) ? 0 : sel2;

    mHomePos.x = mPos.x;
    mHomePos.y = mPos.y;
    mHomePos.z = 3000.0f;
    mPos.z = 3000.0f;

    static const f32 rndRange = 360.0f;
    mUnk488 = dGameCom::rndF(rndRange);
    mUnk48C = dGameCom::rndF(rndRange);
    mUnk490 = dGameCom::rndF(rndRange);
    mUnk494 = dGameCom::rndF(rndRange);

    createMdl();

    if (mUnk4B0 == 0) {
        static const f32 k58 = 2550.0f, k50 = 1.0700000524520874f;
        mUnk484 = mPos.y;
        mUnk480 = (mPos.y - k58) * k50 + mPos.y;
        mUnk47C = (mPos.x - k58) * k50 + mPos.x;
    } else {
        static const f32 k58 = 2550.0f, k50 = 1.0700000524520874f;
        mUnk484 = mPos.x;
        mUnk480 = (mPos.x - k58) * k50 + mPos.x;
        mUnk47C = (mPos.x - k58) * k50 + mPos.x;
    }

    struct WaterEntryArg { f32 x, y, z; u8 a, b; };
    WaterEntryArg arg;
    arg.x = mPos.x;
    arg.y = mPos.y;
    arg.z = mPos.z;
    arg.b = 0;
    arg.a = (u8)(mUnk4B4 + 3);
    mUnk4A4 = fn_800EB6D0(dWaterEntryMng_c::m_instance, &arg, 0);

    if (mUnk4B4 == 0) {
        mUnk4B0 = 2;
        mStateMgr.changeState(daWaterMove_c::StateID_Wait);
    } else if (mUnk4B0 == 0) {
        mStateMgr.changeState(daWaterMove_c::StateID_Udmove);
    } else {
        mStateMgr.changeState(daWaterMove_c::StateID_Lrmove);
    }

    return true;
}

// fn_2_1523A0. UNVERIFIED -- literal transcription, not round-tripped.
int daWaterMove_c::execute() {
    if (mUnk4A8 > 0) {
        mUnk4A8--;
    }
    mStateMgr.executeState();

    fn_800EBC80(dWaterEntryMng_c::m_instance, &mPos, mUnk4A4);

    checkPlayers();
    calcModel();

    mAnimTexSrt.play();

    mUnk470 = mPos.x - mHomePos.x;
    mUnk474 = mPos.y - mHomePos.y;
    mUnk478 = mPos.z - mHomePos.z;

    calcWave();

    ActorScrOutCheck(0);
    return true;
}

// fn_2_152480. draw() -- defers straight to mModel's own draw().
int daWaterMove_c::draw() {
    // Vtable slot 0x14/4 -- scnLeaf_c's 4th virtual (dtor,getType,remove,entry), NOT a
    // "draw" method by that name; matches nw4r's own "entry into this frame's scene"
    // idiom.
    mModel.entry();
    return true;
}

// fn_2_1524B0. doDelete() -- notifies dWaterEntryMng_c that this actor's slot
// (mUnk4A4) is being freed.
int daWaterMove_c::doDelete() {
    fn_800EBC40(dWaterEntryMng_c::m_instance, mUnk4A4);
    return true;
}

// fn_2_1524F0. UNVERIFIED -- literal transcription. Loads "obj_waterfloat" from
// "g3d/obj_waterfloat.brres" (both strings read anchor-relative to
// g_profile_AC_WATER_MOVE itself in the target -- an MWCC string-pooling detail this
// draft does not attempt to reproduce), builds mModel/mAnimTexSrt from it, and sets the
// model's own base animation frame from lbl_2_rodata_8228 (rate constant, unconfirmed).
void daWaterMove_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("obj_waterfloat", "g3d/obj_waterfloat.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl((ulong)mUnk4B4);
    mModel.create(resMdl, &mAllocator, 0x224, 1, nullptr);
    setSoftLight_MapObj(mModel);

    nw4r::g3d::ResAnmTexSrt resAnmTexSrt = mResFile.GetResAnmTexSrt((ulong)mUnk4B4);
    mAnimTexSrt.create(resMdl, resAnmTexSrt, &mAllocator, nullptr, 1);
    mAnimTexSrt.setAnm(mModel, resAnmTexSrt, 0, m3d::FORWARD_LOOP);

    static const f32 lbl_2_rodata_8228 = 1.0f;
    mModel.setAnm(mAnimTexSrt, lbl_2_rodata_8228);
    mModel.setPriorityDraw(-1, 0x85);

    mAllocator.adjustFrmHeap();
}

// fn_2_152640. UNVERIFIED -- literal transcription. Builds the world matrix from
// mPos/mAngle/mScale, adds a per-frame drift computed via sLib::addCalc, then rotates in
// three passes using mUnk498/mUnk49C's own wobble (each pass along one axis, feeding
// mUnk488/48c's own accumulator), and finally calc()s the leaf.
void daWaterMove_c::calcModel() {
    static const f32 lbl_2_rodata_81C8[] = {
        0.5f, 16.0f, 0.125f, 2.0f, 1.0700000524520874f, 250.0f, 1.0299999713897705f, 150.0f,
        1.0700000524520874f, 2550.0f, 1.0299999713897705f, 2550.0f, 0.0f, 128.0f, 160.0f,
        160.0f, 3000.0f
    };
    changePosAngle(&mPos, &mAngle, 0);
    PSMTXTrans(mMatrix, mPos.x, mPos.y, mPos.z);
    {
        mMtx_c tmp;
        PSMTXScale(tmp, mUnk488, mUnk48C, mUnk490);
        PSMTXConcat(mMatrix, tmp, mMatrix);
    }

    if (mUnk4A8 == 0) {
        mUnk4A0 = sLib::addCalc(&mUnk4A0, lbl_2_rodata_81C8[0x48 / 4 - 1],
                                 lbl_2_rodata_81C8[0x64 / 4 - 1], lbl_2_rodata_81C8[0x68 / 4 - 1],
                                 lbl_2_rodata_81C8[0x6c / 4 - 1]);
    } else {
        mUnk4A0 = sLib::addCalc(&mUnk4A0, lbl_2_rodata_81C8[0x60 / 4 - 1],
                                 lbl_2_rodata_81C8[0x70 / 4 - 1], lbl_2_rodata_81C8[0x60 / 4 - 1],
                                 lbl_2_rodata_81C8[0x74 / 4 - 1]);
    }

    (void)mModel;
    calcWave();
}

// fn_2_1528C0. UNVERIFIED -- literal transcription. Walks players via
// fManager_c::searchBaseByGroupType, checks isStatus(0x7e) and (for players not already
// excluded) calls fn_800EBBC0 with mPos.x/mPos.z, folds a per-player result byte through
// lbl_2_rodata_8240's own lookup table into mUnk4B8, forcing mUnk4A8=3 when it changes.
void daWaterMove_c::checkPlayers() {
    static const u8 lbl_2_rodata_8240[4] = {0x01, 0x02, 0x04, 0x08};

    u32 result = 0;
    daPlBase_c *player = (daPlBase_c *)fManager_c::searchBaseByGroupType(2, this);
    while (player != nullptr) {
        u8 kind = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 0x38c);
        if (kind == 1 || kind == 2) {
            if (!player->isStatus(0x7e)) {
                f32 px = mUnk47C, pz = mUnk480;
                u32 r = fn_800EBBC0(player, &px, &pz, mUnk4A4, player->mLayer);
                s32 sr = (s8)r;
                if (sr != -1) {
                    typedef u8 (daPlBase_c::*MemFn)();
                    MemFn fn = *reinterpret_cast<MemFn *>(reinterpret_cast<u8 *>(player) + 0x60);
                    u8 idx = (player->*fn)();
                    result |= lbl_2_rodata_8240[(s8)idx];
                }
            }
        }
        player = (daPlBase_c *)fManager_c::searchBaseByGroupType(2, player);
    }

    if (mUnk4B8 != result) {
        mUnk4A8 = 3;
    }
    mUnk4B8 = (u8)result;
}
// fn_2_1529D0. "Approach" utility -- smoothly drives `value` toward whichever of
// mUnk47C (low)/mUnk480 (high) it is not currently past, clamped by a per-frame step
// derived from lbl_2_rodata_81C8's own trailing floats (+0x7c step scale, +0x80 max
// step, +0x70 min... exact roles unconfirmed, transcribed literally from the target).
// Flips field_348_ref's own direction byte when a clamp actually triggers.
float daWaterMove_c::approach(float value) {
    int which;
    f32 step;
    if (mUnk484 >= value) {
        step = (mUnk47C - value) * wmConstF(31);
        which = 0;
    } else {
        step = (value - mUnk480) * wmConstF(31);
        which = 1;
    }
    step += wmConstF(32);
    if (step >= wmConstF(28)) {
        step = wmConstF(28);
    }
    if (field_348_ref(this) == 1) {
        step = -step;
    }
    value += step;
    if (which == 0) {
        if (value >= mUnk47C) {
            return value;
        }
        field_348_ref(this) = 1;
        return mUnk47C;
    } else {
        if (value <= mUnk480) {
            return value;
        }
        field_348_ref(this) = 0;
        return mUnk480;
    }
}

// fn_2_152AA0. Advances mUnk4BA (a running angle, +0x400 per call -- a quarter turn in
// the 0x10000-per-circle fixed-point convention) and writes its rate-scaled cos()/sin()
// into mUnk498/mUnk49C.
void daWaterMove_c::calcWave() {
    mUnk498 = nw4r::math::CosIdx(mUnk4BA) * wmConstF(34);
    mUnk49C = nw4r::math::SinIdx(mUnk4BA) * wmConstF(34);
    mUnk4BA += 0x400;
}

// ORDER, ground-truthed against bin/dtk/d_basesNP_symbols.txt's own addresses, NOT
// grouped by function kind: finalizeState_Wait(0x152b40), initializeState_Wait(0x152b60),
// executeState_Wait(0x152b70), finalizeState_Udmove(0x152b80),
// initializeState_Udmove(0x152ba0), executeState_Udmove(0x152bb0),
// finalizeState_Lrmove(0x152bf0), initializeState_Lrmove(0x152c10),
// executeState_Lrmove(0x152c20) -- each state's own finalize precedes its init/execute
// pair. The verify_anon order flag (a single function, executeState_Lrmove, "defined too
// late") was real: my previous draft grouped all three init stubs, then all three
// finalize bodies, then both non-trivial executes, which does not match this
// per-state finalize/init/execute grouping. Fixed by reordering only -- no content
// change.

// fn_2_152B40. All THREE states' finalize is the SAME body: reset mUnk47C/mUnk480/
// mUnk484 to lbl_2_rodata_8210's own first word (0.0f).
void daWaterMove_c::finalizeState_Wait() {
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
}
// fn_2_152B60/152B70. Empty stubs -- literally `blr` in the target.
void daWaterMove_c::initializeState_Wait() {}
void daWaterMove_c::executeState_Wait() {}

// fn_2_152B80.
void daWaterMove_c::finalizeState_Udmove() {
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
}
// fn_2_152BA0.
void daWaterMove_c::initializeState_Udmove() {}
// fn_2_152BB0. execute() drives mPos.y through approach().
void daWaterMove_c::executeState_Udmove() {
    mPos.y = approach(mPos.y);
}

// fn_2_152BF0.
void daWaterMove_c::finalizeState_Lrmove() {
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
}
// fn_2_152C10.
void daWaterMove_c::initializeState_Lrmove() {}
// fn_2_152C20. execute() drives mPos.x through approach().
void daWaterMove_c::executeState_Lrmove() {
    mPos.x = approach(mPos.x);
}

// fn_2_152C60. One-slot flag-argument destructor -- same ABI as
// d_a_dummy_door.cpp's own. Destroys mAnimTexSrt, mModel, mAllocator (reverse of
// construction order), then chains to dActorState_c's own dtor.
daWaterMove_c::~daWaterMove_c() {}

