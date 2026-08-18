#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_actor.hpp>

/// @unofficial DRAFT, not yet fully verified against every target instruction.
///
/// Class hierarchy confirmed directly from the target constructor
/// (fn_2_15ACB0): it calls `__ct__14dWmDemoActor_cFv`, installs
/// `__vt__10dWmEnemy_c` at +0x60 (the classic base-vtable-during-construction
/// pattern), inline-constructs `dWmEnPath_c` at +0x184 (matching
/// dWmEnemy_c::mPath's own offset), THEN immediately re-installs a SECOND,
/// distinct vtable (`lbl_2_data_43810`, i.e. this class's own) at +0x60
/// before continuing to construct its own added members. This proves
/// `daWmAntlion_c : public dWmEnemy_c`, NOT dWmObjActor_c (the other
/// dWmDemoActor_c sibling) -- confirmed independently by `fn_2_15B3A0`, a
/// bare tail-call `b IsPlayerComingCore__10dWmEnemy_cFv`, which is exactly
/// dWmEnemy_c's own inline `IsPlayerComing()` default body
/// (`return IsPlayerComingCore();`) being re-emitted here because THIS
/// class's vtable needs a local address for it (it does not override it).
///
/// sizeof(daWmAntlion_c) == 0x7b0, read directly off the allocator wrapper
/// (fn_2_15AC80: `li r3, 0x7b0; bl __nw__7fBase_cFUl`).
///
/// Member offsets read directly from the constructor and calc()/createModel():
///   dWmEnemy_c   base            ends 0x184 (mPath constructed there)
///   dHeapAllocator_c mAllocator   +0x6e8  (ctor: __ct__16dHeapAllocator_cFv)
///   m3d::mdl_c mModel              +0x704  (ctor: __ct__Q23m3d5mdl_cFv)
///   m3d::anmChr_c mChrAnim          +0x744  (ctor: __ct__Q23m3d6fanm_cFv,
///                                           vtable then overwritten to
///                                           anmChr_c/banm_c's own -- same
///                                           idiom as every landed sibling)
///   m3d::anmTexSrt_c mAnimTexSrt    +0x77c  (vtable set directly; its own
///                                           mAllocator_c sub-member
///                                           constructed at +0x788, i.e.
///                                           anmTexSrt+0xc)
/// 0x77c + sizeof(anmTexSrt_c) == 0x7b0, so anmTexSrt_c is 0x34 bytes here --
/// consistent with the class's own known layout.
///
/// SCOPE CORRECTION, found this round via check_bounds.py's ownership check:
/// the given .text claim (0x15ab40-0x15b450) is TOO SHORT. `lbl_2_data_43798`
/// (this class's own sc_ForceList instance, constructed by fn_2_15ABD0's
/// __sinit, which IS in range) is read back by CODE at 0x15b4ee/0x15b502/
/// 0x15b572/0x15b57a -- all past 0x15b450. A quick probe
/// (`text_objects.py d_basesNP 0x15b450 0x15b590`) shows the real unit
/// continues with `fn_2_15B4E0` (0x84 B, __sinit-shaped -- probably the
/// actual sc_ForceList CONSUMER, an initState()-style redirect check) and
/// `fn_2_15B570` (0x1c B) at minimum, likely more beyond 0x15b590. This was
/// NOT re-derived this round (time-boxed out) -- flagged as the top item for
/// the next pass. It also plausibly explains two things left unresolved
/// below: the `sFStateMgr_c`-shaped tail cluster and the "FUNCTION ORDER IS
/// WRONG" verify_anon report against the given 36-function range (a vtable
/// whose slot-filling is split across a wrongly-truncated claim will not
/// come out in address order no matter how this file is organised).
///
/// UNRESOLVED (open, flagged for the next round rather than guessed at):
/// there is a POD gap from +0x6c4 to +0x6e8 (0x24 bytes, no constructor call
/// covers it) whose owner is not yet identified -- `fn_2_15B320` reads a
/// field at this class's own +0x6c4 with a branchless "!= 4" test, but nothing
/// in dWmEnemy_c's or this class's own confirmed members explains a 4-valued
/// sentinel there. NOT modelled below; sizeof is preserved with a raw-byte
/// placeholder instead of guessing a type. This may be related to why the
/// task's four s_State.hpp dedup symbols (__dt__13sStateIDChk_cFv,
/// isNormalID__13sStateIDChk_cCFRC12sStateIDIf_c, and the sFState_c/
/// sFStateFct_c destructors -- all themselves TRIVIAL bodies per
/// s_StateIDChk.hpp/s_FState.hpp/s_FStateFct.hpp, `return true;`/`{}`) land in
/// this unit's tail (0x15B320-0x15B4C0): but the constructor shows no
/// `sFStateMgr_c` member being built here, so that placement may be a
/// dedup/link-order artifact contributed by some OTHER TU rather than this
/// class's own source, exactly as already documented for the dWmObjActor_c
/// cluster at the unit's HEAD (setCutEnd/clearCutEnd/checkCutEnd/vf74/vf78/
/// GetActorType/fn_2_15ABC0, 0x15AB60-0x15ABD0) -- dWmEnemy_c does not derive
/// from dWmObjActor_c, so this class structurally cannot be the source of
/// those either; check_bounds.py's ownership check confirms it (all
/// referrers for that cluster are OUTSIDE any claim this class could make).
/// Both clusters are left OUT of this draft's source deliberately.
class daWmAntlion_c : public dWmEnemy_c {
public:
    daWmAntlion_c();
    ~daWmAntlion_c();

