#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/sLib/s_State.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_heap.hpp>

// @unofficial lbl_2_bss_A6C4 -- one of this unit's own 3 confirmed .bss
// singletons (scout_unit.py: ".bss 3 distinct targets 0xA638..0xA6C4").
// Its address (not its value) is passed as an argument to a virtual
// method both executeState_DemoDown and executeState_DemoUp call, at
// vtable byte offset 0xd4 through this object's own +0x60 vtable
// pointer -- real type/name not identified. Called via a raw vtable
// offset rather than a named method, matching this project's established
// technique for a genuinely-unidentified slot (never eyeball/guess a
// name -- see MAPPING.md for what was checked).
extern u8 lbl_2_bss_A6C4;
extern u8 lbl_2_bss_A648;
typedef void (*daLemmyFootholdVFunc0xD4_t)(dEn_c *, void *);

// @unofficial Local stand-in for the still-opaque `sBgSetInfo` (forward-
// declared only, per the coordinator's own instruction not to invent
// fields for a type reached solely by pointer). This is NOT a claim
// about the real struct's layout -- it exists only so this TU can build
// a temporary with the right BYTES and pass its address cast to
// `const sBgSetInfo *`, matching the two call sites' own stack-store
// pattern (4 floats read from `lbl_2_rodata_4A80+0x4/+0x8/+0xc/+0x10`,
// then 3 zeroed words) as closely as an untyped stand-in can.
struct sBgSetInfoLocal_t {
    float a, b, c, d;
    u32 e, f, g;
};

// @unofficial LEMMY_FOOTHOLD_MAIN's own class layout, read directly off its
// constructor (fn_2_C5CC0) and cross-checked against the already-landed
// d_a_wm_antlion.cpp's own m3d-model construction idiom (same module,
// d_basesNP), which uses the identical "base ctor call, then vtable-patch
// to the derived animation type" shape for its own mChrAnim/mAnimTexSrt
// members.
//
// sizeof == 0x6a8 (both LEMMY_FOOTHOLD and LEMMY_FOOTHOLD_MAIN allocate
// this exact size -- `li r3, 0x6a8` in both classInits).
//
// Base: dEn_c (sizeof 0x528, confirmed via STATIC_ASSERT), NOT
// dActorState_c/dWmEnemy_c -- confirmed via the 5-character mangled ctor
// call `__ct__5dEn_cFv`.
//
// Members, offsets read directly from the ctor:
//   dHeapAllocator_c mAllocator  @ +0x524  (sizeof 0x1c, probed) --
//     reuses 4 bytes of dEn_c's own tail alignment padding (dEn_c's
//     reported sizeof 0x528 rounds up from a true content end at 0x524).
//   u32 m_540                    @ +0x540  (unidentified, explicitly
//     zeroed -- fills the 4-byte gap between mAllocator's end (0x540)
//     and mModel's start (0x544))
//   m3d::mdl_c mModel            @ +0x544  (sizeof 0x40, probed)
//   u32 m_584                    @ +0x584  (unidentified, explicitly
//     zeroed -- fills the 4-byte gap between mModel's end (0x584) and
//     mAnimTexSrt's start (0x588))
//   m3d::anmTexSrt_c mAnimTexSrt @ +0x588  (sizeof 0x2c, probed;
//     matches d_a_wm_antlion.cpp's own anmTexSrt_c probe exactly) --
//     its own embedded banm_c-level fields (mpObj/mpHeap/mAllocator)
//     construct first (vtable briefly `__vt__Q23m3d6banm_c`), then its
//     own vtable overwrites to `__vt__Q23m3d11anmTexSrt_c` as the last
//     step -- ordinary C++ construction order, not hand-rolled.
//   dBg_ctr_c mBgCtr             @ +0x5b4 in LEMMY_FOOTHOLD_MAIN
//                                 (sizeof 0xe4, probed), but @ +0x5c0 in
//     plain LEMMY_FOOTHOLD (fn_2_C61E0) -- a real, confirmed 0xc-byte
//     difference between the two classes, not yet resolved (see
//     MAPPING.md).
//
// UNRESOLVED, not modelled below: the two 4-byte gaps (m_540, m_584) are
// real fields whose names/types are not yet identified -- kept as raw
// u32 placeholders rather than guessed at. The tail after mBgCtr (0x10
// bytes in MAIN, 0x4 bytes in plain FOOTHOLD, ending at the shared
// sizeof 0x6a8) is also not yet modelled. The 0xc-byte layout difference
// between the two classes' own dBg_ctr_c offset is confirmed real (not a
// misread) but its cause (an extra member specific to plain
// LEMMY_FOOTHOLD) is not yet identified.

