#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>

/// @unofficial `lbl_2_data_436A8`, this unit's own .data at offset 0x10
/// (right before dWmLib::sc_ForceList's own F7C0/W7C0 pair) -- a plain
/// NAMED pointer variable holding "cobKoopaShip" (`lbl_2_data_43698`, this
/// unit's own leading pooled string at offset 0). Both setNodePos() and
/// execute() load the node name through THIS variable (`lwz r4,
/// lbl_2_data_436A8@l(r4)`), not through the string literal's own address
/// directly -- confirmed directly from the target, both call sites. Declared
/// here, before the `d_wm_lib.hpp` include, so it emits into .data ahead of
/// dWmLib::sc_ForceList's own strings, matching the target's offset.
static const char *smc_koopaShipNodeName = "cobKoopaShip";

#include <game/bases/d_wm_lib.hpp>

/// @unofficial DRAFT, not yet fully verified against every target instruction.
///
/// Class hierarchy confirmed directly from the target constructor
/// (fn_2_15A5D0): it calls `__ct__14dWmDemoActor_cFv` DIRECTLY (not
/// `__ct__13dWmObjActor_cFv`), so `daWmAnchor_c : public dWmDemoActor_c`,
/// NOT `dWmObjActor_c` (the kinoko/cannon/grid/tower base). This is the whole
/// explanation for the seven real-named symbols in this unit's tail
/// (doDelete/checkCutEnd/setCutEnd/clearCutEnd/vf74/vf78/GetActorType,
/// 0x15AB60-0x15ABD0 that the coordinator flagged): reading this unit's own
/// vtable directly (`lbl_2_data_43710` in
/// bin/dtkspl/d_basesNP/obj/auto_04_0003A960_data.txt) shows ALL SEVEN sit in
/// THIS class's own vtable, unoverridden -- doDelete/checkCutEnd/setCutEnd/
/// clearCutEnd inherited from dWmDemoActor_c directly, vf74/vf78/GetActorType
/// inherited further up, from dWmActor_c (dWmDemoActor_c's own base, and
/// dWmObjActor_c's independently, which is why dtk's symbol map -- built by
/// matching byte-identical content against already-landed dWmObjActor_c-family
/// units like kinoko -- mislabels these three as `13dWmObjActor_c` even
/// though this class never derives it. None of these seven get declared
/// below; they simply inherit.
///
/// `fn_2_15ABC0` (`li r3,0x1; blr`, right before this unit's own `__sinit`)
/// is a DIFFERENT, unrelated weak copy -- NOT referenced by this class's own
/// vtable (its `doDelete` slot points at 0x15A8A0, not 0x15ABC0) -- matching
/// the precedent already on record: AGENT_CONTEXT documents this exact
/// address as `d_a_wm_sandpillar.cpp`'s own placed copy of
/// `dWmDemoActor_c::doDelete()`. Left out of this draft entirely.
///
/// sizeof(daWmAnchor_c) == 0x1f4, read directly off the allocator wrapper
/// (fn_2_15A5A0: `li r3, 0x1f4; bl __nw__7fBase_cFUl`).
///
/// Member offsets read directly from the constructor (fn_2_15A5D0) and
/// createModel()/calcModel() (fn_2_15A8B0/fn_2_15A960):
///   dWmDemoActor_c base        ends 0x184
///   int mUnk184                 +0x184  (ctor: `li r0,-1; stw r0,0x184`)
///   dHeapAllocator_c mAllocator +0x188  (ctor: __ct__16dHeapAllocator_cFv)
///   nw4r::g3d::ResFile mResFile +0x1a4  (ctor: `li r0,0; stw r0,0x1a4`;
///                                        createModel() stores getRes()'s
///                                        return there directly)
///   m3d::smdl_c mModel          +0x1a8  (ctor: __ct__Q23m3d6smdl_cFv)
///   m3d::anmChr_c mUnusedAnim[1] +0x1b4 (ctor: __construct_array, 1 elem,
///                                        0x38 B each -- NOT read by any of
///                                        create/execute/draw/calcModel/
///                                        createModel; kept as an unused
///                                        member, same idiom as
///                                        daWmCannon_c::mUnk200)
///   (unaccounted, 4 B)           +0x1ec  raw padding -- 0x1ec to 0x1f0 is
///                                        never touched by the ctor or by
///                                        any function read so far
///   int mUnk1f0                 +0x1f0  (set to 0 by fn_2_15AAF0, called
///                                        from the create()-only helper
///                                        below)
/// 0x1f0 + 4 == 0x1f4, matching sizeof exactly.
///
/// UNRESOLVED, flagged rather than guessed: `fn_2_15AA10` (called once, from
/// create()) and part of execute()'s own body both open with the identical
/// sequence -- look up a world-map node whose model name is "cobKoopaShip"
/// (`lbl_2_data_43698`, read directly out of
/// bin/dtkspl/d_basesNP/obj/auto_04_0003A960_data.txt) via a DOL cross-module
/// call (`fn_80100640`, unnamed in both `bin/dtk/wiimj2d_symbols.txt` and
/// this REL's own map, exactly the `extern "C"` FUN-address situation
/// AGENT_CONTEXT documents for `fn_80103420` in the kinoko family), then call
/// `daWmMap_c::GetNodePos`. `fn_2_15AA10` goes on to call
/// `dWmMapModel_c::setAnchorShadow(true)` on the resolved node (strong
/// evidence this really is a "boat anchor" marker tied to a Koopa-airship/
/// tank map node) and to reset `mScale` and `mUnk1f0`. The exact C++ shape of
/// the node-name-to-position resolution (argument roles of `fn_80100640`,
/// and why the same few instructions appear twice rather than through one
/// shared call) is NOT settled below -- see the agent report.
extern "C" void *fn_80100640(daWmMap_c *map, const char *name, int unused);

