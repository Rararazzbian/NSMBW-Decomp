#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_castle_bg.hpp>

// MIDDLE_BG_FOR_CASTLE_LUDWIG and BOTTOM_BG_FOR_CASTLE_LUDWIG share this one translation unit
// (coordinator-scoped: .text 0xf5130-0xf6150, 0x1020 bytes, ONE .ctors entry at 0xf5c80, owns
// .rodata 0x5bc0, .data 0x308f8-0x30f34, .bss 0xc1a0-0xc1ac). THIS ROUND: the class skeleton --
// both classInits, the shared ctor, both destructors, and 7 of the 9 new virtuals -- is real,
// confirmed content. create()/doDelete()/execute()/draw() and 2 of the new virtuals are FAKE
// STUBS (clearly marked below, each at its own correct address slot so the order gate stays
// meaningful); roughly 13 more functions in-range (including the `.ctors`-registered `__sinit`
// itself) are scouted only by size/address so far and are NOT YET present in this draft at all
// (absent, not stubbed in the wrong slot -- matching this project's own established
// convention). See MAPPING.md for the full inventory and round-by-round plan.
//
// NOTE ON DEFINITION ORDER: every function below is placed at its correct target-address slot,
// verified with wip/wm_units/check_fn_order.py's own general check plus this unit's own
// build.py (which runs verify_anon.py's ascending-pairing, the only order check that can see a
// real-named draft against fully anonymous targets).

ACTOR_PROFILE(MIDDLE_BG_FOR_CASTLE_LUDWIG, daMiddleBgForCastleLudwig_c, 0);

ACTOR_PROFILE(BOTTOM_BG_FOR_CASTLE_LUDWIG, daBottomBgForCastleLudwig_c, 0);

// fn_2_F51B0. Confirmed content: `return &sStateID::null;` -- see the header's own note.
const sStateID_c &daMiddleBgForCastleLudwig_c::getNullState() { return sStateID::null; }

// fn_2_F51C0. Shared base ctor (used directly by MIDDLE_BG, and via #daBottomBgForCastleLudwig_c
// for BOTTOM_BG). Confirmed content: base #dEn_c ctor (implicit vtable-fixup to THIS class's own
// vtable, inlined here per the established DUMMY_DOOR precedent), then #mAllocator/#mModel's own
// ctors, then `__construct_array` builds #mBgCtr[2] (`sizeof(dBg_ctr_c)==0xe4`, confirmed against
// the real landed header).
daMiddleBgForCastleLudwig_c::daMiddleBgForCastleLudwig_c() : m_540(nullptr) {}

// fn_2_F5240. Base destructor -- confirmed content: `__destroy_arr`s #mBgCtr[2], then
// #mModel/#mAllocator's own dtors, then the base #dEn_c dtor, matching the standard
// one-slot-flag-argument destructor shape already established on DUMMY_DOOR.
daMiddleBgForCastleLudwig_c::~daMiddleBgForCastleLudwig_c() {}

// fn_2_F52D0. Confirmed content: touches ONLY #mBgCtr[0] (not #mBgCtr[1]), a tail call either
// way -- `if (doEntry) mBgCtr[0].entry(); else mBgCtr[0].release();`. NOT a vtable slot (no
// `fn_2_F52D0` entry anywhere in either target vtable dump).
void daMiddleBgForCastleLudwig_c::entryOrRelease(bool doEntry) {
    if (doEntry) {
        mBgCtr[0].entry();
    } else {
        mBgCtr[0].release();
    }
}

// fn_2_F5430 (vtable offset 0x284). FAKE STUB -- already scouted (a 2-entry visibility-byte
// copy from another instance of this same class, plus GetResNode/setNodeVisibility calls
// against a 2-entry `lbl_2_data_30930` name table) but not yet authored as real content.
void daMiddleBgForCastleLudwig_c::vf284(daMiddleBgForCastleLudwig_c *other) {}

// fn_2_F54D0. Vtable slot 2 (create) -- confirmed OVERRIDDEN (not inherited) but NOT YET
// SCOUTED this round. FAKE STUB -- `1` is a placeholder SUCCEEDED-shaped return, not confirmed
// content.
int daMiddleBgForCastleLudwig_c::create() { return 1; }

// fn_2_F5810. Vtable slot 8 (execute) -- FAKE STUB, same caveat as #create.
int daMiddleBgForCastleLudwig_c::execute() { return 1; }