class daLemmyFoothold_c : public dEn_c {
public:
    daLemmyFoothold_c();

    // @unofficial fn_2_C6650, 12 words. `mModel.entry(); return 1;` --
    // entry() confirmed at vtable byte offset 0x14 by a probe compile of
    // m3d::mdl_c (a hand-count would have said setAnm() -- exactly the
    // "never eyeball" trap; the probe settled it in one step). Matches
    // d_a_wm_antlion.cpp's own established `mModel.entry(); return
    // SUCCEEDED;` draw() idiom exactly.
    virtual int draw();
    // @unofficial fn_2_C6680, 10 words. `mBgCtr.release(); return 1;`
    virtual int doDelete();
    // @unofficial fn_2_C6590, 23 words, read in full before writing:
    // `mStateMgr.executeState(); calcModel(); mBgCtr.calc(); return 1;`
    virtual int execute();
    // @unofficial fn_2_C6310, 32 words, read in full before writing:
    // `vUnk2A4(); vUnk2A8(); <vtable-slot-0xd4>(&lbl_2_bss_A6C4);
    // mStateMgr.refreshState(); return 1;` -- refreshState() confirmed
    // at vtable slot 0x1c by a probe compile of dEn_c::mStateMgr
    // (sStateStateMgr_c's own inherited sStateMgrIf_c slot layout).
    virtual int create();

    // @unofficial Three states, read directly from .data: StateID_DemoWait,
    // StateID_DemoDown, StateID_DemoUp. All three genuinely VIRTUAL --
    // confirmed via the state objects' own PMF encoding in .data (a
    // vtable BYTE OFFSET, not a relocatable address; word shape
    // {vtable_offset, 0x60, 0}, the FLOOR_JR_A-established signature for
    // a virtual pointer-to-member). No name collision with
    // daLemmyFootholdMain_c's own (unrelated) StateID_DemoWait: that
    // class's states are NON-virtual (see below), so its STATE_DEFINE
    // never emits the file-scope baseID_ template this class's own
    // STATE_VIRTUAL_DEFINE needs -- both can use the ordinary macro
    // untouched, no hand-expansion required.
    //
    // DECLARATION ORDER IS LOAD-BEARING here: these three states must
    // come BEFORE vUnk2A4/vUnk2A8/calcModel below, confirmed by reading
    // fn_2_C6310 (a not-yet-authored function, still called `vUnk2A4()`/
    // `vUnk2A8()` below) dispatch through this class's own vtable at
    // +0x2a4/+0x2a8 -- i.e. right after the states' own 9 slots
    // (+0x280..+0x2a0) end, not before them.
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFoothold_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFoothold_c, DemoDown);
    STATE_VIRTUAL_FUNC_DECLARE(daLemmyFoothold_c, DemoUp);

    // @unofficial Two more NEW virtuals this class introduces, real
    // name/content not yet identified -- found while fixing calcModel()'s
    // own vtable slot (execute() dispatches calcModel() at +0x2ac; it
    // only lands there once two slots' worth of real virtuals sit before
    // it). Both are called, no-arg, from fn_2_C6310 (32 words, not yet
    // authored -- likely create(), see the file-scope comment near
    // STATE_VIRTUAL_DEFINE below) at vtable+0x2a4/+0x2a8 respectively.
    // Declared here purely to get the SLOT NUMBERS right for calcModel(),
    // confirmed correct because execute() now matches exactly.
    // vUnk2A4 is `int`, not `void`: target fn_2_C64D0/fn_2_C5F30 both
    // end with `li r3, 0x1` immediately before the epilogue, a real
    // returned value, not a leftover unused store (confirmed by the
    // draft being exactly one word SHORT/LONG against each target until
    // this was added -- a size mismatch this project treats as
    // structural, not scheduling). vUnk2A8's own body remains open.
    virtual int vUnk2A4();
    virtual void vUnk2A8();
    // @unofficial Dispatched through this class's own vtable at +0x2ac
    // (confirmed: execute() now matches target exactly at this slot).
    // Named by the established project-wide idiom (execute() calling a
    // class-specific calc through its own vtable, e.g.
    // d_a_wm_antlion.cpp's own execute()/calc() pair).
    virtual void calcModel();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mModel;
    nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    m3d::anmTexSrt_c mAnimTexSrt;
    float m_5b4;
    // @unofficial +0x5b8. Confirmed a target/goal Y position, not just a
    // generic float: both executeState_DemoDown and executeState_DemoUp
    // compute `this[0x5b8] - mPos.y` as a distance-to-target and drive
    // mSpeed.y from it (DemoUp additionally snaps mPos.y to this value
    // once within 1.0 unit and calls a completion callback -- see the
    // state bodies below). Real declared name not identified; named for
    // its confirmed role rather than left as a raw offset.
    float mTargetPosY;
    float m_5bc;
    dBg_ctr_c mBgCtr;
    u8 mTail[0x4];
};

