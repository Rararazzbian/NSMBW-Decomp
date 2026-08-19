#include <game/bases/d_a_wm_killer.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_effect_manager.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_w_camera.hpp>
#include <game/framework/f_manager.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_base.hpp>
#include <string.h>
#include <cmath>

// #unk_1684A0's two far-actor state getters on daWmPlayer_c. Both live well outside this
// unit's own claimed .text range (0x167940-0x1686e0), in the same REL module but an
// un-landed TU -- the R_2_1_ same-module cross-TU convention (see agent_sandpillar's own
// R_2_1_171400 precedent), not the DOL-absolute extern "C" form.
extern "C" int R_2_1_1994D0(daWmPlayer_c *player);
extern "C" int R_2_1_1994B0(daWmPlayer_c *player);


// This unit's shared float-constant pool (lbl_2_rodata_89B8..89DC in the target). MWCC folds
// an offset into an `lfs` immediate only when the address is an object's own first element,
// never for an indexed read into a shared array (the rule WM_ITEM found the hard way, as a
// wall in the other direction -- there the draft had an extra `addi` the target lacked; here
// the target HAS the `addi`, which is exactly the signature of an indexed read). Declaring one
// named array and indexing it reproduces that `addi` instead of each literal claiming its own
// dedicated slot. Values/order read directly off the real .rodata bytes at file offset
// 0x1c6600+addr, index i <-> address 0x89b8+4*i.
static const float sConsts[] = { 1.0f, 300.0f };

// #unk_1681C0's node-name template. The target .data emits this string's own pointer
// (lbl_2_data_4526C, at data+0x3c) BEFORE ACTOR_PROFILE's own g_profile object -- confirmed
// from the target's own byte layout (data+0x34 "Fk00", data+0x3c -> pointer to it, data+0x40
// -> g_profile, data+0x44 the two u16 profile ids). #unk_1681C0 memcpy's these 4 bytes into a
// local buffer, nulls a 5th byte, then adds `(char)mParam` to the last character to select one
// of several per-instance world-map nodes ("Fk00", "Fk01", ...).
static const char *smc_nodeNameTemplate = "Fk00";

// #unk_1680F0's own node name -- a plain, fixed "Killer" (distinct from #unk_1681C0's
// per-instance "Fk0N" template). Also used as the mesh-node name for a SECOND, undeclared
// dWmLib overload: getModelNodePos__6dWmLibFPCQ23m3d6bmdl_cPCc, lowercase, taking a node NAME
// string rather than the header's own uppercase GetModelNodePos(model, int nodeId) overload.
// Forward-declared here (shadow-only, not touching the shared header) so the compiler emits
// the exact matching mangled symbol via ordinary C++ overload resolution.
namespace dWmLib {
    nw4r::math::VEC3 getModelNodePos(const m3d::bmdl_c *model, const char *nodeName);
}

ACTOR_PROFILE(WM_KILLER, daWmKiller_c, 0);

// #createModel's two anim resource names. Each is followed in the target .data by its OWN
// named pointer (a genuine `lwz` load, not an embedded `addi` literal) -- confirmed via
// relocation dump: data+0x4528c -> data+0x4527c ("cobKillerShot"), data+0x452a0 ->
// data+0x45290 ("cobRotaryShot"). The other three strings used there (the shared
// "g3d/model.brres" path and the two arcNames "cobKiller"/"cobRotary") are bare embedded
// literals with no pointer, so they stay as plain string literal arguments. Declared in this
// exact order (matching the target's own .data layout, which is otherwise entirely accounted
// for by <game/bases/d_wm_lib.hpp>'s sc_ForceList block and ACTOR_PROFILE's own g_profile
// object -- no other unidentified structure).
static const char *smc_killerShot = "cobKillerShot";
static const char *smc_rotaryShot = "cobRotaryShot";

