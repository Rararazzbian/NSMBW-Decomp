#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_wm_lib.hpp>

/// @brief The world map actor for the sunken pirate ship course icon (World 7).
/// @details A near-twin of #daWmSandPillar_c/#daWmSmallCloud_c's minimal shape, but simpler than
/// both: no per-Type state machine (unlike sandpillar) and no bgm-sync/animation set (unlike
/// smallcloud) -- just a single static model repositioned every frame to its
/// #dWmObjActor_c::mResNodeIdx course node.
/// @unofficial Reconstructed from anonymous (unnamed) target symbols; class name and member names
/// are inferred from codegen evidence, not from any mangled name. `sizeof(daWmSinkShip_c) == 0x1b0`,
/// read directly off the allocator wrapper (`fn_2_179380`: `li r3, 0x1b0; bl __nw__7fBase_cFUl`).
///
/// Class hierarchy confirmed directly from the target constructor (`fn_2_1793B0`): it calls
/// `bl __ct__14dWmDemoActor_cFv` (the grand-base ctor, NOT `dWmObjActor_c`'s -- that one is
/// entirely inline and gets folded into this ctor instead of emitting its own `bl`), installs this
/// class's OWN vtable (`lbl_2_data_471E0`) at +0x60 (the dBase_c secondary-vtable-pointer slot,
/// see the antlion_mng finding), and stores `-1` at +0x184 BEFORE constructing `mAllocator` at
/// +0x188 -- that store is `dWmObjActor_c`'s own `mResNodeIdx(-1)` in-class member initializer
/// being inlined (same idiom as its `-inline noauto` member-function inlining), not something this
/// class writes itself. Confirmed conclusively by the target's own vtable dump
/// (`lbl_2_data_471E0`, read directly out of `bin/dtkspl/d_basesNP/obj/auto_04_00046BE0_data.o`):
/// the `GetActorType` slot resolves to the IMPORTED `GetActorType__13dWmObjActor_cFv`, and two
/// trailing slots resolve to `vf74__13dWmObjActor_cFv`/`vf78__13dWmObjActor_cFv` -- both
/// `dWmObjActor_c`-only symbols with no `dWmDemoActor_c` equivalent, so `daWmSinkShip_c` derives
/// from `dWmObjActor_c`, not `dWmDemoActor_c` directly.
///
/// Member offsets read directly from the constructor/destructor:
///   dWmObjActor_c base (incl. mResNodeIdx)   ends 0x188
///   dHeapAllocator_c mAllocator               +0x188  (ctor: __ct__16dHeapAllocator_cFv)
///   m3d::smdl_c mModel                        +0x1a4  (ctor: __ct__Q23m3d6smdl_cFv)
/// 0x1a4 + sizeof(m3d::smdl_c) == 0x1b0, the classInit operand.
///
/// The destructor (`fn_2_179410`) destructs mModel(+0x1a4) then mAllocator(+0x188) -- reverse
/// declaration order -- then, on the deleting-destructor path, destructs +0x158/+0x13c
/// (`dWmDemoActor_c`'s OWN `mModel`/`mHeapAllocator`, both inline-empty dtors folded in, the same
/// vague-linkage idiom documented for antlion_mng) before calling `__dt__10dWmActor_cFv`.
///
/// `execute()` (`fn_2_179510`) does NOT override `processCutsceneCommand` -- the target's vtable
/// slot for it resolves to the IMPORTED `processCutsceneCommand__14dWmDemoActor_cFib`, so the call
/// here dispatches virtually through the inherited base implementation. It also does not gate on
/// `dCsSeqMng_c::FUN_80915600()` the way `daWmSmallCloud_c::execute()` does -- the call is
/// unconditional in the target.
///
/// `mResNodeIdx` is used directly as the `GetNodePos(long, mVec3_c&)` node index (NOT the
/// string-name overload smallcloud/sandpillar use for their own course-node lookups) -- confirmed
/// by the mangled callee, `GetNodePos__9daWmMap_cFlR7mVec3_c`.
///
/// `createModel()` (`fn_2_1795D0`) uses a FIXED archive/model name, "cobSunkenShip" -- read
/// directly out of the target's own .data (`lbl_2_data_471D0`, size 0xE, confirmed ASCII) --
/// reused for both `dResMng_c::getRes()`'s archive parameter and `ResFile::GetResMdl()`'s model
/// name, unlike smallcloud's per-world `sprintf`'d archive name. `mAllocator.createFrmHeap`'s
/// resulting model handle is a LOCAL, not a persistent member -- there is no third stored offset
/// anywhere in this unit for a `mResFile`-shaped field. It also calls
/// `dWmActor_c::setSoftLight_MapObj` (the "_MapObj" overload), not smallcloud's "_Map" one --
/// confirmed by the mangled callee name.
///
/// `create()`'s clip-sphere radius (`fn_2_1794B0`) is a plain float literal, confirmed 100.0f by
/// direct decode of the target's own merged rodata pool (`lbl_2_rodata_8F98`, offset 0: `0x42C80000`
/// == 100.0f). The pool's remaining three floats (offset 4/8/c: 2160.0f/-30.0f/-478.0f) are
/// `dWmLib::sc_ForceList`'s own `mNodePos` initializer, confirming this TU's `__sinit`
/// (`fn_2_179730`) emits its own private copy of `dWmLib::sc_ForceList` and
/// `dWmLib::c_StartPointKinokoHouseID` purely because `d_wm_lib.hpp` is included here -- both are
/// non-`extern` namespace-scope statics with non-trivial construction, so every TU that includes
/// the header gets its own; this unit's copy needs no source-level reference to either symbol to be
/// forced into existence (see AGENT_CONTEXT.md, "An object nobody references can still be
/// required"). `fn_2_1797C0` (this unit's last function) is the matching array destructor for that
/// same private copy, placed after `__sinit` per the standard rule.
class daWmSinkShip_c : public dWmObjActor_c {
public:
    daWmSinkShip_c();
    ~daWmSinkShip_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    /// @unofficial fn_2_1795D0. See class-level doc comment.
    void createModel();
    /// @unofficial fn_2_179680. Identical shape to daWmSmallCloud_c::calcModel() /
    /// daWmSandPillar_c's own equivalent: stage mMatrix from mPos/mAngle, hand it and mScale to
    /// mModel, then calc(false).
    void calcModel();

    dHeapAllocator_c mAllocator; ///< This class's OWN allocator, @ 0x188 (distinct from
                                 ///< dWmObjActor_c's inherited dWmDemoActor_c::mHeapAllocator).
    m3d::smdl_c mModel; ///< This class's OWN model, @ 0x1a4 (distinct from the inherited
                        ///< dWmDemoActor_c::mModel). Total size 0x1b0 (measured).
};

ACTOR_PROFILE(WM_SINKSHIP, daWmSinkShip_c, 0);

daWmSinkShip_c::daWmSinkShip_c() {}
daWmSinkShip_c::~daWmSinkShip_c() {}

int daWmSinkShip_c::create() {
    createModel();
    mClipSphere.set(mPos, 100.0f);
    calcModel();
    return SUCCEEDED;
}

int daWmSinkShip_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    calcModel();

    return SUCCEEDED;
}

int daWmSinkShip_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmSinkShip_c::doDelete() {
    return SUCCEEDED;
}

void daWmSinkShip_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobSunkenShip", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobSunkenShip");
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmSinkShip_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}