daLemmyFoothold_c::daLemmyFoothold_c() {}
ACTOR_PROFILE(LEMMY_FOOTHOLD, daLemmyFoothold_c, 0);

// @unofficial State bodies -- STUBS for now (empty), not yet read against
// target bytes. Declaring them (even empty) is what is needed to get the
// framework-generated machinery (state object dtor/number()/superID()/
// isSameName()/three trampolines -- none hand-authored) to compile and
// be positioned correctly; the REAL per-state behaviour is separate,
// unauthored work for a later pass.
// @unofficial Attributed by reading the PMF function-pointer values
// directly out of the target's own vtable (per the coordinator's
// technique), NOT from verify_anon's size-based candidate pairing.
// daLemmyFoothold_c's states use the VIRTUAL (vtable-offset) PMF
// encoding in target -- unlike daLemmyFootholdMain_c's own (direct
// address) encoding -- so the state object's own PMF fields in .data
// hold a vtable BYTE OFFSET, not a relocatable address; reading target's
// own vtable (lbl_2_data_27E10) at those exact offsets gave the real
// function addresses. WHY the two classes differ in PMF encoding is not
// resolved (both use the identical STATE_VIRTUAL_FUNC_DECLARE/_DEFINE
// shape) -- flagged as a genuine open question, not chased further this
// round since it only affects __sinit's own bytes (explicitly
// deprioritised, see MAPPING.md).
void daLemmyFoothold_c::initializeState_DemoWait() {}
void daLemmyFoothold_c::executeState_DemoWait() { mAnimTexSrt.play(); }
void daLemmyFoothold_c::finalizeState_DemoWait() {}
void daLemmyFoothold_c::initializeState_DemoDown() {
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mAccelY = -0.1850000023841858f;
}
// @unofficial fn_2_C6720, 0x84 bytes. Read in full:
// mAnimTexSrt.play(); then a distance check between mTargetPosY and
// mPos.y against a rodata threshold (lbl_2_rodata_4AA4) gating the SAME
// raw vtable-slot-0xd4 call executeState_DemoUp uses below (confirmed
// identical shape at both call sites, cross-checked instruction by
// instruction). The exact rodata threshold value and full gating
// condition were read but the call's own real identity (what state
// transition or notification it performs) is not resolved -- executed
// via the raw vtable-offset idiom below rather than guessed at.
void daLemmyFoothold_c::executeState_DemoDown() {
    mAnimTexSrt.play();
    calcSpeedY();
    posMove();
    if (mTargetPosY - mPos.y > 128.0f) {
        void *vtable = *(void **) ((u8 *) this + 0x60);
        daLemmyFootholdVFunc0xD4_t f = *(daLemmyFootholdVFunc0xD4_t *) ((u8 *) vtable + 0xd4);
        f(this, &lbl_2_bss_A6C4);
    }
}
void daLemmyFoothold_c::finalizeState_DemoDown() {}
void daLemmyFoothold_c::initializeState_DemoUp() {}
// @unofficial fn_2_C67D0, 0xBC bytes -- read in full before writing.
// mTargetPosY (+0x5b8, see the class comment above) is a genuine
// target/goal Y position: this state eases mSpeed.y toward it
// (`(mTargetPosY - mPos.y) / 10.0f`), calls posMove(), then re-measures
// the (now smaller) remaining distance; once |distance| < 1.0f it snaps
// mPos.y to the exact target and calls the same raw vtable-slot-0xd4
// method as executeState_DemoDown above (same unidentified method,
// same &lbl_2_bss_A6C4 argument -- confirmed the identical call shape
// at both sites before writing this).
void daLemmyFoothold_c::executeState_DemoUp() {
    mAnimTexSrt.play();
    mSpeed.y = (mTargetPosY - mPos.y) / 10.0f;
    posMove();
    float dist = mTargetPosY - mPos.y;
    float absDist = (dist > 0.0f) ? dist : -dist;
    if (absDist < 1.0f) {
        mPos.y = mTargetPosY;
        void *vtable = *(void **) ((u8 *) this + 0x60);
        daLemmyFootholdVFunc0xD4_t f = *(daLemmyFootholdVFunc0xD4_t *) ((u8 *) vtable + 0xd4);
        f(this, &lbl_2_bss_A6C4);
    }
}
void daLemmyFoothold_c::finalizeState_DemoUp() {}

