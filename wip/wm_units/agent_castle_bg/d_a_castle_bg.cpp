#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_castle_bg.hpp>

// MIDDLE_BG_FOR_CASTLE_LUDWIG and BOTTOM_BG_FOR_CASTLE_LUDWIG share this one translation unit
// (coordinator-scoped: .text 0xf5130-0xf6150, 0x1020 bytes, ONE .ctors entry at 0xf5c80, owns
// .rodata 0x5bc0, .data 0x308f8-0x30f34, .bss 0xc1a0-0xc1ac).
//
// CORRECTED THIS ROUND: a full slot-by-slot diff of both classes' complete vtables (not an
// eyeballed prefix) found that THREE of the nine new virtuals (vtable offsets 0x280/0x298/
// 0x29c), not just the destructor, differ between daMiddleBGForCastleLudwig_c and
// daBottomBGForCastleLudwig_c. An earlier draft this round had attached the wrong class's
// functions to the wrong slot (backwards) -- caught and fixed before being reported done, per
// this project's "diff anything you believe is boilerplate" caution. Real class name also
// corrected to "daMiddleBGForCastleLudwig_c" (capital BG), read directly from a state-ID name
// string literal in .data, not guessed.
//
// STILL PARTIAL: create()/doDelete()/execute()/draw(), MIDDLE_BG's own vf280/vf29c, and
// BOTTOM_BG's own vf29c are FAKE STUBS (clearly marked, each at its correct address slot).
// Roughly a dozen more functions in-range (including __sinit and the two plain STATE_DEFINE
// states this unit turns out to have) are scouted only by size/address and are NOT YET present
// in this draft. See MAPPING.md for the full inventory.

ACTOR_PROFILE(MIDDLE_BG_FOR_CASTLE_LUDWIG, daMiddleBGForCastleLudwig_c, 0);

ACTOR_PROFILE(BOTTOM_BG_FOR_CASTLE_LUDWIG, daBottomBGForCastleLudwig_c, 0);

// fn_2_F51B0. Confirmed content: `return &sStateID::null;`.
const sStateID_c &daMiddleBGForCastleLudwig_c::getNullState() { return sStateID::null; }

// fn_2_F51C0. Shared base ctor. Confirmed content: base dEn_c ctor (implicit vtable-fixup to
// THIS class's own vtable), then mAllocator/mModel's own ctors, then __construct_array builds
// mBgCtr[2].
daMiddleBGForCastleLudwig_c::daMiddleBGForCastleLudwig_c() : m_540(nullptr) {}

// fn_2_F5240. Base destructor -- confirmed content: __destroy_arr's mBgCtr[2], then
// mModel/mAllocator's own dtors, then the base dEn_c dtor.
daMiddleBGForCastleLudwig_c::~daMiddleBGForCastleLudwig_c() {}

// fn_2_F52D0. Confirmed content: touches ONLY mBgCtr[0], a tail call either way.
void daMiddleBGForCastleLudwig_c::entryOrRelease(bool doEntry) {
    if (doEntry) {
        mBgCtr[0].entry();
    } else {
        mBgCtr[0].release();
    }
}

// fn_2_F52F0. Confirmed content: NOT a vtable slot -- an ordinary helper touching BOTH zones
// (unlike entryOrRelease's own zone-0-only shape) plus a model render option.
void daMiddleBGForCastleLudwig_c::activate(bool show) {
    if (show) {
        mModel.setOption(1, 0);
    } else {
        mModel.setOption(1, 1);
    }
    if (show) {
        mBgCtr[1].entry();
        mBgCtr[0].entry();
    } else {
        mBgCtr[1].release();
        mBgCtr[0].release();
    }
}

// The window-node name table both vf280 and vf284 index -- real strings read directly from
// .data (lbl_2_data_30910/lbl_2_data_3091C): "window_left"/"window_right".
static const char *const sc_nodeNames[2] = { "window_left", "window_right" };

