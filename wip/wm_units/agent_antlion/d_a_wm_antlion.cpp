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

    /// @unofficial fn_2_15AE00. createModel(); mClipSphere.set(mPos, 100.0f)
    /// (radius confirmed directly out of original/d_basesNP.rel's .rodata at
    /// +0x8598); calc() called virtually (through this class's own vtable, slot
    /// 0xc8) exactly like execute()'s tail call to the same slot.
    virtual int create();
    /// @unofficial fn_2_15AE70. processCutsceneCommand() called through this
    /// class's OWN vtable (slot 24, +0x60), THEN mAnimTexSrt.play()
    /// (vtable+0x14) and mModel.play() (vtable+0x1c) on the sub-objects' own
    /// vtables, THEN calc() virtually (slot 0xc8) -- confirmed
    /// instruction-for-instruction against the target.
    virtual int execute();
    /// @unofficial fn_2_15AF10. `mModel.entry(); return SUCCEEDED;` -- the
    /// scnLeaf_c::entry() probe (three tiny compiled probes on
    /// `m->remove()`/`m->setAnm(*a)`/`m->play()` located those at slots
    /// 4/6/7; entry() keeps scnLeaf_c's own original slot 5, never
    /// re-declared by bmdl_c/mdl_c) is real, but was attached to the WRONG
    /// NAME for several rounds. The real vtable dump shows `preDelete`/
    /// `postDelete` (dWmActor_c) immediately follow slot 5, and `preDraw`/
    /// `postDraw` immediately follow slot 11 -- each lifecycle stage's pre/
    /// post hooks sit right after THAT stage's own action slot, so slot 5
    /// is doDelete's and slot 11 is draw's, the reverse of the earlier
    /// assumption. `mModel.entry()` is the standard `draw()` idiom already
    /// landed in every sibling (ghost, tower, ...) -- it belongs here, on
    /// draw(), not on doDelete().
    virtual int draw();
    /// @unofficial fn_2_15AF40. Confirmed via the same real-vtable dump:
    /// this slot is NOT overridden with anything beyond the trivial
    /// dWmDemoActor_c default (`return SUCCEEDED;`) -- declared explicitly
    /// out-of-line anyway (matching the "strong batch" lever below) so its
    /// DEFINITION can be placed at the correct target address.
    virtual int doDelete();
    /// @unofficial fn_2_15B1C0. `switch`-shaped dispatch on cutsceneCommandId
    /// 0x48/0x4a, the first gated on isFirstFrame -- confirmed exact against
    /// the target.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    /// @unofficial The following overrides (through updateBgmAnimRate below)
    /// were found via `check_vtable.py` against the REAL vtable
    /// (`lbl_2_data_43810`, dumped from
    /// `bin/dtkspl/d_basesNP/obj/auto_04_0003A960_data.o` -- almost every
    /// OTHER slot carries a real exported name already, e.g.
    /// `doWalk__10dWmEnemy_cFv`, `initDemoAnger__10dWmEnemy_cFv`, proving
    /// dWmEnemy_c's own non-inline virtuals are normally IMPORTED from the
    /// DOL when left un-overridden. These slots are `fn_2_XXXXXX` instead --
    /// REL-LOCAL, not imported -- so despite several of them having the
    /// exact same behaviour as dWmEnemy_c's own DOL-side body (read directly
    /// out of source/dol/bases/d_wm_enemy.cpp), the ORIGINAL source really
    /// does redeclare each one locally. DECLARED HERE in dWmEnemy_c's own
    /// header order (matching the vtable's flattened slot order), not
    /// grouped by discovery order, on the theory that MWCC's emission order
    /// for un-overridden inherited defaults tracks declaration/slot order --
    /// see the "FUNCTION ORDER" note in the task report for whether this
    /// actually closed the gap.
    /// @unofficial slot 28 (fn_2_15B310, `lwz r3,0x7ac(r3); blr`). +0x7ac is
    /// NOT inside mAnimTexSrt (that ends at +0x7a8, sizeof 0x2c, confirmed by
    /// probe) -- it is the second of two trailing int members after it.
    virtual int GetIndex();
    /// @unofficial slot 29 (fn_2_15B4D0), a `bctr` tail-call through THIS
    /// class's own vtable+0x70 -- exactly GetIndex()'s slot.
    virtual int GetNextIndex();
    /// @unofficial slot 38 (fn_2_15B490), `bctr` tail-call to vtable+0x8c
    /// (initDemoLose()'s own slot).
    virtual void initDemoStarLose();
    /// @unofficial slot 39 (fn_2_15B480), `bctr` tail-call to vtable+0x90
    /// (procDemoLose()'s own slot).
    virtual bool procDemoStarLose();
    /// @unofficial slot 40 (fn_2_15B470), bare `blr` -- empty body, REL-local
    /// rather than importing dWmEnemy_c's own (also empty, per
    /// d_wm_enemy.cpp -- but that copy is never referenced here).
    virtual void initDemoBgmDance();
    /// @unofficial slot 41 (fn_2_15B460), `li r3,1; blr`.
    virtual bool procDemoBgmDance();
    /// @unofficial slot 43 (fn_2_15B410). Bytes are `mVec3_c::Zero` copied
    /// into the return slot -- IDENTICAL to dWmEnemy_c::getPointOffset()'s
    /// own DOL body (`return mVec3_c::Zero;`, d_wm_enemy.cpp), just declared
    /// locally instead of imported.
    virtual mVec3_c getPointOffset(int index);
    /// @unofficial slot 48 (fn_2_15B3D0), `lwz r0,4(r3); extrwi r3,r0,4,24;
    /// blr` -- a 4-bit field at bit offset 4, IDENTICAL to
    /// dWmEnemy_c::getStartPoint()'s own DOL body (`return
    /// ACTOR_PARAM(startPoint);`, d_wm_enemy.cpp). dWmEnemy_c's own
    /// `ACTOR_PARAM_CONFIG(startPoint, 4, 4)` is private to dWmEnemy_c, so
    /// this class needs its own copy of the same (offset, width) pair to
    /// spell the identical extraction.
    virtual int getStartPoint();
    /// @unofficial Overrides dWmEnemy_c::calc() (matches fn_2_15B110: the
    /// same PSMTXTrans/ZXYrotM/setLocalMtx/setScale/calc(false) idiom as
    /// every other landed wm sibling's calcModel(), just reached through the
    /// vtable slot dWmEnemy_c already reserves for calc() rather than a
    /// direct call).
    virtual void calc();
    /// @unofficial slot 51 (fn_2_15B3B0), bare `blr` -- empty, matching
    /// dWmEnemy_c::calculateEffect()'s own DOL body (`{}`) but REL-local.
    virtual void calculateEffect();
    /// @unofficial slot 61 (fn_2_15B320), the SAME "!= 4" branchless
    /// comparison as dWmEnemy_c's own inline default -- turns out 0x6c4 is
    /// simply `mCurrProc`'s fixed offset within the dWmEnemy_c layout
    /// (PROC_TYPE_WAIT_WALK == 4), NOT an antlion-specific field as first
    /// suspected. `mCurrProc`/`PROC_TYPE_WAIT_WALK` are private to
    /// dWmEnemy_c so this can't be spelled directly from a derived class;
    /// explicit-scope-calling the base's own (inline) method sidesteps
    /// that while still producing an explicit, STRONG-linkage override
    /// (needed to control DEFINITION-order placement -- root cause of the
    /// order defect: this one weak/deferred function, sitting between
    /// GetIndex and everything after it in the target's real address
    /// order, was the single point pinning the whole rest of the sequence
    /// to "too late").
    virtual int isWaitWalkEnd();
    /// @unofficial slot 60 (fn_2_15B340), bare `blr` -- empty, REL-local.
    virtual void updateBgmAnimRate();
    /// @unofficial slot 59 (fn_2_15B350), bare `blr` -- matches
    /// dWmEnemy_c::PostWaitWalk()'s own inline default (`{}`) but REL-local
    /// (explicit override needed to control DEFINITION-order placement --
    /// see the "strong vs weak batch" note on draw() above).
    virtual void PostWaitWalk();
    /// @unofficial slot 46 (fn_2_15B3E0), bare `blr` -- matches
    /// dWmEnemy_c::setWalkAnm(float)'s own inline default (`{}`), same
    /// reasoning as PostWaitWalk() above.
    virtual void setWalkAnm(float anmRate);
    /// @unofficial The following are ALSO explicit re-declarations of
    /// otherwise-untouched dWmEnemy_c inline defaults, same "strong batch"
    /// reasoning as draw()/PostWaitWalk()/setWalkAnm() above -- needed so
    /// their DEFINITIONS can be placed in target address order instead of
    /// being deferred to the trailing weak-function block.
    virtual short GetChangeDirRate();
    virtual int GetMoveRate();
    virtual short GetWalkWaitFrame();
    virtual bool IsRandomMove();
    virtual bool IsPlayerComing();
    virtual int vfc4(int pointNo);
    virtual float getWalkAnmRate();
    virtual short getWaitAngle();
    virtual mVec3_c calcBlowOffPos(float offsetX);
    virtual float GetTerritory();
    virtual int GetActorType();
    virtual void initWalk();

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
    /// @unofficial +0x7a8. sizeof(m3d::anmTexSrt_c) is 0x2c (probed directly:
    /// `(int)sizeof(m3d::anmTexSrt_c)` compiles to `li r3,0x2c`), so
    /// mAnimTexSrt ends at +0x7a8, NOT at sizeof(daWmAntlion_c) (+0x7b0) as
    /// first assumed -- there are 8 more bytes after it. GetIndex() reads
    /// the SECOND word of this pair (+0x7ac); the first is unaccounted for.
    int mUnk7A8;
    int mIndexCache; ///< @unofficial +0x7ac, read directly by GetIndex().

    /// @unofficial Private to dWmEnemy_c there; redeclared here so
    /// getStartPoint() can spell the identical bit-extraction locally.
    ACTOR_PARAM_CONFIG(startPoint, 4, 4);
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