/// @unofficial dWmMapModel_c::setAnchorShadow(bool). REL-internal (not a DOL
/// cross-module call like fn_80100640), so this is NOT the extern "C"
/// FUN-address convention -- it is a plain forward declaration matching the
/// real mangled name, used because the receiver this call computes is NOT
/// `&daWmMap_c::m_instance->mModels[idx]` (dWmMapModel_c has the wrong
/// size for that -- see setNodePos() below for the arithmetic and the
/// evidence). Confined to this .cpp; no shared header is touched.
extern "C" void setAnchorShadow__13dWmMapModel_cFb(void *thisPtr, bool anchor);

class daWmAnchor_c : public dWmDemoActor_c {
public:
    daWmAnchor_c();
    virtual ~daWmAnchor_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    /// @unofficial Explicit out-of-line override with a body IDENTICAL to
    /// dWmDemoActor_c's own inherited default (`return SUCCEEDED;`). Per
    /// HANDOFF.md's "definition order sets .text placement, but only among
    /// STRONG functions" rule: an explicit override joins the definition-
    /// order batch even with an identical body, while a purely inherited
    /// virtual defers to a block at the end. The target places this one
    /// (0x15a8a0) immediately after draw() -- confirmed content-identical
    /// to dWmDemoActor_c::doDelete() (`li r3,0x1; blr`), so the override
    /// exists ONLY to pin its position, not to change behaviour.
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    /// @unofficial Round 3 on the weak-cluster ordering. Declaring
    /// GetActorType() in-class inline alone got both its body AND its
    /// position right (target 0x15abb0, at the very end). Declaring
    /// clearCutEnd()+vf74() in-class inline alone moved THEM but did not
    /// reproduce the target's full sequence, while checkCutEnd()/vf78()
    /// (left purely inherited that round) stayed inert. Conclusion: a
    /// purely inherited virtual gives nothing to order; a DECLARED in-class
    /// inline override is weak (still defers to the end-of-TU block, same
    /// mechanism the kinoko family's getModelName() needed) but is NOT
    /// inert -- declaration reaches its emission position the same way
    /// declaration order already reaches vtable slot assignment.
    ///
    /// So: all six declared in-class inline here, bodies IDENTICAL to what
    /// each would otherwise inherit, in the TARGET's own address order
    /// (0x15ab60-0x15abb0): setCutEnd, clearCutEnd, checkCutEnd, vf78,
    /// vf74, GetActorType. If declaration order drives weak-emission order
    /// the same way it drives vtable slots, this reproduces the target
    /// exactly; if it does not, that's a clean negative -- see agent report
    /// for the measured draft.txt order this produced.
    virtual int GetActorType() { return ACTOR_MAP_OBJECT; }
    virtual void vf74() {}
    virtual bool vf78() { return false; }
    virtual bool checkCutEnd() { return mIsCutEnd; }
    virtual void clearCutEnd() { mIsCutEnd = false; }
    virtual void setCutEnd() { mIsCutEnd = true; }

    void createModel();
    void calcModel();
    void setNodePos(); ///< @unofficial fn_2_15AA10, best-effort, NOT verified.

    /// @unofficial fn_2_15AB00. Empty body (`blr`, no `li r3`), reached ONLY
    /// through the pointer-to-member table below -- confirmed directly:
    /// `lbl_2_rodata_8574` (bin/dtkspl/d_basesNP/obj/auto_03_00006D00_rodata.o,
    /// this unit's own .rodata, 0x8570-0x8598) is a single 0xC-byte
    /// pointer-to-member entry `{0x0, -1, fn_2_15AB00}` -- the CW ABI
    /// encoding for a non-virtual member function pointer -- and execute()
    /// (fn_2_15A770) calls `(this->*table[mUnk1f0])()` through it via
    /// `__ptmf_scall`, using `mUnk1f0` (+0x1f0) as the index. Named `state_0`
    /// on the working theory this is a one-entry state-dispatch table (only
    /// entry 0 exists in the visible range; mUnk1f0 is always reset to 0 by
    /// fn_2_15AAF0). NOT confirmed against a virtual slot -- this is a
    /// plain, non-virtual member.
    void state_0();

    typedef void (daWmAnchor_c::*StateFunc_t)();
    static const StateFunc_t scStateTable[1]; ///< @unofficial lbl_2_rodata_8574.