// fn_2_F5380 (vtable offset 0x280, MIDDLE_BG's OWN override -- CORRECTED this round, was
// mis-attached to BOTTOM_BG). Confirmed content: per node, roll a 10% random visibility flag
// (rndInt(100) < 10 -- semantics confirmed by hand-tracing the target's own cntlzw/slw/srwi
// bit-trick, exact codegen match not yet reproduced), look the node up via mModel's own ResMdl,
// and call setNodeVisibility with the node's own id (0 if not found).
void daMiddleBGForCastleLudwig_c::vf280() {
    nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
    for (int i = 0; i < 2; i++) {
        m_764[i] = dGameCom::rndInt(0x64) < 0xa;
        nw4r::g3d::ResNode node = resMdl.GetResNode(sc_nodeNames[i]);
        d3d::setNodeVisibility(&mModel, node.GetID(), m_764[i]);
    }
}

// fn_2_F5430 (vtable offset 0x284, shared by both classes). Confirmed content: copies
// m_764[i] from \p other (byte-for-byte, same index), then the SAME node-lookup/
// setNodeVisibility idiom as vf280.
void daMiddleBGForCastleLudwig_c::vf284(daMiddleBGForCastleLudwig_c *other) {
    nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
    for (int i = 0; i < 2; i++) {
        m_764[i] = other->m_764[i];
        nw4r::g3d::ResNode node = resMdl.GetResNode(sc_nodeNames[i]);
        d3d::setNodeVisibility(&mModel, node.GetID(), m_764[i]);
    }
}

// fn_2_F54D0. Vtable slot 2 (create) -- confirmed IDENTICAL in both classes' vtables (full
// diff, not eyeballed) but NOT YET SCOUTED. FAKE STUB.
// fn_2_F54D0. Confirmed content: createModel() and vf29c() (both REAL virtual calls, compile
// to the identical vtable dispatch the target's own raw offset calls use), then
// changeState(daBottomBGForCastleLudwig_c::StateID_DemoWait) -- `lbl_2_bss_C1AC`, resolved this
// round by direct __sinit bytes (see MAPPING.md); despite this shared function living on the
// BASE class, the target genuinely references the DERIVED class's own state object, not a
// generic/shared one -- taken from the disassembly as-is, not second-guessed. Then a second raw
// virtual call through the still-unnamed object at offset 0x394 execute() also reaches, this
// time slot 0x1c instead of 0x10.
int daMiddleBGForCastleLudwig_c::create() {
    createModel();
    vf29c();
    changeState(daBottomBGForCastleLudwig_c::StateID_DemoWait);

    typedef void (*StateMgrFn_t)(void *);
    (*(StateMgrFn_t *) ((const u8 *) *(void **) ((u8 *) this + 0x394) + 0x1c))((u8 *) this + 0x394);

    return SUCCEEDED;
}

// fn_2_F5550 (vtable offset 0x298, MIDDLE_BG's OWN createModel override -- CORRECTED this
// round, was mis-attached to BOTTOM_BG). Confirmed content: byte-for-byte the SAME
// createFrmHeap/getRes/GetResMdl/m3d::mdl_c::create/setSoftLight_Map/adjustFrmHeap sequence as
// BOTTOM_BG's own override below, confirmed by direct disasm comparison (identical instruction
// shape, only the string-pool references differ) -- using MIDDLE_BG's own arc/model strings,
// read directly out of this unit's own .data (lbl_2_data_30938/lbl_2_data_30954, decoded
// byte-for-byte, NOT guessed): "g3d/W7_shiroboss_bg_M.brres" / "W7_shiroboss_bg_M".
void daMiddleBGForCastleLudwig_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("W7_shiroboss_bg_M", "g3d/W7_shiroboss_bg_M.brres");
    m_540 = (void *) resFile.ptr();
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("W7_shiroboss_bg_M");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    // Real content found this round -- was missing entirely from the earlier draft (a real
    // content gap, not a scheduling residual, which is why the earlier version was 61/73
    // differing with 29 lines simply absent). A REAL, landed RTTI cast
    // (nw4r::g3d::G3dObj::DynamicCast<T>, include/lib/nw4r/g3d/g3d_obj.h) against `this+0x548`
    // (mModel's own internal G3dObj*), then SetScnObjOption(0x30001, 0) -- both real, landed
    // members of nw4r::g3d::ScnMdl (g3d_scnmdl.h). The `0x30001` option value has no landed
    // named constant (grepped) -- used as the literal the bytes show.
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(*(nw4r::g3d::G3dObj **) ((u8 *) this + 0x548));
    scnMdl->SetScnObjOption(0x30001, 0);

    dActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}

