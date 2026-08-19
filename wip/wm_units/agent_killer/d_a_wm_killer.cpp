#include <game/bases/d_a_wm_killer.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_res_mng.hpp>

ACTOR_PROFILE(WM_KILLER, daWmKiller_c, 0);

daWmKiller_c::daWmKiller_c() : m_208(false) {}
daWmKiller_c::~daWmKiller_c() {}

// create(). Confirmed call sequence and the mClipSphere.set(mPos, 100.0f) quad (matching
// castle/WM_START's own create() shape) -- the five sub-calls' own bodies are not yet authored.
int daWmKiller_c::create() {
    createModel();
    unk_167F20();
    mClipSphere.set(mPos, 100.0f);
    unk_167C70();
    unk_167FB0();
    return SUCCEEDED;
}

// PARTIALLY AUTHORED (0x124 bytes, vtable slot 8). The opening call is confirmed byte-exact:
// an ordinary virtual call to #processCutsceneCommand, using the same
// dCsSeqMng_c::ms_instance->GetCutName()/m_164 idiom already landed in daWmGhost_c::execute()
// (source/d_basesNP/bases/d_a_wm_ghost.cpp:47-48) -- the double `0x60(r30)` vtable
// dereference this compiles to is dBase_c's ordinary secondary vtable (dBase_c : public
// fBase_c, public cOwnerSetMg_c), not a raw offset cast; see 002a0d7's antlion_mng finding.
// The remainder (an unidentified far call `fn_800FC6D0(buf, f, f)` filling a 6-float
// structure that gets stored into a far .bss symbol, a conditional call into another far
// function `fn_2_169550` gated on m_210/#unk_1684A0, then calls into #unk_168380 and
// #unk_167FB0) is NOT yet authored -- content unconfirmed, not guessed.
int daWmKiller_c::execute() {
    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);
    mPad_1f8[0] = 0x1;
    return SUCCEEDED;
}

int daWmKiller_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// NOT YET AUTHORED (0xa8 bytes). Confirmed NOT a vtable slot (check_vtable.py) -- an ordinary
// member function, called once from #create.
void daWmKiller_c::unk_167C70() {
    mPad_1f8[0] = 0x2;
}

int daWmKiller_c::doDelete() {
    return SUCCEEDED;
}

// createModel(). Confirmed by content, not position: shares the exact createFrmHeap /
// getRes / GetResMdl / smdl_c::create / GetResAnmChr / anmChr_c::create / setRate / setFrame /
// setSoftLight_MapObj / adjustFrmHeap shape already landed in daWmGhost_c::createModel() and
// daWmSandPillar_c::createModel(). Two variants selected by ACTOR_PARAM(Kind) (offset 16,
// width 16 -- a plain `mParam >> 16` with no mask, since it consumes the whole upper half),
// confirmed by reading the real strings out of this unit's own .data object (lbl_2_data_45230,
// this unit's own .data opener) at file offset 0x1d0c00+addr: "cobKiller"/"cobKillerShot" for
// Kind==0, "cobRotary"/"cobRotaryShot" for Kind==1, sharing the path "g3d/model.brres". The
// rate/frame constant (lbl_2_rodata_89D4) read as 0.0f from .rodata at file offset
// 0x1c6600+addr, matching the ghost/sandpillar convention exactly.
void daWmKiller_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    if (ACTOR_PARAM(Kind) == 0) {
        nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobKiller", "g3d/model.brres");
        nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobKiller");
        mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
        nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr("cobKillerShot");
        mAnmChr.create(resMdl, resAnmChr, &mAllocator, nullptr);
        mAnmChr.setRate(0.0f);
        mAnmChr.setFrame(0.0f);
        mModel.setAnm(mAnmChr);
    } else if (ACTOR_PARAM(Kind) == 1) {
        nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobRotary", "g3d/model.brres");
        nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobRotary");
        mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
        nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr("cobRotaryShot");
        mAnmChr.create(resMdl, resAnmChr, &mAllocator, nullptr);
        mAnmChr.setRate(0.0f);
        mAnmChr.setFrame(0.0f);
        mModel.setAnm(mAnmChr);
    }

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmKiller_c::unk_167F20() {
    for (int i = 0; i < 10; i++) {
        unsigned long param = i | (ACTOR_PARAM(SpawnKind) << 8);
        mVec3_c pos(mMotion[0], mMotion[1], mMotion[2]);
        mChildren[i] = dWmActor_c::construct(fProfile::WM_KILLERBULLET, this, param, &pos, nullptr);
    }
}

// NOT YET AUTHORED (0xb0 bytes). Called last from #create.
void daWmKiller_c::unk_167FB0() {
    mPad_1f8[0] = 0x5;
}

// processCutsceneCommand. Structurally confirmed: early-return on cutsceneCommandId==-1, then
// gated on !checkCutEnd() (a real dWmDemoActor_c virtual call, not guessed), then a
// isFirstFrame && cutsceneCommandId==0x38 branch calling #unk_1684A0, then an unconditional
// write to a base-class field at this+0x139 shared with WM_START's own identically-shaped
// residual (not owned by this class, raw offset write per that unit's precedent).
void daWmKiller_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }
    if (!checkCutEnd()) {
        if (isFirstFrame && cutsceneCommandId == 0x38) {
            m_208 = unk_1684A0(true);
        }
        *(bool *) ((u8 *) this + 0x139) = true;
    }
}

void daWmKiller_c::unk_1680F0() {
    mPad_1f8[0] = 0x6;
}

void daWmKiller_c::unk_1681C0() {
    mPad_1f8[0] = 0x7;
}

void daWmKiller_c::unk_168260() {
    mPad_1f8[0] = 0x8;
}

void daWmKiller_c::unk_1682B0() {
    mPad_1f8[0] = 0x9;
}

void daWmKiller_c::unk_1682D0() {
    mPad_1f8[0] = 0xa;
}

void daWmKiller_c::unk_1682F0() {
    mPad_1f8[0] = 0xb;
}

void daWmKiller_c::unk_168380() {
    mPad_1f8[0] = 0xc;
}

void daWmKiller_c::unk_168590() {
    mPad_1f8[0] = 0xd;
}

// NOT YET AUTHORED (0xe4 bytes). Called from #processCutsceneCommand with (this, true) --
// placeholder bool(bool) signature inferred only from the call site.
bool daWmKiller_c::unk_1684A0(bool b) {
    return b;
}