// fn_2_F5890 (vtable offset 0x2a0). Confirmed content: a matrix update -- `mMatrix.trans(mPos);
// mModel.setLocalMtx(&mMatrix); mModel.setScale(mScale);` -- notably WITHOUT the ZXYrotM
// rotation step or a trailing `mModel.calc()` call that the fuller "matrix update" idiom
// elsewhere on this project has, confirmed from the target's own instruction sequence (no
// rotation-shaped code and no third model call anywhere in this function).
void daMiddleBgForCastleLudwig_c::vf2a0() {
    mMatrix.trans(mPos);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
}

// fn_2_F58F0. Vtable slot 5 (doDelete) -- FAKE STUB, same caveat as #create.
int daMiddleBgForCastleLudwig_c::doDelete() { return 1; }

// fn_2_F5940. Vtable slot 11 (draw) -- FAKE STUB, same caveat as #create.
int daMiddleBgForCastleLudwig_c::draw() { return 1; }

// fn_2_F5970 (vtable offset 0x28c). Confirmed content: empty body -- BINDING requires this be
// defined out-of-line (see the header's own note) despite having no logic at all.
void daMiddleBgForCastleLudwig_c::vf28c() {}

// fn_2_F5980 (vtable offset 0x294). Confirmed content: empty body, same as #vf28c.
void daMiddleBgForCastleLudwig_c::vf294() {}

// fn_2_F5990 (vtable offset 0x290). Confirmed content: a pure forwarding thunk into #mModel's
// OWN vtable at offset 0x1c (`lwzu r12,0x544(r3); lwz r12,0x1c(r12); mtctr r12; bctr` -- the
// `lwzu` both loads #mModel's vtable pointer AND adjusts `this` to `&mModel` for the tail
// call). Modelled as a raw vtable-slot call since #m3d::mdl_c's own slot-0x1c method has no
// landed name to call by; matches the destructor's own raw-cast precedent elsewhere on this
// project for an unnamed base-class member.
void daMiddleBgForCastleLudwig_c::vf290() {
    typedef void (*Vtbl1CFn_t)(m3d::mdl_c *);
    m3d::mdl_c *m = &mModel;
    (*(Vtbl1CFn_t *) ((const u8 *) *(void **) m + 0x1c))(m);
}

// fn_2_F59A0 (vtable offset 0x298, #createModel). Confirmed content: the standard
// createFrmHeap/getRes/GetResMdl/m3d::mdl_c::create/setSoftLight_Map/adjustFrmHeap idiom
// already established on other landed actors, just sourced from this class's own #m_540
// cache instead of a fresh local -- the actual string arguments are NOT YET confirmed (this
// unit's own `.rodata`/`.data` string pool has not been read this round), so `"CASTLE_BG"` is
// a clearly-marked placeholder, not a real value.
void daMiddleBgForCastleLudwig_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("CASTLE_BG", "g3d/CASTLE_BG.brres");
    m_540 = (void *) resFile.ptr();
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("CASTLE_BG");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
    dActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}

// fn_2_F5AD0 (vtable offset 0x29c). FAKE STUB -- unscouted this round (0x128 bytes).
void daMiddleBgForCastleLudwig_c::vf29c() {}

// fn_2_F5C00 (vtable offset 0x288). Confirmed content: a pure forwarding thunk, tail-calling
// THIS SAME object's own #vf2a0 -- decoded directly from the target (`lwz r12,0x60(r3);
// lwz r12,0x2a0(r12); mtctr r12; bctr`, a raw vtable-slot-to-vtable-slot forward, not a guessed
// equivalent).
void daMiddleBgForCastleLudwig_c::vf288() { vf2a0(); }

// fn_2_F5C10 (vtable offset 0x280). Confirmed content: empty body, same shape as #vf28c/#vf294.
void daMiddleBgForCastleLudwig_c::vf280() {}

// fn_2_F5C20. Confirmed content: the standard empty-body derived destructor, chaining to the
// base's own (#daMiddleBgForCastleLudwig_c's dtor) with the shared one-slot flag-argument shape
// -- byte-for-byte the same pattern already landed on DUMMY_DOOR_CHILD/PARENT.
daBottomBgForCastleLudwig_c::~daBottomBgForCastleLudwig_c() {}