// fn_2_F5680 (vtable offset 0x29c, MIDDLE_BG's OWN override -- CORRECTED this round, was
// mis-attached to BOTTOM_BG). FAKE STUB -- unscouted this round (0x18C bytes target, the
// LARGEST function in the whole unit).
// fn_2_F5680 (vtable offset 0x29c, MIDDLE_BG's OWN override, 0x18C bytes -- the LARGEST
// function in the unit). Read fresh rather than varied, per the coordinator's own threshold
// (size/diff mismatch means missing content, not scheduling). Confirmed content: sets up BOTH
// dBg_ctr_c zones via the real (un-landed-header) `set(dActor_c*, const sBgSetInfo*, u8, u8,
// mVec3_c*)` overload plus `entry()`, using real constants read from this unit's own .rodata
// (`lbl_2_rodata_5BC0`) -- two mVec3_c-shaped fields at `this+0x74c`/`this+0x758` both set to
// mPos, mScale set to a uniform 1.0 constant -- then resets BOTH window nodes' visibility to
// false via the SAME GetResNode/setNodeVisibility idiom vf280/vf284 use (real, confirmed node
// name table).
void daMiddleBGForCastleLudwig_c::vf29c() {
    static const float sc_1 = 1.0f;       // lbl_2_rodata_5BC0+0x4
    static const float sc_n208 = -208.0f; // lbl_2_rodata_5BC0+0x8
    static const float sc_128 = 128.0f;   // lbl_2_rodata_5BC0+0xc
    static const float sc_n176 = -176.0f; // lbl_2_rodata_5BC0+0x10
    static const float sc_n128 = -128.0f; // lbl_2_rodata_5BC0+0x14
    static const float sc_176 = 176.0f;   // lbl_2_rodata_5BC0+0x18
    static const float sc_208 = 208.0f;   // lbl_2_rodata_5BC0+0x1c

    ((mVec3_c *) ((u8 *) this + 0x74c))->set(mPos.x, mPos.y, mPos.z);
    ((mVec3_c *) ((u8 *) this + 0x758))->set(mPos.x, mPos.y, mPos.z);
    mScale.x = sc_1;
    mScale.y = sc_1;
    mScale.z = sc_1;

    sBgSetInfo info0 = { sc_n208, sc_128, sc_n176, sc_n128, 0, 0, 0 };
    mVec3_c vec0(sc_1, sc_1, sc_1);
    set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c(
        &mBgCtr[0], this, &info0, 3, *((const u8 *) this + 0x38f), &vec0);
    mBgCtr[0].entry();

    sBgSetInfo info1 = { sc_176, sc_128, sc_208, sc_n128, 0, 0, 0 };
    set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c(
        &mBgCtr[1], this, &info1, 3, *((const u8 *) this + 0x38f), &vec0);
    mBgCtr[1].entry();

    nw4r::g3d::ResMdl resMdl = mModel.getResMdl();
    for (int i = 0; i < 2; i++) {
        m_764[i] = false;
        nw4r::g3d::ResNode node = resMdl.GetResNode(sc_nodeNames[i]);
        d3d::setNodeVisibility(&mModel, node.GetID(), 0);
    }
}

// fn_2_F5810. Vtable slot 8 (execute) -- FAKE STUB, same caveat as create.
// fn_2_F5810. Confirmed content: a raw virtual call through a still-unnamed object at
// offset 0x394 (slot 4 on ITS OWN vtable -- almost certainly dActorMultiState_c's own
// mStateMgr, since #create's own changeState(...) forwards to something at the SAME offset
// per dActorMultiState_c::changeState()'s own real body, but not independently confirmed by
// field name -- modelled as a raw cast, matching the destructor's own precedent for an
// unnamed base-class member elsewhere on this project), then vf2a0() (a REAL virtual call,
// compiles to the identical vtable dispatch the target's own raw offset-0x2a0 call uses), then
// dBg_ctr_c::calc() on both zones.
int daMiddleBGForCastleLudwig_c::execute() {
    // PARKED: three variants tried (inline expression; a named single local; two named locals
    // matching the target's own two-step instruction order) -- all land on r5 for the adjusted
    // "this" where the target reuses r3, a register-allocation wall, not a content problem.
    typedef void (*StateMgrExecFn_t)(void *);
    (*(StateMgrExecFn_t *) ((const u8 *) *(void **) ((u8 *) this + 0x394) + 0x10))((u8 *) this + 0x394);

    vf2a0();

    for (int i = 0; i < 2; i++) {
        mBgCtr[i].calc();
    }
    return SUCCEEDED;
}