class daLemmyFootholdMain_c : public dEn_c {
public:
    daLemmyFootholdMain_c() {}

    // @unofficial Two states, read directly from .data: StateID_DemoWait,
    // StateID_Wait. Both genuinely NON-VIRTUAL -- confirmed via the state
    // objects' own PMF encoding: a direct, relocatable function address
    // (word shape {-1, fn_addr, 0}, the FLOOR_JR_A-established signature
    // for a non-virtual pointer-to-member), not a vtable offset. Corrected
    // this round from an earlier, wrong guess that used the VIRTUAL macro
    // for both classes uniformly -- the two classes' states are NOT
    // declared the same way in the real source.
    STATE_FUNC_DECLARE(daLemmyFootholdMain_c, DemoWait);
    STATE_FUNC_DECLARE(daLemmyFootholdMain_c, Wait);

    // @unofficial Same three overrides as daLemmyFoothold_c above, at its
    // own target addresses (0xC60F0 draw, 0xC6120 doDelete, 0xC6000
    // execute -- all read/confirmed the same way).
    virtual int draw();
    virtual int doDelete();
    virtual int execute();
    // @unofficial fn_2_C5D70, 32 words -- this class's own counterpart
    // to daLemmyFoothold_c::create() above, identical shape.
    virtual int create();
    // @unofficial Same two unidentified virtuals as daLemmyFoothold_c's
    // own vUnk2A4/vUnk2A8 above (dispatched from fn_2_C5D70, this
    // class's own not-yet-authored counterpart to fn_2_C6310, at
    // vtable+0x280/+0x284 -- no state slots precede them here since this
    // class's own states are non-virtual). vUnk2A4 is `int`, see the
    // sibling class's own declaration above for the evidence.
    virtual int vUnk2A4();
    virtual void vUnk2A8();
    virtual void calcModel();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mRes;
    m3d::mdl_c mModel;
    nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    m3d::anmTexSrt_c mAnimTexSrt;
    dBg_ctr_c mBgCtr;
    u8 mTail[0x10];
};

ACTOR_PROFILE(LEMMY_FOOTHOLD_MAIN, daLemmyFootholdMain_c, 0);