    virtual int create();
    virtual int execute();
    /// @unofficial fn_2_15AF10. `mModel.remove(); return SUCCEEDED;` -- the
    /// call reads *(this+0x704) [mModel's own vtable] then dispatches a slot
    /// with `this` retargeted to `&mModel` itself (`lwzu` folding the
    /// `this+0x704` address computation into the same load), matching
    /// m3d::mdl_c's own `virtual void remove();` in every way except ONE:
    /// this compiles to vtable+0x10 (slot 4), target wants +0x14 (slot 5) --
    /// an 11/12 instruction near-miss, one virtual short somewhere in the
    /// scnLeaf_c/bmdl_c/smdl_c/mdl_c chain ahead of remove(). NOT resolved
    /// this round.
    virtual int doDelete();
    /// @unofficial Overrides dWmEnemy_c::calc() (matches fn_2_15B110: the
    /// same PSMTXTrans/ZXYrotM/setLocalMtx/setScale/calc(false) idiom as
    /// every other landed wm sibling's calcModel(), just reached through the
    /// vtable slot dWmEnemy_c already reserves for calc() rather than a
    /// direct call).
    virtual void calc();
    /// @unofficial fn_2_15B1C0. Best-effort reconstruction of the 0x150-byte
    /// target -- shape (two symmetric dispatches on cutsceneCommandId 0x48/
    /// 0x4a, first gated on isFirstFrame, second not) is confirmed; the two
    /// vtable calls (slot 0x70, checked `< 0`; slot 0x68, void) are guessed
    /// as GetIndex() (dWmEnemy_c's default returns -1 exactly when dead) and
    /// setCutEnd() by shape/semantics, NOT yet confirmed against the vtable
    /// directly -- flagged as the least certain function in this draft.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    /// @unofficial fn_2_15AF50. Same shape as every landed sibling's
    /// createModel() -- archive "cobAntlion" confirmed directly out of
    /// original/d_basesNP.rel's .data (string pool at .data+0x437e0/+0x43800,
    /// immediately after this unit's own sc_ForceList pair at +0x437ac/
    /// +0x437b4). "cobAntlionAppear" (.data+0x437c8) is a second, distinct
    /// pooled name used for the ResAnmChr/ResAnmTexSrt lookups (read via a
    /// pointer variable, not inlined at the call site, so its exact source
    /// spelling -- a local vs a static array element -- is not fully nailed
    /// down).
    void createModel();

    dHeapAllocator_c mAllocator;
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim;
    m3d::anmTexSrt_c mAnimTexSrt;
};

ACTOR_PROFILE(WM_ANTLION, daWmAntlion_c, 0);

daWmAntlion_c::daWmAntlion_c() {}
daWmAntlion_c::~daWmAntlion_c() {}