    int mUnk184;                    ///< @unofficial 0x184. -1 in the ctor.
    dHeapAllocator_c mAllocator;     ///< @unofficial 0x188
    nw4r::g3d::ResFile mResFile;     ///< @unofficial 0x1a4
    m3d::smdl_c mModel;              ///< @unofficial 0x1a8
    m3d::anmChr_c mUnusedAnim[1];    ///< @unofficial 0x1b4. @unused -- never
                                      ///< read by any function in this unit.
    u8 mUnk1ec[4];                   ///< @unofficial 0x1ec. Unaccounted pad.
    int mUnk1f0;                     ///< @unofficial 0x1f0. State index; 0 via
                                      ///< fn_2_15AAF0. See state_0() above.
};

const daWmAnchor_c::StateFunc_t daWmAnchor_c::scStateTable[1] = { &daWmAnchor_c::state_0 };

ACTOR_PROFILE(WM_ANCHOR, daWmAnchor_c, 0);

daWmAnchor_c::daWmAnchor_c() : mUnk184(-1) {}

daWmAnchor_c::~daWmAnchor_c() {}

int daWmAnchor_c::create() {
    createModel();
    mClipSphere.set(mPos, 500.0f);
    calcModel();
    setNodePos();

    return SUCCEEDED;
}

int daWmAnchor_c::execute() {
    if (dCsSeqMng_c::ms_instance->FUN_80915600()) {
        processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);
    } else {
        (this->*scStateTable[mUnk1f0])();
    }

    /// @unofficial Same "cobKoopaShip" node-name lookup as setNodePos()
    /// below (fn_80100640 + GetNodePos(const char*, mVec3_c&)), but with no
    /// fallback-to-constant branch here. Best-effort, NOT verified byte-exact.
    {
        void *found = fn_80100640(daWmMap_c::m_instance, smc_koopaShipNodeName, 0);
        int off = found ? *reinterpret_cast<int *>(reinterpret_cast<u8 *>(found) + 8) : 0;
        const char *name = off ? reinterpret_cast<const char *>(reinterpret_cast<u8 *>(found) + off) : nullptr;
        daWmMap_c::m_instance->GetNodePos(name, mPos);
    }

    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmAnchor_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmAnchor_c::doDelete() {
    return SUCCEEDED;
}

void daWmAnchor_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("cobAnchor", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobAnchor");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);
    dWmActor_c::setSoftLight_MapObj(mModel);

    mAllocator.adjustFrmHeap();
}

void daWmAnchor_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

/// @unofficial fn_2_15AA10. Best-effort reconstruction, NOT verified
/// byte-exact -- see the class-level comment for what is unresolved
/// (fn_80100640's exact signature/semantics, and the dWmMapModel_c
/// `setAnchorShadow(bool)` call, which is not modelled here at all because
/// dWmMapModel_c is currently an opaque `u8 mPad[0xbf8]` in the shared
/// header and adding the method needs a proposed header change this draft
/// does not make).
void daWmAnchor_c::setNodePos() {
    daWmMap_c *map = daWmMap_c::m_instance;
    void *found = fn_80100640(map, smc_koopaShipNodeName, 0);
    if (found) {
        int off = *reinterpret_cast<int *>(reinterpret_cast<u8 *>(found) + 8);
        const char *name = off ? reinterpret_cast<const char *>(reinterpret_cast<u8 *>(found) + off) : nullptr;
        map->GetNodePos(name, mPos);
    } else {
        mPos.x = mPos.y = mPos.z = 0.0f;
    }

    /// @unofficial dWmMapModel_c::setAnchorShadow(true) on the resolved map
    /// node. The target computes
    /// `daWmMap_c::m_instance + currIdx*0xbf8 + 0x1a0` and calls
    /// `setAnchorShadow__13dWmMapModel_cFb` on THAT address. The mangled
    /// name says the receiver IS a `dWmMapModel_c*` -- which means `0xbf8`
    /// is the size of some LARGER struct that CONTAINS a `dWmMapModel_c` at
    /// +0x1a0, not the size of `dWmMapModel_c` itself. Our shared header's
    /// `dWmMapModel_c { u8 mPad[0xbf8]; }` is therefore modelling the WRONG
    /// object -- flagged to the coordinator, declined for a header change
    /// (3 landed units already reference it; not worth the risk for one
    /// function in a unit with other blockers). Modelled here with a local
    /// cast confined to this .cpp instead, per the coordinator's direction
    /// (matching what WM_NOTE's agent did in the same situation).
    {
        u8 *node = reinterpret_cast<u8 *>(daWmMap_c::m_instance)
                 + daWmMap_c::m_instance->currIdx * 0xbf8 + 0x1a0;
        setAnchorShadow__13dWmMapModel_cFb(node, true);
    }

    mScale.x = mScale.y = mScale.z = 1.0f;
    mUnk1f0 = 0;
}

void daWmAnchor_c::state_0() {}

void daWmAnchor_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    if (!isStaff()) {
        mIsCutEnd = true;
    }
}