// @unofficial State bodies -- STUBS for now, see the note on
// daLemmyFoothold_c's own state bodies above.
// @unofficial Attributed by reading the PMF function-pointer relocations
// directly out of the state objects' own .data fields (per the
// coordinator's technique) -- daLemmyFootholdMain_c's states use DIRECT
// (relocatable-address) PMF encoding, unlike daLemmyFoothold_c's own
// (vtable-offset) encoding above. Confirmed: initializeState_DemoWait/
// finalizeState_DemoWait/initializeState_Wait/finalizeState_Wait are all
// genuinely trivial (target bodies are bare `blr`); executeState_DemoWait/
// executeState_Wait both resolve to the mAnimTexSrt vtable dispatch thunk
// (`lwzu r12,0x588(r3); lwz r12,0x14(r12); mtctr; bctr`) at
// 0xC6170/0xC61A0 -- confirmed via a probe compile of m3d::anmTexSrt_c
// showing play() lands at vtable byte offset 0x14 exactly.
void daLemmyFootholdMain_c::initializeState_DemoWait() {}
void daLemmyFootholdMain_c::executeState_DemoWait() { mAnimTexSrt.play(); }
void daLemmyFootholdMain_c::finalizeState_DemoWait() {}
void daLemmyFootholdMain_c::initializeState_Wait() {}
void daLemmyFootholdMain_c::executeState_Wait() { mAnimTexSrt.play(); }
void daLemmyFootholdMain_c::finalizeState_Wait() {}

// @unofficial draw()/doDelete()/execute() for both classes -- confirmed
// via probe compile (m3d::mdl_c::entry() at vtable+0x14, NOT a hand
// count) and full disassembly reads (see the class-body comments above
// for the exact target addresses and reasoning).
int daLemmyFoothold_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}
int daLemmyFoothold_c::doDelete() {
    mBgCtr.release();
    return 1;
}
int daLemmyFoothold_c::execute() {
    mStateMgr.executeState();
    calcModel();
    mBgCtr.calc();
    return 1;
}
int daLemmyFootholdMain_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}
int daLemmyFootholdMain_c::doDelete() {
    mBgCtr.release();
    return 1;
}
int daLemmyFootholdMain_c::execute() {
    mStateMgr.executeState();
    calcModel();
    mBgCtr.calc();
    return 1;
}

// @unofficial __sinit registers these five state objects in this exact
// order (read directly off fn_2_C6920's own sequence of
// __ct__10sStateID_cFPCc/__register_global_object call pairs, matching
// the coordinator's own .data-address scan): MAIN::DemoWait, MAIN::Wait,
// FOOTHOLD::DemoWait, FOOTHOLD::DemoDown, FOOTHOLD::DemoUp. Static
// initializers run in TEXTUAL declaration order within a TU, so this
// block's own ordering is load-bearing, not cosmetic.
//
// MAIN's two states use the plain, NON-virtual macro (STATE_DEFINE) --
// corrected this round from the virtual macro, see the class-body note
// above. This macro emits no baseID_ helper at all, so there is no
// collision with FOOTHOLD's own (virtual, unrelated) DemoWait below --
// the earlier hand-expansion this state needed is no longer necessary.
STATE_DEFINE(daLemmyFootholdMain_c, DemoWait);
STATE_DEFINE(daLemmyFootholdMain_c, Wait);