// fn_2_F5890 (vtable offset 0x2a0, shared). Confirmed content: a matrix update -- mMatrix.
// trans(mPos); mModel.setLocalMtx(&mMatrix); mModel.setScale(mScale); -- no rotation, no calc.
void daMiddleBGForCastleLudwig_c::vf2a0() {
    mMatrix.trans(mPos);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
}

// fn_2_F58F0. Vtable slot 5 (doDelete) -- FAKE STUB, same caveat as create.
// fn_2_F58F0. Confirmed content: releases both dBg_ctr_c zones.
int daMiddleBGForCastleLudwig_c::doDelete() {
    for (int i = 0; i < 2; i++) {
        mBgCtr[i].release();
    }
    return SUCCEEDED;
}

// fn_2_F5940. Vtable slot 11 (draw) -- FAKE STUB, same caveat as create.
// fn_2_F5940. Confirmed content: a single virtual call through mModel's own vtable.
int daMiddleBGForCastleLudwig_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// DemoWait, THIS ROUND'S STATE-MACHINE FIX. fn_2_F5980 (vtable offset 0x294) =
// initializeState_DemoWait: confirmed content, empty body. fn_2_F5990 (offset 0x290) =
// executeState_DemoWait: confirmed content, a pure forwarding thunk into mModel's OWN vtable at
// offset 0x1c. fn_2_F5970 (offset 0x28c) = finalizeState_DemoWait: confirmed content, empty
// body. Mapping is the PMF-field read order found in .data (0x294,0x290,0x28c in that order),
// matching STATE_VIRTUAL_DEFINE's own (&initializeState, &executeState, &finalizeState)
// argument order -- see the header's own note. STATE_VIRTUAL_DEFINE itself (the static
// StateID_DemoWait object) is placed after BOTTOM_BG's own dtor at the end of the file since it
// is compiler-generated .data, not .text, and so is not subject to the function-order gate.
void daMiddleBGForCastleLudwig_c::initializeState_DemoWait() {}

void daMiddleBGForCastleLudwig_c::executeState_DemoWait() {
    typedef void (*Vtbl1CFn_t)(m3d::mdl_c *);
    m3d::mdl_c *m = &mModel;
    (*(Vtbl1CFn_t *) ((const u8 *) *(void **) m + 0x1c))(m);
}

void daMiddleBGForCastleLudwig_c::finalizeState_DemoWait() {}

// fn_2_F59A0 (vtable offset 0x298, BOTTOM_BG's OWN createModel override). Confirmed content:
// same idiom as the base class's own override, using BOTTOM_BG's own arc/model strings, read
// directly out of .data (lbl_2_data_30968/lbl_2_data_30984): "g3d/W7_shiroboss_bg_D.brres" /
// "W7_shiroboss_bg_D".
void daBottomBGForCastleLudwig_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("W7_shiroboss_bg_D", "g3d/W7_shiroboss_bg_D.brres");
    m_540 = (void *) resFile.ptr();
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("W7_shiroboss_bg_D");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    // Same real content the base's own override has -- see its own note.
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(*(nw4r::g3d::G3dObj **) ((u8 *) this + 0x548));
    scnMdl->SetScnObjOption(0x30001, 0);

    dActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}