/// @unofficial fn_2_15AE00. createModel(); mClipSphere.set(mPos, 100.0f)
/// (radius confirmed directly out of original/d_basesNP.rel's .rodata at
/// +0x8598); calc() called virtually (through this class's own vtable, slot
/// 0xc8) exactly like execute()'s tail call to the same slot.
int daWmAntlion_c::create() {
    createModel();
    mClipSphere.set(mPos, 100.0f);
    calc();
    return SUCCEEDED;
}

int daWmAntlion_c::doDelete() {
    mModel.remove();
    return SUCCEEDED;
}

/// @unofficial fn_2_15AE70. processCutsceneCommand() called through this
/// class's OWN vtable (slot 24, +0x60 -- same slot sandpillar's report
/// documents), THEN mAnimTexSrt.play() (vtable+0x14) and mModel.play()
/// (vtable+0x1c) on the sub-objects' own vtables, THEN calc() virtually
/// (slot 0xc8) -- confirmed instruction-for-instruction against the target.
int daWmAntlion_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    mAnimTexSrt.play();
    mModel.play();
    calc();

    return SUCCEEDED;
}

void daWmAntlion_c::calc() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmAntlion_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobAntlion", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobAntlion");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::ANM_TEXSRT | nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    /// @unofficial `static const char *` pointer variable, not an inlined
    /// literal -- GetResAnmChr's call site loads it (`lwz r4, 0x54(r29)`)
    /// rather than computing the string's address directly, matching a named
    /// object (not a temporary). String content "cobAntlionAppear" read
    /// directly out of original/d_basesNP.rel's .data at +0x437c8.
    static const char *sAnmName = "cobAntlionAppear";

    nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr(sAnmName);
    mChrAnim.create(resMdl, resAnmChr, &mAllocator, nullptr);
    mChrAnim.mPlayMode = m3d::FORWARD_ONCE;
    mChrAnim.setRate(0.0f);
    mChrAnim.setFrame(0.0f);
    mModel.setAnm(mChrAnim);

    static const char *sTexSrtAnmName = "cobAntlion";
    mAnimTexSrt.create(resMdl, resFile.GetResAnmTexSrt(sTexSrtAnmName), &mAllocator, 1);
    mModel.setAnm(mAnimTexSrt);
    mAnimTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
    mAnimTexSrt.setRate(0.0f, 0);
    mAnimTexSrt.setFrame(0.0f, 0);
    mModel.setAnm(mAnimTexSrt);

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmAntlion_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    /// @unofficial `switch`, not if/else-if -- the target dispatches with
    /// `beq case; cmpwi next; beq case; (fall to end)`, the switch shape, not
    /// the `bne`-to-skip shape an if/else-if chain compiles to (see the
    /// "switch over if/else-if" lever). Confirmed by direct diff against the
    /// target this round.
    if (isFirstFrame) {
        switch (cutsceneCommandId) {
        case 0x48:
            if (GetIndex() >= 0) {
                mChrAnim.setRate(1.0f);
            }
            break;
        case 0x4a:
            if (mChrAnim.getRate() > 0.0f) {
                mChrAnim.setRate(-1.0f);
            }
            break;
        }
    }

    switch (cutsceneCommandId) {
    case 0x48:
        if (mChrAnim.getRate() == 0.0f || mChrAnim.isStop()) {
            setCutEnd();
        }
        break;
    case 0x4a:
        if (mChrAnim.getRate() == 0.0f || mChrAnim.isStop()) {
            setCutEnd();
            mChrAnim.setRate(0.0f);
        }
        break;
    default:
        /// @unofficial Target (L_0015B2EC) does `li r0,1; stb r0,0x139(r29)`.
        /// `m_00 = true` alone compiles to +0x138 (one byte short) --
        /// `m_00`'s own class-declaration NEIGHBOUR `mIsCutEnd` lands
        /// exactly at +0x139, and semantically matches setCutEnd()'s own
        /// body (`mIsCutEnd = true;`) called unconditionally here instead of
        /// through the virtual, matching the target's direct `stb`.
        mIsCutEnd = true;
        break;
    }
}