// @unofficial FOOTHOLD's three states all use the ordinary virtual macro
// directly -- no hand-expansion needed now that MAIN's own states are
// correctly non-virtual (see above): there is no shared baseID_DemoWait
// symbol to collide over.
STATE_VIRTUAL_DEFINE(daLemmyFoothold_c, DemoWait);
STATE_VIRTUAL_DEFINE(daLemmyFoothold_c, DemoDown);
STATE_VIRTUAL_DEFINE(daLemmyFoothold_c, DemoUp);
// @unofficial fn_2_C65F0 (FOOTHOLD, 21 words) / fn_2_C6060 (MAIN, 36
// words) -- read in full before writing. Both build a transform matrix
// from mPos/mAngle then push it and mScale to the model; MAIN's is the
// fuller shape (rotates on all three axes), FOOTHOLD's skips rotation
// entirely (translation only) -- a real, confirmed difference between
// the two classes, not an oversight. Both explain why execute()'s
// vtable-slot mismatch (0x280 vs the wanted 0x288/0x2ac) closes once
// these are declared: calcModel() needed to exist as a real virtual
// before its own slot number would be correct.
void daLemmyFoothold_c::calcModel() {
    mMatrix.trans(mPos);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
}
void daLemmyFootholdMain_c::calcModel() {
    mMatrix.trans(mPos);
    mMatrix.YrotM(mAngle.y);
    mMatrix.XrotM(mAngle.x);
    mMatrix.ZrotM(mAngle.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
}
// @unofficial fn_2_C64D0 (48 words), read in full: snapshots mPos into
// m_5b4/mTargetPosY/m_5bc, resets mScale to 1.0f, then calls
// `dBg_ctr_c::set(dActor_c*, const sBgSetInfo*, u8, u8, mVec3_c*)`
// (the third overload, now landed in the real
// include/game/bases/d_bg_ctr.hpp), sets a flag bit on mBgCtr's own
// (already-declared) mFlags field, then calls mBgCtr.entry(). The `u8`
// at `this+0x38f` is a real field read directly off target, not yet
// identified by name -- raw offset cast rather than guessed.
//
// The `info` literals here are NOT the same four values as MAIN's own
// copy below -- measured directly from original/d_basesNP.rel's
// .rodata (section 4, file offset 0x1c6600 + 0x4a80/+0x18/+0x8): this
// class's target only issues 3 rodata loads (0x0=1.0, 0x18=-16.0,
// 0x8=16.0) and reuses two of them (info.a==info.d==-16.0f,
// info.b==info.c==16.0f), where MAIN's own copy loads 5 DISTINCT
// constants with no reuse. Copying MAIN's -152/16/152/-48 values here
// (as an earlier round did) forced two extra, unnecessary rodata loads
// and was the actual cause of a real SIZE mismatch (50 words emitted
// vs target's 48) -- not a scheduling residual.
int daLemmyFoothold_c::vUnk2A4() {
    m_5b4 = mPos.x;
    mTargetPosY = mPos.y;
    m_5bc = mPos.z;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mVec3_c v;
    v.x = 1.0f;
    v.y = 1.0f;
    v.z = 1.0f;
    sBgSetInfoLocal_t info;
    info.a = -16.0f;
    info.b = 16.0f;
    info.c = 16.0f;
    info.d = -16.0f;
    info.e = 0;
    info.f = 0;
    info.g = 0;
    u8 u = *((u8 *) this + 0x38f);
    mBgCtr.set(this, (const sBgSetInfo *) &info, 3, u, &v);
    mBgCtr.mFlags |= 4;
    mBgCtr.entry();
    return 1;
}
// @unofficial fn_2_C6390 (78 words) -- this class's own createModel(),
// matching the established d_a_wm_antlion.cpp idiom closely (heap
// allocator, resource lookup, m3d::mdl_c create, setSoftLight_MapObj,
// anmTexSrt_c create + setAnm). Archive strings ("g3d/
// boss_lemmy_ashiba.brres" / "boss_lemmy_ashiba") read directly out of
// .data and confirmed SHARED between both classes -- daLemmyFoothold_c's
// own copy of this function reaches the identical two addresses via a
// different base register (g_profile_LEMMY_FOOTHOLD+0x18/+0x34), not a
// separate string.
void daLemmyFoothold_c::vUnk2A8() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    mRes = dResMng_c::m_instance->getRes("boss_lemmy_ashiba", "g3d/boss_lemmy_ashiba.brres");
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("boss_lemmy_ashiba");
    mModel.create(mdl, &mAllocator, 0x24, 1, nullptr);
    dActor_c::setSoftLight_MapObj(mModel);
    mResAnmTexSrt = mRes.GetResAnmTexSrt("boss_lemmy_ashiba");
    mAnimTexSrt.create(mdl, mResAnmTexSrt, &mAllocator, nullptr, 1);
    mAnimTexSrt.setAnm(mModel, mResAnmTexSrt, 0, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnimTexSrt);
    mAnimTexSrt.setRate(1.0f, 0);
    mAllocator.adjustFrmHeap();
}
// @unofficial fn_2_C5F30 (50 words), read in full: same shape as
// daLemmyFoothold_c's own vUnk2A4() above, but writes the mPos snapshot
// into THIS class's own tail region (+0x698/+0x69c/+0x6a0, right after
// mBgCtr which ends at +0x698 for this class -- confirmed the offset,
// not assumed from symmetry with FOOTHOLD alone).
int daLemmyFootholdMain_c::vUnk2A4() {
    *(float *) ((u8 *) this + 0x698) = mPos.x;
    *(float *) ((u8 *) this + 0x69c) = mPos.y;
    *(float *) ((u8 *) this + 0x6a0) = mPos.z;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mVec3_c v;
    v.x = 1.0f;
    v.y = 1.0f;
    v.z = 1.0f;
    sBgSetInfoLocal_t info;
    info.a = -152.0f;
    info.b = 16.0f;
    info.c = 152.0f;
    info.d = -48.0f;
    info.e = 0;
    info.f = 0;
    info.g = 0;
    u8 u = *((u8 *) this + 0x38f);
    mBgCtr.set(this, (const sBgSetInfo *) &info, 3, u, &v);
    mBgCtr.mFlags |= 4;
    mBgCtr.entry();
    return 1;
}
// @unofficial fn_2_C5DF0 (78 words) -- MAIN's own createModel(). Same
// logical shape as daLemmyFoothold_c's own above (same calls, same
// order), but NOT the same emitted addressing -- measured directly from
// the raw relocation table (wip/wm_units/profile_map.py's relocations()),
// not from the disassembly's symbol labels, which can mislead (a label
// is dtk's nearest-symbol heuristic, not evidence of what the source
// computed). FOOTHOLD's own copy (fn_2_C6390) has DIRECT relocations to
// two addresses (0x27dc8, 0x27de4) with no shared base -- ordinary fresh
// string literals. MAIN's copy has exactly TWO relocations, BOTH
// targeting 0x27db0 -- g_profile_LEMMY_FOOTHOLD's own address, i.e.
// daLemmyFoothold_c's profile object, declared earlier in this same
// TU -- with the two getRes() arguments reached by PLAIN IMMEDIATE
// arithmetic off that one base (+0x34 for the name, +0x18 for the path,
// no further relocation for either add). So MAIN's real source reaches
// FOOTHOLD's own archive name/path via pointer arithmetic on
// &g_profile_LEMMY_FOOTHOLD, not by writing its own fresh string
// literals for this call -- the offsets are unofficial (no named field
// exists for them in the current fProfile::fActorProfile_c), but the
// BASE and the "no separate relocation for the two adds" shape are
// measured, not guessed. GetResMdl/GetResAnmTexSrt below still take a
// literal "boss_lemmy_ashiba" -- their own target relocations (not
//0x27db0-relative) show that pattern was NOT changed for those two
// calls, only for the getRes() call itself.
void daLemmyFootholdMain_c::vUnk2A8() {
    const char *path = (const char *) ((u8 *) &g_profile_LEMMY_FOOTHOLD + 0x18);
    const char *name = (const char *) ((u8 *) &g_profile_LEMMY_FOOTHOLD + 0x34);
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);
    mRes = dResMng_c::m_instance->getRes(name, path);
    nw4r::g3d::ResMdl mdl = mRes.GetResMdl("boss_lemmy_ashiba");
    mModel.create(mdl, &mAllocator, 0x24, 1, nullptr);
    dActor_c::setSoftLight_MapObj(mModel);
    mResAnmTexSrt = mRes.GetResAnmTexSrt("boss_lemmy_ashiba");
    mAnimTexSrt.create(mdl, mResAnmTexSrt, &mAllocator, nullptr, 1);
    mAnimTexSrt.setAnm(mModel, mResAnmTexSrt, 0, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnimTexSrt);
    mAnimTexSrt.setRate(1.0f, 0);
    mAllocator.adjustFrmHeap();
}

int daLemmyFoothold_c::create() {
    vUnk2A4();
    vUnk2A8();
    void *vtable = *(void **) ((u8 *) this + 0x60);
    daLemmyFootholdVFunc0xD4_t f = *(daLemmyFootholdVFunc0xD4_t *) ((u8 *) vtable + 0xd4);
    f(this, &lbl_2_bss_A6C4);
    mStateMgr.refreshState();
    return 1;
}
int daLemmyFootholdMain_c::create() {
    vUnk2A4();
    vUnk2A8();
    void *vtable = *(void **) ((u8 *) this + 0x60);
    daLemmyFootholdVFunc0xD4_t f = *(daLemmyFootholdVFunc0xD4_t *) ((u8 *) vtable + 0xd4);
    f(this, &lbl_2_bss_A648);
    mStateMgr.refreshState();
    return 1;
}