int daWmAntlion_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    mAnimTexSrt.play();
    mModel.play();
    calc();

    return SUCCEEDED;
}

/// @unofficial RENAMED this round: the `preDelete`/`postDelete` hooks
/// (slot 6/7, dWmActor_c) immediately follow slot 5, and `preDraw`/
/// `postDraw` (slot 12/13) immediately follow slot 11 -- each lifecycle
/// stage's own pre/post hooks sit right after that stage's OWN action slot
/// in the real vtable dump. That means slot 5 (fn_2_15AF40) is doDelete's
/// slot and slot 11 (fn_2_15AF10) is draw's -- the OPPOSITE of what this
/// draft assumed for several rounds. `mModel.entry(); return SUCCEEDED;`
/// is the standard `draw()` idiom landed in every sibling (ghost, tower,
/// ...); it belongs on draw(), not doDelete(), and now is.
int daWmAntlion_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmAntlion_c::doDelete() {
    return SUCCEEDED;
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
    nw4r::g3d::ResAnmTexSrt resAnmTexSrt = resFile.GetResAnmTexSrt(sTexSrtAnmName);
    mAnimTexSrt.create(resMdl, resAnmTexSrt, &mAllocator, nullptr, 1);
    mModel.setAnm(mAnimTexSrt);
    mAnimTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
    mAnimTexSrt.setRate(0.0f, 0);
    mAnimTexSrt.setFrame(0.0f, 0);
    mModel.setAnm(mAnimTexSrt);

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
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

int daWmAntlion_c::GetIndex() {
    return mIndexCache;
}

int daWmAntlion_c::isWaitWalkEnd() {
    return dWmEnemy_c::isWaitWalkEnd();
}

void daWmAntlion_c::updateBgmAnimRate() {}

void daWmAntlion_c::PostWaitWalk() {}

short daWmAntlion_c::GetWalkWaitFrame() {
    return 20;
}

int daWmAntlion_c::GetMoveRate() {
    return 100;
}

short daWmAntlion_c::GetChangeDirRate() {
    return 50;
}

bool daWmAntlion_c::IsRandomMove() {
    return true;
}

bool daWmAntlion_c::IsPlayerComing() {
    return IsPlayerComingCore();
}

void daWmAntlion_c::calculateEffect() {}

int daWmAntlion_c::vfc4(int pointNo) {
    return pointNo;
}

int daWmAntlion_c::getStartPoint() {
    return ACTOR_PARAM(startPoint);
}

void daWmAntlion_c::setWalkAnm(float anmRate) {}

float daWmAntlion_c::getWalkAnmRate() {
    return 1.0f;
}

short daWmAntlion_c::getWaitAngle() {
    return 0;
}

mVec3_c daWmAntlion_c::getPointOffset(int index) {
    return mVec3_c::Zero;
}

mVec3_c daWmAntlion_c::calcBlowOffPos(float offsetX) {
    return mVec3_c(mPos.x + offsetX, mPos.y, mPos.z);
}

bool daWmAntlion_c::procDemoBgmDance() {
    return true;
}

void daWmAntlion_c::initDemoBgmDance() {}

bool daWmAntlion_c::procDemoStarLose() {
    return procDemoLose();
}

void daWmAntlion_c::initDemoStarLose() {
    initDemoLose();
}

void daWmAntlion_c::initWalk() {}

float daWmAntlion_c::GetTerritory() {
    return 0.0f;
}

int daWmAntlion_c::GetActorType() {
    return ACTOR_MAP_ENEMY;
}

int daWmAntlion_c::GetNextIndex() {
    return GetIndex();
}