// #unk_1682F0's smoke-effect call. dWmEffectManager_c::m_pInstance-> at a raw DOL address
// (unnamed target, NOT the header's own playEffect() overload -- argument count/types differ:
// this call takes (effectId, m3d::smdl_c*, const char*, int, int), not playEffect's
// (int, mVec3_c*, mAng3_c*, mVec3_c*)), so it must be a third, undocumented method on the
// same instance. Declared as a raw free function taking the instance explicitly, per the
// project's established convention for unowned/undeclared cross-module calls.
extern "C" int fn_80103420(dWmEffectManager_c *self, int effectId, m3d::smdl_c *model, const char *name, int, int);

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
    m_204 = 1;
    return SUCCEEDED;
}

int daWmKiller_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// unk_167C70(). Confirmed NOT a vtable slot (check_vtable.py) -- an ordinary member function,
// called from #create AND (per its own body) a second time via #unk_167FB0's own call site --
// no, rather #unk_167C70 itself calls #unk_167FB0 first, then two static getters whose results
// are discarded, then zeroes #m_20c/#m_210, then on Kind==1 turns the killer to face
// mVec3_c(#mTargetPos) - mPos via the landed dWmDemoActor_c::setDirection. The two-temp shape
// (mTargetPos materialised into its own mVec3_c stack temp, THEN the subtraction into a second
// temp) is the project's own "two temps -> binary operator" rule.
void daWmKiller_c::unk_167C70() {
    unk_167FB0();
    mVec3_c tmp0 = unk_1680F0(this);
    mVec3_c tmp1 = unk_1681C0(this);
    m_20c = 0;
    m_210 = false;
    if ((int) ACTOR_PARAM(Kind) == 1) {
        mVec3_c targetPos(mTargetPos[0], mTargetPos[1], mTargetPos[2]);
        mVec3_c dir = targetPos - mPos;
        setDirection(dir);
    }
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
        nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr(smc_killerShot);
        mAnmChr.create(resMdl, resAnmChr, &mAllocator, nullptr);
        mAnmChr.setRate(0.0f);
        mAnmChr.setFrame(0.0f);
        mModel.setAnm(mAnmChr);
    } else if (ACTOR_PARAM(Kind) == 1) {
        nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobRotary", "g3d/model.brres");
        nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobRotary");
        mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
        nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr(smc_rotaryShot);
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

// unk_167FB0(). Called from #create's tail AND from #unk_167C70's own start (two call sites).
// Rebuilds #mMatrix from #mPos/#mAngle (both inherited dBaseActor_c members, no offset casts
// needed -- mMatrix@0x7c, mPos@0xac and mAngle@0x100 all confirmed via the landed
// d_base_actor.o corpus), then pushes it (and #mScale) down into #mModel.
void daWmKiller_c::unk_167FB0() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(*(const nw4r::math::VEC3 *) &mScale);
    mModel.calc(false);
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

// unk_1680F0(daWmKiller_c *self). Sibling to #unk_1681C0, same hidden-return shape, but looks
// up a fixed "Killer" mesh node (via m3d::getNodeID against #mModel's own ResMdl) instead of a
// per-instance map node. On success, uses dWmLib::getModelNodePos (the undeclared string-taking
// overload, see its own forward declaration above); on failure (nodeID < 0), falls back to
// mVec3_c(mPos.x, mPos.y + 50.0f, mPos.z) (real constant read from .rodata). Writes the result
// into #mMotion (NOT #mTargetPos -- the spawn position #unk_167F20 uses) and returns it.
mVec3_c daWmKiller_c::unk_1680F0(daWmKiller_c *self) {
    nw4r::g3d::ResMdl resMdl = self->mModel.getResMdl();
    int nodeID = m3d::getNodeID(resMdl, "Killer");

    // Unconditional fallback, only overwritten below on success -- matches the target's own
    // shape exactly (it computes this BEFORE branching on nodeID, not in an else-arm).
    mVec3_c pos(self->mPos.x, self->mPos.y + 50.0f, self->mPos.z);
    if (nodeID >= 0) {
        nw4r::math::VEC3 nodePos = dWmLib::getModelNodePos(&self->mModel, "Killer");
        pos.x = nodePos.x;
        pos.y = nodePos.y;
        pos.z = nodePos.z;
    }

    self->mMotion[0] = pos.x;
    self->mMotion[1] = pos.y;
    self->mMotion[2] = pos.z;
    return pos;
}

// unk_1681C0(daWmKiller_c *self). Confirmed content: builds a per-instance world-map node name
// ("Fk00", "Fk01", ... -- see #smc_nodeNameTemplate's own note) by adding `(char)self->mParam`
// to the template's last character, looks it up via daWmMap_c::GetNodePos(), writes the result
// into self->mTargetPos AND returns it (the hidden-return-pointer buffer IS the GetNodePos
// out-param, an RVO-style match, not a separate copy).
mVec3_c daWmKiller_c::unk_1681C0(daWmKiller_c *self) {
    char name[5];
    memcpy(name, smc_nodeNameTemplate, 4);
    u8 lowByte = (u8) self->mParam;
    name[4] = 0;
    name[3] += (s8) lowByte;

    mVec3_c pos;
    daWmMap_c::m_instance->GetNodePos(name, pos);

    self->mTargetPos[0] = pos.x;
    self->mTargetPos[1] = pos.y;
    self->mTargetPos[2] = pos.z;
    return pos;
}

// unk_168260(int index). Confirmed content: index<0 short-circuits to true; otherwise reaches
// into mChildren[index] (the unowned WM_KILLERBULLET sibling class, see #unk_1682B0's note) at
// raw offset 0x1b0 (a 4-byte state-ish field, checked == 2) and 0x1f9 (a byte immediately after
// this unit's own #mTargetPos range, checked != 0) -- both conditions must hold for a true result.
bool daWmKiller_c::unk_168260(int index) {
    if (index < 0) {
        return true;
    }
    bool result = false;
    dWmActor_c *child = mChildren[index];
    if (*(int *) ((u8 *) child + 0x1b0) == 2 && *(u8 *) ((u8 *) child + 0x1f9) != 0) {
        result = true;
    }
    return result;
}

// unk_1682B0(int index). Confirmed content: guards index<0, then reaches into
// mChildren[index] (a WM_KILLERBULLET child, an unowned/undeclared sibling class) at raw
// offset 0x1f8+0xc=0x204 and writes a literal 1 -- the same raw-offset-cast technique already
// established for reaching into an unowned class's field (see WM_START's `this+0x139` and
// `daWmPlayer_c+0x29c` precedent). Coincidentally the SAME offset this unit's own #m_204
// occupies, suggesting the sibling class shares this unit's layout up to that point, but its
// real identity/layout is not owned here.
void daWmKiller_c::unk_1682B0(int index) {
    if (index < 0) {
        return;
    }
    *(u8 *) ((u8 *) mChildren[index] + 0x204) = 1;
}

// unk_1682D0(int index, u8 value). Same shape as #unk_1682B0 but writes a caller-supplied byte
// to offset 0x1f8 of mChildren[index] instead of a literal to 0x204.
void daWmKiller_c::unk_1682D0(int index, u8 value) {
    if (index < 0) {
        return;
    }
    *(u8 *) ((u8 *) mChildren[index] + 0x1f8) = value;
}

// unk_1682F0(). Confirmed content: sets #mAnmChr's rate/frame (real constants read from
// .rodata: 1.0f/0.0f), fires a "smoke" effect via the raw fn_80103420 call (see its own note
// above) attached to #mModel, then plays sound 0x63 at #mPos via dWmSeManager_c::m_pInstance.
void daWmKiller_c::unk_1682F0() {
    mAnmChr.setRate(1.0f);
    mAnmChr.setFrame(0.0f);
    fn_80103420(dWmEffectManager_c::m_pInstance, 0x13, &mModel, "smoke", 0, 0);
    dWmSeManager_c::m_pInstance->playSound(0x63, mPos, 1);
}

// unk_168380(). Iterates every daWmKiller_c sibling (dBase_c::searchBaseByProfName over the
// shared WM_KILLER profile, note the DIFFERENT mangled class from #unk_168590's
// fManager_c::searchBaseByProfName -- confirmed from the target's own mangled symbol, not
// guessed) and tracks which one's #mTargetPos is within 300.0f (real constant) of the player's
// position into #m_204, via ACTOR_PARAM(SpawnKind). No-op (early return) if no sibling exists.
void daWmKiller_c::unk_168380() {
    daWmKiller_c *sibling = (daWmKiller_c *) dBase_c::searchBaseByProfName(fProfile::WM_KILLER, nullptr);
    mVec3_c playerPos = daWmPlayer_c::ms_instance->mPos;

    if (sibling == nullptr) {
        return;
    }

    float threshold = sConsts[1];
    while (sibling != nullptr) {
        mVec3_c targetPos(sibling->mTargetPos[0], sibling->mTargetPos[1], sibling->mTargetPos[2]);
        bool isClose = std::fabs(playerPos.distTo(targetPos)) < threshold;
        if (isClose) {
            m_204 = ACTOR_PARAM_LOCAL(sibling->mParam, SpawnKind);
        } else if (m_204 == (int) ACTOR_PARAM_LOCAL(sibling->mParam, SpawnKind)) {
            m_204 = -1;
        }
        sibling = (daWmKiller_c *) dBase_c::searchBaseByProfName(fProfile::WM_KILLER, sibling);
    }
}

// unk_168590(). Static (no `this` -- confirmed from its only call site). Iterates every
// daWmKiller_c sibling actor (searchBaseByProfName over the shared WM_KILLER profile) and
// checks whether any with ACTOR_PARAM(SpawnKind) >= 1 has its own #mClipSphere inside the
// camera's view clip.
bool daWmKiller_c::unk_168590() {
    daWmKiller_c *killer = (daWmKiller_c *) fManager_c::searchBaseByProfName(fProfile::WM_KILLER, nullptr);
    bool result = false;
    while (killer != nullptr) {
        if ((int) ACTOR_PARAM_LOCAL(killer->mParam, SpawnKind) >= 1) {
            if (dWCamera_c::m_instance->mViewClip.CheckClipSphere(&killer->mClipSphere)) {
                result = true;
                break;
            }
        }
        killer = (daWmKiller_c *) fManager_c::searchBaseByProfName(fProfile::WM_KILLER, killer);
    }
    return result;
}

// unk_1684A0(bool isFirstFrame). Two genuine early returns (bypassing the flag/#unk_168590
// computation entirely): a cutscene-command-0x38 special case returning #m_208 directly, and
// an ACTOR_PARAM(SpawnKind)==0 case returning the camera's own CheckClipSphere directly. Only
// the remaining (SpawnKind != 0) path reaches the daWmPlayer_c state checks and #unk_168590.
bool daWmKiller_c::unk_1684A0(bool isFirstFrame) {
    if (!isFirstFrame && dCsSeqMng_c::ms_instance->GetCutName() == dCsSeqMng_c::CUTSCENE_CMD_56) {
        return m_208;
    }

    if (ACTOR_PARAM(SpawnKind) == 0) {
        return dWCamera_c::m_instance->mViewClip.CheckClipSphere(&mClipSphere);
    }

    bool flag = false;
    if (R_2_1_1994D0(daWmPlayer_c::ms_instance) == 0x17 || R_2_1_1994D0(daWmPlayer_c::ms_instance) == 0x25 ||
        R_2_1_1994B0(daWmPlayer_c::ms_instance) == 0x25 || R_2_1_1994B0(daWmPlayer_c::ms_instance) == 0x17) {
        flag = true;
    }

    bool viewResult = unk_168590();
    return !flag && viewResult;
}