// fn_2_F5AD0 (vtable offset 0x29c, BOTTOM_BG's OWN override). FAKE STUB -- unscouted this round
// (0x128 bytes target).
// fn_2_F5AD0 (vtable offset 0x29c, BOTTOM_BG's OWN override, 0x128 bytes). Same shape as the
// base's own vf29c above but WITHOUT the trailing node-visibility-reset loop (this class's own
// target is 0x128 vs the base's 0x18C, exactly the size of that missing tail).
void daBottomBGForCastleLudwig_c::vf29c() {
    static const float sc_1 = 1.0f;
    static const float sc_n208 = -208.0f;
    static const float sc_128 = 128.0f;
    static const float sc_n176 = -176.0f;
    static const float sc_n128 = -128.0f;
    static const float sc_176 = 176.0f;
    static const float sc_208 = 208.0f;

    ((mVec3_c *) ((u8 *) this + 0x74c))->set(mPos.x, mPos.y, mPos.z);
    ((mVec3_c *) ((u8 *) this + 0x758))->set(mPos.x, mPos.y, mPos.z);
    mScale.x = sc_1;
    mScale.y = sc_1;
    mScale.z = sc_1;

    sBgSetInfo info0 = { sc_n208, sc_128, sc_n176, sc_n128, 0, 0, 0 };
    mVec3_c vec0(sc_1, sc_1, sc_1);
    set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c(
        &mBgCtr[0], this, &info0, 3, *((const u8 *) this + 0x38f), &vec0);
    mBgCtr[0].entry();

    sBgSetInfo info1 = { sc_176, sc_128, sc_208, sc_n128, 0, 0, 0 };
    set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c(
        &mBgCtr[1], this, &info1, 3, *((const u8 *) this + 0x38f), &vec0);
    mBgCtr[1].entry();
}

// fn_2_F5C00 (vtable offset 0x288, shared). Confirmed content: a pure forwarding thunk,
// tail-calling THIS SAME object's own vf2a0.
void daMiddleBGForCastleLudwig_c::vf288() { vf2a0(); }

// fn_2_F5C10 (vtable offset 0x280, BOTTOM_BG's OWN override -- CORRECTED this round, was
// mis-attached to the base class). Confirmed content: empty body -- genuinely a DIFFERENT
// function from the base's own vf280 (fn_2_F5380, 0x98 bytes there), not shared.
void daBottomBGForCastleLudwig_c::vf280() {}

// fn_2_F5C20. Confirmed content: the standard empty-body derived destructor, chaining to the
// base's own destructor with the shared one-slot flag-argument shape -- byte-for-byte the same
// pattern already landed on DUMMY_DOOR_CHILD/PARENT.
daBottomBGForCastleLudwig_c::~daBottomBGForCastleLudwig_c() {}

// STATE_VIRTUAL_DEFINE -- compiler-generated .data (the static StateID_DemoWait object and its
// own template machinery), not .text, so not subject to the function-order gate.
STATE_VIRTUAL_DEFINE(daMiddleBGForCastleLudwig_c, DemoWait);

// BOTTOM_BG's OWN DemoWait -- same bodies as the base's own (confirmed by __sinit copying the
// SAME three PMF addresses), declared separately only because STATE_VIRTUAL_FUNC_DECLARE
// requires it for the vtable-slot machinery; the actual behavior is identical.
void daBottomBGForCastleLudwig_c::initializeState_DemoWait() {}

void daBottomBGForCastleLudwig_c::executeState_DemoWait() {
    typedef void (*Vtbl1CFn_t)(m3d::mdl_c *);
    m3d::mdl_c *m = &mModel;
    (*(Vtbl1CFn_t *) ((const u8 *) *(void **) m + 0x1c))(m);
}

void daBottomBGForCastleLudwig_c::finalizeState_DemoWait() {}

// Hand-expansion of STATE_VIRTUAL_DEFINE's own object-definition line (NOT the macro itself --
// see the header's own note on why the macro cannot be invoked a second time for this state
// name). The base-state argument is `daMiddleBGForCastleLudwig_c::StateID_DemoWait` directly --
// exactly what the macro's own `baseID_DemoWait<StateIDBase_DemoWait>()` would have resolved to,
// since the base class DOES declare "DemoWait". The name string is confirmed from __sinit's own
// bytes to be the SAME string as the base's own object uses (not a new
// "daBottomBGForCastleLudwig_c::StateID_DemoWait" one) -- used as the bytes say, not as the
// macro would have generated it.
sFStateVirtualID_c<daBottomBGForCastleLudwig_c> daBottomBGForCastleLudwig_c::StateID_DemoWait(
    daMiddleBGForCastleLudwig_c::StateID_DemoWait,
    "daMiddleBGForCastleLudwig_c::StateID_DemoWait",
    &daBottomBGForCastleLudwig_c::initializeState_DemoWait,
    &daBottomBGForCastleLudwig_c::executeState_DemoWait,
    &daBottomBGForCastleLudwig_c::finalizeState_DemoWait);
