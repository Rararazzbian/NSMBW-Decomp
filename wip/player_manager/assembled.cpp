#include <game/bases/d_a_player_manager.hpp>

#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_multi_manager.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_cd_data.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_game_display.hpp>
#include <game/bases/d_next.hpp>
#include <game/bases/d_quake.hpp>
#include <game/bases/d_stage_timer.hpp>
#include <game/bases/d_pause_manager.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_score_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/snd/snd_scene_manager.hpp>
#include <game/snd/snd_audio_mgr.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/mLib/m_vec.hpp>
#include <constants/sound_list.h>

// =========================================================================
// Out-of-line storage definitions for daPyMng_c's static members.
//
// NO AUTHORING BATCH OWNS THESE; they are the lead's to place (per
// wip/player_manager/statics_defs.inc), and their ORDER is the whole point:
// definition order in the .cpp controls emission order within each section,
// and the target anchors several adjacent .bss arrays off m_playerID's own
// relocation -- reaching mPlayerEntry as 0x40(r31) and so on, including
// arrays a given function never reads. MWCC only shares one base register
// across several objects once it can see all their definitions in a single
// TU, which is why standalone drafts of getNumInGame, getCoinAll,
// initYoshiPriority, setYoshiPriority, isCreateBalloon and
// checkCorrectCreateInfo all showed register-allocation differences in
// isolation that were NOT source defects.
//
// .bss and .sbss are separate sections, so each preserves the relative
// order of its own objects independently; writing the .bss group first and
// the .sbss group second satisfies both at once.
//
// Section placement is automatic, not chosen: MWCC routes objects of 8
// bytes or fewer to .sbss and larger ones to .bss. The int[4] arrays are
// 0x10 and land in .bss; the scalars, m_star_time/m_star_count (s16[4] = 8)
// and m_yoshiColor (u8[4] = 4) land in .sbss.
// =========================================================================

// --- .bss, in target address order 0x80355110 .. 0x803551D0 ----------------
int daPyMng_c::m_playerID[4];          // 0x80355110  <- the base anchor
int daPyMng_c::m_yoshiID[4];           // 0x80355120
int daPyMng_c::mCourseInList[4];       // 0x80355130
int daPyMng_c::m_yoshiFruit[4];        // 0x80355140
int daPyMng_c::mPlayerEntry[4];        // 0x80355150  = m_playerID + 0x40
PLAYER_TYPE_e daPyMng_c::mPlayerType[4];    // 0x80355160  = + 0x50
PLAYER_POWERUP_e daPyMng_c::mPlayerMode[4]; // 0x80355170  = + 0x60
u32 daPyMng_c::mCreateItem[4];         // 0x80355180  = + 0x70
int daPyMng_c::mRest[4];               // 0x80355190
int daPyMng_c::mCoin[4];               // 0x803551A0  indexed by PLAYER_TYPE_e
int daPyMng_c::m_quakeTimer[4];        // 0x803551B0  = + 0xA0
int daPyMng_c::m_quakeEffectFlag[4];   // 0x803551C0  = + 0xB0

// The four embedded managers. Each is preceded in .bss by a 0xC
// __register_global_object destructor-chain node that the compiler emits,
// and __sinit constructs them in exactly this order.
daPyDemoMng_c daPyMng_c::mDemoManager;  // 0x803551E0, 0x98
dMultiMng_c   daPyMng_c::mMultiManager; // 0x80355284, 0x5C
dAttention_c  daPyMng_c::mAttention;    // 0x803552F0, 0x58
dPyEffectMng_c daPyMng_c::mEffectMng;   // 0x80355354, 0xC5C  (ends 0x80355FB0)

// --- .sbss, in target address order 0x80429F80 .. 0x80429FD0 ---------------
int daPyMng_c::mNum;                 // 0x80429F80
u32 daPyMng_c::mCtrlPlrNo;           // 0x80429F84
u8  daPyMng_c::mActPlayerInfo;       // 0x80429F88  (1 byte, then a 3-byte pad)
u8  daPyMng_c::m_yoshiColor[4];      // 0x80429F8C  indexed raw, unscaled
s16 daPyMng_c::m_star_time[4];       // 0x80429F90
s16 daPyMng_c::m_star_count[4];      // 0x80429F98
int daPyMng_c::mScore;               // 0x80429FA0
PLAYER_POWERUP_e daPyMng_c::mKinopioMode; // 0x80429FA4
int daPyMng_c::mTimeUpPlayerNum;     // 0x80429FA8
int daPyMng_c::mAllBalloon;          // 0x80429FAC
int daPyMng_c::mPauseEnableInfo;     // 0x80429FB0
u32 daPyMng_c::mPauseDisable;        // 0x80429FB4
u32 daPyMng_c::mStopTimerInfo;       // 0x80429FB8
u32 daPyMng_c::mStopTimerInfoOld;    // 0x80429FBC
int daPyMng_c::mQuakeTrigger;        // 0x80429FC0
int daPyMng_c::mBgmState;            // 0x80429FC4
int daPyMng_c::mBonusNoCap;          // 0x80429FC8
int daPyMng_c::mKinopioCarryCount;   // 0x80429FCC

// 0x80429FD0 -- the last object in the unit's .sbss, unnamed, no class
// mangling, read/written only inside setHipAttackQuake (found by B7). Type
// is `s8`, not `bool`/`u8`: the target reads it back with `lbz` followed by
// a record-form `extsb.` (a signed-byte test). Must be defined immediately
// after the .sbss block above and before anything else that could claim
// .sbss space -- nothing else in this TU does.
static s8 lbl_80429FD0;

// =========================================================================
// Undeclared fields on frozen, shared, foreign classes -- accessed via
// raw-offset helpers per this project's established convention (see
// d_a_player_demo_manager.cpp's field_XXX_ref precedent), never as invented
// header members. Offsets proven directly from this unit's own
// disassembly; see BATCH1/2/4/6/7/8.md for the per-offset evidence.
//
// Two of these fields are read from two different functions authored by two
// different batches (B1's getPlayerCreateAction/initStage and B2's
// createCourseInit); both call sites are unified onto ONE helper below
// instead of landing two separately-named raw-cast definitions of the same
// field, which is what the batches would have produced independently.
// =========================================================================

// dScStage_c +0x120e / +0x1211 (u8 each): the "file"/"gotoNo" indices fed to
// dCd_c::getFileP()/getNextGotoP(). Read by getPlayerCreateAction (B1) AND
// createCourseInit (B2) -- one definition, two call sites.
static inline u8 &stageField_0x120e(dScStage_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x120e);
}
static inline u8 &stageField_0x1211(dScStage_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x1211);
}

// dInfo_c +0xaf4 (s32, signed, tested >= 0): read by initStage's course-in
// gate (B1) AND createCourseInit's mid-scroll override (B2) -- likewise one
// definition, two call sites. dInfo_c's own pad11[0x712] (object offset
// 0x3ec..0xafe) covers this whole range, so this is a real, currently-padded
// field, not a foreign object.
static inline int &infoField_0xaf4(dInfo_c *p) {
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0xaf4);
}
// dInfo_c +0x24 (u8, tested != 0): initStage only (B1).
static inline u8 &infoField_0x24(dInfo_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x24);
}
// dInfo_c +0x10..+0x1c: 3 consecutive floats (one mVec3_c-sized copy), plus
// a trailing +0x1c int. createCourseInit only (B2).
static inline mVec3_c &infoField_0x10(dInfo_c *p) {
    return *reinterpret_cast<mVec3_c *>(reinterpret_cast<u8 *>(p) + 0x10);
}
static inline int &infoField_0x1c(dInfo_c *p) {
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x1c);
}
// dInfo_c +0xafc (u8): isEffectStop only (B8). A DIFFERENT offset from
// +0xaf4 above -- not the same field, do not merge.
static inline u8 &infoField_0xafc(dInfo_c *p) {
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0xafc);
}

// lbl_802EF478 -- RESOLVED by the lead. .rodata:0x802EF478, 0x10, int[4]
// {0, 1, 3, 2}. Nothing in the binary reads its four words, and the agent
// that found it correctly refused to invent a use -- but it also stopped one
// step short. A plain file-scope `const` array IS folded away by -O4, because
// at namespace scope a const array has INTERNAL linkage in C++ and is stripped
// as unused. `extern` gives it external linkage, and then it survives.
//
// This is the exact lever that fixed d_a_en_hatena_balloon's l_speed_ratiodt,
// a 0x40 float table the whole binary never references and whose absence left
// .rodata 0x20 short and failed four of five binaries. AGENT_CONTEXT Sec.6
// states both halves; only the folding half was applied here.
//
// Its POSITION is load-bearing: exactly 0x10 before l_start_pos_ofs, so
// createCourseInit's +0x160/+0x180 offsets land correctly.
extern const int lbl_802EF478[4] = {0, 1, 3, 2};

// l_start_pos_ofs -- CORRECTION, see wip/player_manager/RODATA.md: STATICS.md
// and BOUNDS.md were both wrong to call this "not ours" / outside our
// .rodata bound. getPlayerSetPos__9daPyMng_cFUcUc -- unambiguously daPyMng_c,
// inside our own .text range -- loads it directly by name at 0x8005EDD8/DC
// (lis/addi @ha/@l, no other TU's code ever references the symbol). Left
// `extern` it would be an UNDEFINED SYMBOL at link time: no landed TU in
// source/ defines it. 28 entries, mVec3_c[28], indexed by nextGoto->mType
// (`mulli r5, r0, 0xc`, stride 0xc = sizeof(mVec3_c)). Values read byte-exact
// out of target_rodata.txt (.rodata:0x802EF488, size 0x150).
const Vec l_start_pos_ofs[28] = {
    {0.0f, -16.0f, 8.0f}, {0.0f, -16.0f, 8.0f}, {16.0f, -16.0f, 0.0f}, {16.0f, -32.0f, 0.0f},
    {16.0f, -16.0f, 0.0f}, {16.0f, -32.0f, 0.0f}, {0.0f, -32.0f, 0.0f}, {0.0f, 0.0f, 8.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}, {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f},
    {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 8.0f}, {8.0f, -32.0f, 8.0f}, {0.0f, -16.0f, 8.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, -16.0f, 8.0f},
};

// lbl_802EF478 -- UNRESOLVED, see wip/player_manager/RODATA.md. .rodata
// 0x802EF478, size 0x10, values {0, 1, 3, 2} (int[4]). Proven ours by a
// compile-time argument (createCourseInit computes `addi r30, r29, 0x160`
// off it to reach scOfsX, and a same-TU-only relative .rodata offset can only
// be compile-time-constant if both objects come from THIS compilation), and
// its position is load-bearing: it is exactly 0x10 bytes before
// l_start_pos_ofs, so createCourseInit's `+0x160`/`+0x180` offsets land on
// scOfsX/the order[] init pool only if this object exists at that exact
// spot. But exhaustive search of all 67 of our functions' `@ha` references
// (see RODATA.md) found NO direct read of its own 4 words anywhere -- B2
// already flagged the same "never read by anything in this batch" finding.
// A plain unused `static const` would be folded away by -O4 (AGENT_CONTEXT
// Sec.6), so writing one here would NOT reproduce it. Left absent rather
// than guessing a fake use; the gap is flagged so the section is not
// silently short by 0x10 once someone finds the real trigger.

// daPyDemoMng_c's own unnamed helper (0x8005D280 in that TU, no symbol) --
// a real, already-landed global function in d_a_player_demo_manager.cpp,
// not a class member and not declared in any header. Forward-declared here
// (B2).
void makeCourseInList(daPyDemoMng_c *pMgr);

// dAcPy_c +0x153c (s8, signed -- the target uses cmpwi, not cmplwi):
// getActScrollInfo / getScrollNum only (B4).
static inline s8 &scrollFlagRef(dAcPy_c *p) {
    return *reinterpret_cast<s8 *>(reinterpret_cast<u8 *>(p) + 0x153c);
}

// getYoshi's fBase_c-vtable-slot-0x6c dispatch (dActor_c::getPlrNo(), an
// INLINE virtual -- calling it by name would instantiate a stray local weak
// copy the target does not have; see BATCH4.md). Untyped fetch instead (B4).
typedef s8 &(*GetPlrNoFn)(dActor_c *);
static inline GetPlrNoFn getVfunc6c(fBase_c *base) {
    return (*(GetPlrNoFn **)((u8 *)base + 0x60))[0x6c / 4];
}

// fBase_c +0xb (u8, protected mDeleteRequested): deleteCullingYoshi only
// (B7).
static inline bool isDeleteRequested(fBase_c *p) {
    return *(reinterpret_cast<const u8 *>(p) + 0xb) != 0;
}

// daPlBase_c +0x1036 (u8, yoshi-priority rank byte): initYoshiPriority /
// setYoshiPriority only (B8).
// RESOLVED: 0x1036 is daPlBase_c::mPlayerLayer, an already-declared, matched
// member -- the banked dAcPy_c constructor stores mPlayerNo there. These two
// functions REUSE the draw layer as the Yoshi priority rank, which is coherent:
// a Yoshi's draw layer IS its priority. No cast and no new field needed.

// fn_80060DB0 -- file-scope static, called from setHipAttackQuake one basic
// block before its own epilogue; defined further down at its own address
// (B7). No cross-TU caller anywhere in source/, no syms.txt pin.
static void fn_80060DB0();

// =========================================================================
// Functions, in TARGET ADDRESS ORDER (0x8005E9A0 .. 0x80061304).
//
// getCourseIn__10dScStage_cFv (0x8005EC90) and getFileP__5dCd_cFi
// (0x8005EE70) are foreign weak inline bodies flushed automatically into
// this object file because we call them (initStage / getPlayerSetPos,
// getPlayerCreateAction, createCourseInit respectively) -- nobody authors
// them, and no source appears here for either.
// =========================================================================

// --- B1: 0x8005E9A0-0x8005EEDF ---------------------------------------------

daYoshi_c *daPyMng_c::createYoshi(mVec3_c &pos, int type, dAcPy_c *rider) {
    if (rider == nullptr) {
        u32 param = ACTOR_PARAM_GEN(dAcPy_c, PlayerNo, type) | ACTOR_PARAM_GEN(dAcPy_c, CreateAction, 1);
        return (daYoshi_c *) dActor_c::construct(fProfile::YOSHI, param, &pos, nullptr, 0);
    }
    u32 param = (rider->mParam & BIT_FLAG(dAcPy_c::PARAM_Direction >> 8)) | ACTOR_PARAM_GEN(dAcPy_c, PlayerNo, type);
    daYoshi_c *yoshi = (daYoshi_c *) dActor_c::construct(fProfile::YOSHI, param, &pos, nullptr, 0);
    if (yoshi != nullptr && yoshi->fn_8014eb70(rider, 1)) {
        yoshi->setCreateAction((rider->mParam >> (dAcPy_c::PARAM_CreateAction >> 8)) &
                               ((1 << (dAcPy_c::PARAM_CreateAction & 0xff)) - 1));
    }
    return yoshi;
}

void daPyMng_c::initGame() {
    mPlayerMode[0] = (PLAYER_POWERUP_e) 0;
    mCreateItem[0] = 0;
    mPlayerMode[1] = (PLAYER_POWERUP_e) 0;
    mCreateItem[1] = 0;
    mPlayerMode[3] = (PLAYER_POWERUP_e) 0;
    mCreateItem[3] = 0;
    mPlayerEntry[0] = 0;
    mPlayerType[0] = (PLAYER_TYPE_e) 0;
    mPlayerEntry[1] = 0;
    mPlayerType[1] = (PLAYER_TYPE_e) 1;
    mPlayerEntry[2] = 0;
    mPlayerType[2] = (PLAYER_TYPE_e) 3;
    mPlayerEntry[3] = 0;
    mPlayerType[3] = (PLAYER_TYPE_e) 2;
    mPlayerMode[2] = (PLAYER_POWERUP_e) 0;
    mCreateItem[2] = 0;
    mActPlayerInfo |= 1;
    setDefaultParam();
    mBonusNoCap = 0;
    mKinopioCarryCount = 0;
}

void daPyMng_c::initStage() {
    checkCorrectCreateInfo();
    mNum = 0;
    mCtrlPlrNo = 0;
    mActPlayerInfo = 0;
    for (int i = 0; i < 4; i++) {
        setPlayer(i, nullptr);
        m_star_time[i] = 0;
        m_star_count[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i] != 0) {
            addNum(i);
        }
    }
    if (dScStage_c::getCourseIn() &&
        infoField_0xaf4(dInfo_c::getInstance()) >= 0 &&
        isEntryNum1() &&
        infoField_0x24(dInfo_c::getInstance()) != 0) {
        for (int i = 0; i < 4; i++) {
            if (mPlayerEntry[i] == 0) {
                fn_8005f570((PLAYER_POWERUP_e) 0, i);
                break;
            }
        }
    }
    mTimeUpPlayerNum = 0;
    mAllBalloon = 0;
    mPauseEnableInfo = 0;
    mPauseDisable = 0;
    mStopTimerInfo = 0;
    mStopTimerInfoOld = 0;
    mQuakeTrigger = 0;
    for (int i = 0; i < 4; i++) {
        m_quakeTimer[i] = 0;
        m_quakeEffectFlag[i] = 0;
    }
    mBgmState = 0;
    checkBonusNoCap();
    mKinopioCarryCount = 0;
    daPyDemoMng_c::mspInstance->initStage();
    dMultiMng_c::mspInstance->initStage();
}

void daPyMng_c::exitStage() {
}

void daPyMng_c::courseIn() {
    createCourseInit();
    mPauseDisable = 0;
    daPyDemoMng_c::mspInstance->initCourseIn();
}

void daPyMng_c::setDefaultParam() {
    for (int i = 0; i < 4; i++) {
        mRest[mPlayerType[i]] = 5;
        mCoin[mPlayerType[i]] = 0;
    }
    for (int i = 0; i < 4; i++) {
        m_playerID[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        m_yoshiID[i] = 0;
    }
    mScore = 0;
}

nw4r::math::VEC3 daPyMng_c::getPlayerSetPos(u8 file, u8 gotoNo) {
    nw4r::math::VEC3 result;
    dCdFile_c *cdFile = dCd_c::getFileP(file);
    sNextGotoData *nextGoto = cdFile->getNextGotoP(gotoNo);
    float y = -(float) nextGoto->mY;
    float x = (float) nextGoto->mX;
    result.z = 0.0f;
    result.y = y;
    result.x = x;
    int idx = nextGoto->mType;
    result.x = x + l_start_pos_ofs[idx].x;
    result.y = y + l_start_pos_ofs[idx].y;
    if (!(nextGoto->mFlags & 0x40)) {
        result.x = result.x + l_start_pos_ofs[idx].z;
    }
    return result;
}

int daPyMng_c::getPlayerCreateAction() {
    dScStage_c *stage = dScStage_c::getInstance();
    dCdFile_c *cdFile = dCd_c::getFileP(stageField_0x120e(stage));
    sNextGotoData *nextGoto = cdFile->getNextGotoP(stageField_0x1211(stage));
    return nextGoto->mType;
}

// --- B2: 0x8005EEE0-0x8005F5BF ---------------------------------------------

bool daPyMng_c::create(int plrNo, mVec3_c *pos, int type, u8 flag) {
    if (mPlayerEntry[plrNo] != 0) {
        dActor_c::construct(fProfile::PLAYER, (plrNo & 0xf) | ((type & 0xff) << 16) | ((flag & 1) << 24),
                             pos, nullptr, 0);
        return true;
    }
    return false;
}

void daPyMng_c::createCourseInit() {
    dScStage_c *stage = dScStage_c::getInstance();
    u8 action = getPlayerCreateAction();
    mVec3_c pos;

    // Written as three nested single-value checks, not `action != 0 && != 1
    // && != 0x17`: the flattened form lets MWCC fold the 0/1 pair into one
    // `cmplwi/ble` range test, but the target emits three separate `beq`s
    // (see BATCH2.md).
    if (action != 0) {
        if (action != 1) {
            if (action != 0x17) {
                makeCourseInList(daPyDemoMng_c::mspInstance);
                pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
                u8 flag;
                if (pos.x <= dGameCom::getDispCenterX()) {
                    flag = 0;
                } else {
                    flag = 1;
                }
                for (int i = 0; i < 4; i++) {
                    create(i, &pos, action, flag);
                }
                return;
            }
        }
    }

    daPyDemoMng_c::mspInstance->init();
    decideCtrlPlrNo();

    if (dScStage_c::m_isStaffCredit) {
        static const float scOfsX[4] = {-184.0f, 200.0f, -208.0f, 224.0f};
        static const float scOfsY[4] = {-48.0f, -48.0f, 0.0f, 0.0f};
        pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
        pos.x = 504.0f;
        for (int i = 0; i < 4; i++) {
            u8 flag = (scOfsX[i] >= 0.0f) ? 1 : 0;
            mVec3_c p2;
            p2.x = pos.x + scOfsX[i];
            p2.y = pos.y + scOfsY[i];
            p2.z = pos.z;
            create(i, &p2, action, flag);
        }
        return;
    }

    pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
    dCdFile_c *file = dCd_c::getFileP(stageField_0x120e(stage));
    u8 scrollDir = (u8)file->mpScrollData->mID;

    if (dScStage_c::m_isCourseIn) {
        dInfo_c *info = dInfo_c::getInstance();
        if (infoField_0xaf4(info) >= 0) {
            pos = infoField_0x10(info);
            scrollDir = infoField_0x1c(info) & 1;
        }
    }

    int order[4] = {-1, -1, -1, -1};
    if (dScStage_c::m_gameMode == 2) {
        order[0] = 0;
        order[1] = 1;
        order[2] = 2;
        order[3] = 3;
    } else {
        float weight[4];
        for (int i = 0; i < 4; i++) {
            weight[i] = 0.1f + dGameCom::rnd();
        }
        int count = 0;
        for (int i = 0; i < 4; i++) {
            int j = 0;
            for (; j < count; j++) {
                if (weight[i] < weight[j]) {
                    break;
                }
            }
            for (int k = count; k > j; k--) {
                order[k] = order[k - 1];
            }
            if (j < 4) {
                order[j] = i;
            }
            if (count < 4) {
                count++;
            }
        }
    }

    int noBalloonCount = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i] != 0 && !isCreateBalloon(i)) {
            noBalloonCount++;
        }
    }

    float xStep = 12.0f * (float)(noBalloonCount - 1);
    if (scrollDir != 0) {
        pos.x -= xStep;
    } else {
        pos.x += xStep;
    }

    for (int i = 0; i < 4; i++) {
        int plrNo = order[i];
        if (plrNo == -1) {
            continue;
        }
        if (!create(plrNo, &pos, action, scrollDir)) {
            continue;
        }
        if (isCreateBalloon(plrNo)) {
            continue;
        }
        if (scrollDir == 0) {
            pos.x += 24.0f;
        } else {
            pos.x -= 24.0f;
        }
    }
}

// .sdata2 literal -- indexed by `flag`, NOT hand-declared as a named object
// per SHARED-BRIEF (0x8042BD70, {0x19, 0x1a}). File scope (not a
// function-local static) is deliberate: a function-local `static const`
// here mangles as `@LOCAL@fn_8005f4d0...@scBaseID` (proven -- see
// BATCH2.md), which does NOT match the target's anonymous `lbl_8042BD70`
// pool form.
static const int scBaseID[2] = {0x19, 0x1a};

bool daPyMng_c::fn_8005f4d0(mVec3_c *pos, int mode, int flag) {
    for (int i = 0; i < 4; i++) {
        if (getPlayer(i) == nullptr) {
            fn_8005f570((PLAYER_POWERUP_e)mode, i);
            create(i, pos, scBaseID[flag], 0);
            return true;
        }
    }
    return false;
}

bool daPyMng_c::fn_8005f570(PLAYER_POWERUP_e mode, int i) {
    u8 idx = i;
    int type = mPlayerType[i];
    u8 mask = 1 << idx;
    mActPlayerInfo |= mask;
    mPlayerEntry[i] = 1;
    mCreateItem[type] = 8;
    mKinopioMode = mode;
}

// --- B3: 0x8005F5C0-0x8005FA5F -----------------------------------------
//
// update() and setHipAttackQuake() (B7, further down) both touch
// mRest/m_quakeTimer/m_quakeEffectFlag through a hand-rolled `char *base =
// (char *) m_playerID` pointer cast in the batches' own isolated drafts.
// Per SHARED-BRIEF section 0 (confirmed by probe): once the static
// definitions are present and enough arrays/uses exist in one TU, MWCC
// emits the base-anchored addressing form BY ITSELF from ordinary array
// syntax. The pointer-cast workaround is therefore reverted here to plain
// `mRest[i]` / `m_quakeTimer[i]` / `m_quakeEffectFlag[i]` array accesses.

void daPyMng_c::update() {
    checkLastAlivePlayer();

    dGameDisplay_c *disp = dScStage_c::getGameDisplay();
    if (disp != nullptr) {
        int buf[4];
        for (int j = 0; j < 4; j++) {
            buf[j] = mRest[j];
        }
        disp->setPlayNum(buf);
        disp->setCoinNum(getCoinAll());
        disp->setScore(mScore);
        disp->setCollect();
    }

    for (int i = 0; i < 4; i++) {
        if (m_quakeTimer[i] != 0) {
            m_quakeTimer[i]--;
            if (m_quakeTimer[i] == 0) {
                m_quakeEffectFlag[i] = 0;
            }
        }
    }

    if (dNext_c::m_instance->mNextDataSet) {
        bool found = false;
        for (int i = 0; i < 4; i++) {
            if (checkPlayer(i)) {
                dAcPy_c *p = getCtrlPlayer(i);
                if (p != nullptr) {
                    if (!p->isStatus(daPlBase_c::STATUS_64) && !p->isWaitFrameCountMax()) {
                        found = true;
                    }
                }
            }
        }
        if (!found) {
            dNext_c::m_instance->mMultiplayerDelay = 0;
        }
    }

    if (dQuake_c::m_instance->mFlags & 0x38) {
        if (mQuakeTrigger == 0) {
            for (int i = 0; i < 4; i++) {
                dAcPy_c *p = getCtrlPlayer(i);
                if (p != nullptr) {
                    if (dQuake_c::m_instance->mFlags & 0x20) {
                        p->onStatus(daPlBase_c::STATUS_QUAKE_BIG);
                    } else if (dQuake_c::m_instance->mFlags & 0x08) {
                        p->onStatus(daPlBase_c::STATUS_QUAKE_SMALL);
                    } else if (m_quakeEffectFlag[i] == 0) {
                        p->onStatus(daPlBase_c::STATUS_QUAKE_SMALL);
                    }
                }
            }
        }
        mQuakeTrigger = 1;
    } else {
        mQuakeTrigger = 0;
    }

    if (mPauseDisable == 0) {
        PauseManager_c::m_instance->setPauseEnable(true);
    } else {
        PauseManager_c::m_instance->setPauseEnable(false);
    }

    if (mStopTimerInfo != mStopTimerInfoOld) {
        if (mStopTimerInfo != 0) {
            dStageTimer_c::m_instance->mStopped = true;
        } else {
            dStageTimer_c::m_instance->mStopped = false;
        }
        mStopTimerInfoOld = mStopTimerInfo;
    }

    daPyDemoMng_c::mspInstance->update();
    dPyEffectMng_c::mspInstance->update();
}

bool daPyMng_c::isPlayerPauseEnable(s8 plrNo) {
    if (checkPlayer(plrNo) && (mPauseEnableInfo & (1 << plrNo))) {
        return true;
    }
    return false;
}

void daPyMng_c::setPlayer(int idx, dAcPy_c *player) {
    if (player == nullptr) {
        m_playerID[idx] = 0;
    } else {
        m_playerID[idx] = player->mUniqueID;
    }
}

dAcPy_c *daPyMng_c::getPlayer(int idx) {
    return (dAcPy_c *) fManager_c::searchBaseByID((fBaseID_e) m_playerID[idx]);
}

void daPyMng_c::decideCtrlPlrNo() {
    for (int i = 0; i < 4; i++) {
        if (mActPlayerInfo & (1 << i)) {
            mCtrlPlrNo = i;
            return;
        }
    }
}

bool daPyMng_c::setYoshi(daPlBase_c *player) {
    if (player == nullptr) {
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (m_yoshiID[i] == 0) {
            m_yoshiID[i] = player->mUniqueID;
            return true;
        }
    }
    return false;
}

void daPyMng_c::releaseYoshi(daPlBase_c *player) {
    if (player == nullptr) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        int id = m_yoshiID[i];
        if (id == player->mUniqueID) {
            m_yoshiID[i] = 0;
            return;
        }
    }
}

// --- B4: 0x8005FA60-0x8005FDAF ----------------------------------------

daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (getVfunc6c(base)((dActor_c *)base) == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}

int daPyMng_c::getYoshiNum() {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]) != nullptr) {
            count++;
        }
    }
    return count;
}

daYoshi_c *daPyMng_c::getYoshiDirectP(int idx) {
    return (daYoshi_c *)fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[idx]);
}

dAcPy_c *daPyMng_c::getCtrlPlayer(int plrNo) {
    dAcPy_c *player = getPlayer(plrNo);
    if (player == nullptr) {
        return nullptr;
    }
    daYoshi_c *yoshi = player->getRideYoshi();
    if (yoshi != nullptr) {
        return (dAcPy_c *)yoshi;
    }
    return player;
}

dPyMdlMng_c::ModelType_e daPyMng_c::getCourseInPlayerModelType(u8 idx) {
    static const dPyMdlMng_c::ModelType_e scModelTypeDt[4] = {
        (dPyMdlMng_c::ModelType_e)0,
        (dPyMdlMng_c::ModelType_e)1,
        (dPyMdlMng_c::ModelType_e)2,
        (dPyMdlMng_c::ModelType_e)3,
    };
    PLAYER_TYPE_e type = mPlayerType[idx];
    if (mCreateItem[type] & 0x8) {
        return (dPyMdlMng_c::ModelType_e)4;
    }
    return scModelTypeDt[type];
}

void daPyMng_c::setCarryOverYoshiInfo(u8 plrNo, u8 yoshiColor, int fruitCount) {
    m_yoshiColor[plrNo] = yoshiColor;
    m_yoshiFruit[plrNo] = fruitCount;
}

int daPyMng_c::getYoshiColor(u8 plrNo) {
    return m_yoshiColor[plrNo];
}

int daPyMng_c::getYoshiFruit(u8 plrNo) {
    return m_yoshiFruit[plrNo];
}

int daPyMng_c::getActScrollInfo() {
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scrollFlagRef(player) != 1) {
                    u8 bit = 1 << i;
                    mask |= bit;
                }
            } else {
                u8 bit = 1 << i;
                mask |= bit;
            }
        }
    }
    return mask;
}

int daPyMng_c::getScrollNum() {
    u8 count = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scrollFlagRef(player) != 1) {
                    count++;
                }
            } else {
                count++;
            }
        }
    }
    return count;
}

// --- B5: 0x8005FDB0-0x8006024F -----------------------------------------

bool daPyMng_c::addNum(int plrNo) {
    u8 idx = plrNo;
    u32 bit = 1 << idx;
    if (!(mActPlayerInfo & bit)) {
        mActPlayerInfo |= bit;
        dAcPy_c *player = getPlayer(plrNo);
        if (player == nullptr || !player->isItemKinopio()) {
            addNum();
        }
        return true;
    }
    return false;
}

bool daPyMng_c::decNum(int plrNo) {
    u8 idx = plrNo;
    u32 bit = 1 << idx;
    if (mActPlayerInfo & bit) {
        mActPlayerInfo &= ~bit;
        dAcPy_c *player = getPlayer(plrNo);
        if (player == nullptr || !player->isItemKinopio()) {
            decNum();
        }
        decideCtrlPlrNo();
        return true;
    }
    return false;
}

void daPyMng_c::addNum() {
    if (mNum < 4) {
        mNum++;
    }
}

void daPyMng_c::decNum() {
    if (mNum > 0) {
        mNum--;
    }
}

int daPyMng_c::getNumInGame() {
    u8 num = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i] && mRest[mPlayerType[i]] > 0) {
            num++;
        }
    }
    return num;
}

u32 daPyMng_c::getEntryNum() {
    u8 num = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i] != 0) {
            num++;
        }
    }
    return num;
}

int daPyMng_c::getItemKinopioNum() {
    u8 num = 0;
    for (int i = 0; i < 4; i++) {
        dAcPy_c *player = getPlayer(i);
        if (player != nullptr && player->isItemKinopio()) {
            num++;
        }
    }
    return num;
}

dAcPy_c *daPyMng_c::getItemKinopio() {
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        player = getPlayer(i);
        if (player != nullptr && player->isItemKinopio()) {
            return player;
        }
    }
    return nullptr;
}

int daPyMng_c::getPlayerIndex(PLAYER_TYPE_e type) {
    for (int i = 0; i < 4; i++) {
        if (mPlayerType[i] == type) {
            return i;
        }
    }
    return -1;
}

bool daPyMng_c::changeItemKinopioPlrNo(int &idx) {
    dAcPy_c *player = getPlayer(idx);
    if (player != nullptr && player->isItemKinopio()) {
        idx = 0;
        return true;
    }
    return false;
}

int daPyMng_c::getCourseInListPlrNo(int idx) {
    return mCourseInList[idx];
}

int daPyMng_c::getCoinAll() {
    return mCoin[mPlayerType[0]] + mCoin[mPlayerType[1]] + mCoin[mPlayerType[2]] +
           mCoin[mPlayerType[3]];
}

// --- B6: 0x80060250-0x800608DF -----------------------------------------

// The three anonymous-namespace constants -- owned by B6 per BATCHES.md.
// MUST be non-const: as `const int`, -O4 constant-folds every use site and
// the symbols are never emitted at all, leaving .sdata 0x10 short (proven by
// B6). Kept here as plain, non-const, file-scope-anonymous-namespace ints.
namespace {
    int scRestMax = 99;
    int scCoinMax = 99;
    int scScoreMax = 999999999;
}

void daPyMng_c::incCoin(int plrNo) {
    daPyMng_c::changeItemKinopioPlrNo(plrNo);
    dMultiMng_c::mspInstance->incCoin(plrNo);

    if (daPyMng_c::getCoinAll() < scCoinMax) {
        mCoin[mPlayerType[plrNo]]++;
    } else {
        int single = 1;
        if (daPyMng_c::getEntryNum() > 1) {
            single = 0;
            dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
            mVec2_c pos(
                bgParam->xStart() + bgParam->xSize() / 2,
                bgParam->yStart() - bgParam->ySize() / 2
            );
            int remote = 0;
            for (int i = 0; i < 4; i++) {
                if (mPlayerEntry[i]) {
                    remote |= dAudio::getRemotePlayer(i);
                }
            }
            dAudio::SoundEffectID_t(SE_SYS_100COIN_ONE_UP).playMapSound(pos, remote);
            dAudio::SoundEffectID_t(SE_SYS_100COIN_ONE_UP_RC).playMapSound(pos, remote);
        }

        bool restFlag = single;
        for (int i = 0; i < 4; i++) {
            if (mPlayerEntry[i]) {
                dAcPy_c *p = daPyMng_c::getPlayer(i);
                if (p != nullptr && !p->isStatus(4)) {
                    dScoreMng_c::m_instance->fn_800e25a0(8, i, single);
                    continue;
                }
            }
            daPyMng_c::addRest(i, 1, restFlag);
        }

        for (int i = 0; i < 4; i++) {
            mCoin[mPlayerType[i]] = 0;
        }
    }
}

bool daPyMng_c::addRest(int plrNo, int amount, bool flag) {
    daPyMng_c::changeItemKinopioPlrNo(plrNo);

    if (flag == 1) {
        dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
        mVec2_c pos(
            bgParam->xStart() + bgParam->xSize() / 2,
            bgParam->yStart() - bgParam->ySize() / 2
        );
        dAudio::SoundEffectID_t(SE_SYS_ONE_UP).playMapSound(pos, dAudio::getRemotePlayer(plrNo));
        dAudio::SoundEffectID_t(SE_SYS_ONE_UP_RC).playMapSound(pos, dAudio::getRemotePlayer(plrNo));
    }

    int max = scRestMax;
    int rest = amount + mRest[daPyMng_c::getPlayerType(plrNo)];
    if (rest >= max) {
        rest = max;
    }
    mRest[daPyMng_c::getPlayerType(plrNo)] = rest;
}

void daPyMng_c::incRestAll(bool b) {
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            daPyMng_c::addRest(i, 1, b);
        }
    }
}

int daPyMng_c::decRest(int plrNo) {
    daPyMng_c::changeItemKinopioPlrNo(plrNo);
    SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_ONE_DOWN, dAudio::getRemotePlayer(plrNo));

    int rest = mRest[mPlayerType[plrNo]];
    if (rest <= 0) {
        return 0;
    }

    mRest[mPlayerType[plrNo]] = rest - 1;
    if (mPlayerType[plrNo] == 0) {
        mBonusNoCap = 0;
    }
    return rest - 1;
}

void daPyMng_c::addScore(int value, int plrNo) {
    int score = value + mScore;
    if (score >= scScoreMax) {
        score = scScoreMax;
    }
    mScore = score;

    if ((u32)plrNo <= 3) {
        daPyMng_c::changeItemKinopioPlrNo(plrNo);
        dMultiMng_c::mspInstance->addScore(value, plrNo);
    }
}

void daPyMng_c::setCourseInStarBGM() {
    if (mBgmState & 1) {
        SndSceneMgr::sInstance->fn_8019bd90(4);
    }
}

void daPyMng_c::startStarBGM() {
    if (!(mBgmState & 1)) {
        mBgmState |= 1;
        SndSceneMgr::sInstance->fn_8019bd90(4);
    }
}

void daPyMng_c::stopStarBGM() {
    if (mBgmState & 1) {
        for (int i = 0; i < 4; i++) {
            dAcPy_c *p = daPyMng_c::getPlayer(i);
            // dAcPy_c's own layout is out of scope for this unit; field
            // @0x1070 is an undeclared timer, not part of the daPyMng_c
            // class.
            if (p != nullptr && *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x1070) >= 0x3c) {
                return;
            }
        }
        mBgmState &= ~1;
        SndSceneMgr::sInstance->fn_8019be60(4);
    }
}

namespace {
    // dAcPy_c's own state-table layout is out of scope for this unit; the
    // target calls through the raw slot at *(p+0x60)+0xE0 with `p` as the
    // sole argument, so reproduce that shape rather than inventing a member.
    struct Unk60Vtbl_c {
        typedef int (*Fn)(dAcPy_c *);
        Fn fn[0x39];
    };
}

void daPyMng_c::startMissBGM(int plrNo) {
    dAcPy_c *p = daPyMng_c::getPlayer(plrNo);
    if (p != nullptr) {
        if (!(*reinterpret_cast<Unk60Vtbl_c **>(reinterpret_cast<u8 *>(p) + 0x60))->fn[0x38](p)
            && *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x1554) != 0) {
            SndSceneMgr::sInstance->startMiss();
        }
    }
}

void daPyMng_c::startYoshiBGM() {
    if (!(mBgmState & 2)) {
        mBgmState |= 2;
        SndSceneMgr::sInstance->fn_8019bd90(0x200);
    }
}

void daPyMng_c::stopYoshiBGM() {
    if (mBgmState & 2) {
        for (int i = 0; i < 4; i++) {
            dAcPy_c *p = daPyMng_c::getPlayer(i);
            if (p != nullptr && p->isStatus(0x4b)) {
                return;
            }
        }
        mBgmState &= ~2;
        SndSceneMgr::sInstance->fn_8019be60(0x200);
    }
}

// --- B7: 0x800608E0-0x80060F1F -----------------------------------------

bool daPyMng_c::checkLastAlivePlayer() {
    bool multiplayer = getEntryNum() > 1;
    if (multiplayer) {
        if (mNum <= 1) {
            if (!(mBgmState & 4)) {
                mBgmState |= 4;
                SndSceneMgr::sInstance->fn_8019bd90(0x400);
            }
        } else {
            if (mBgmState & 4) {
                mBgmState &= ~4;
                SndSceneMgr::sInstance->fn_8019be60(0x400);
            }
        }
    }
    return false;
}

void daPyMng_c::executeLastPlayer() {
    for (int i = 0; i < 4; i++) {
        daYoshi_c *p = static_cast<daYoshi_c *>(fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]));
        if (p) {
            p->executeLastPlayer();
        }
    }
    for (int i = 0; i < 4; i++) {
        dAcPy_c *p = getPlayer(i);
        if (p) {
            p->executeLastPlayer();
        }
    }
}

void daPyMng_c::executeLastAll() {
    for (int i = 0; i < 4; i++) {
        daYoshi_c *p = static_cast<daYoshi_c *>(fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]));
        if (p) {
            p->executeLastAll();
        }
    }
    for (int i = 0; i < 4; i++) {
        dAcPy_c *p = getPlayer(i);
        if (p) {
            p->executeLastAll();
        }
    }
}

int daPyMng_c::deleteCullingYoshi() {
    dBgParameter_c *bg = dBgParameter_c::ms_Instance_p;
    mVec2_c mid;
    mid.y = bg->yStart() - bg->ySize() * 0.5f;
    mid.x = bg->xStart() + bg->xSize() * 0.5f;

    fBase_c *farthest = 0;
    float farthestDist = 0.0f;
    for (int i = 0; i < 4; i++) {
        daYoshi_c *p = static_cast<daYoshi_c *>(fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]));
        if (!p) continue;
        if (isDeleteRequested(p)) continue;
        if (getVfunc6c(p)((dActor_c *)p) != -1) continue;
        if (!p->isStatus(daPlBase_c::STATUS_DISPLAY_OUT_DEAD)) continue;

        mVec2_c ppos;
        ppos.y = p->mPos.y;
        ppos.x = p->mPos.x;
        mVec2_c delta;
        delta.y = mid.y - ppos.y;
        delta.x = mid.x - ppos.x;
        float distSq = delta.x * delta.x + delta.y * delta.y;
        float dist = EGG::Mathf::sqrt(distSq);
        if (dist > farthestDist) {
            farthestDist = dist;
            farthest = p;
        }
    }

    if (farthest) {
        farthest->deleteRequest();
        return 1;
    }
    return 0;
}

void daPyMng_c::setHipAttackQuake(int type, u8 plrNo) {
    // .bss:0x80355FB0 -- three ints immediately after mEffectMng, belonging
    // to nobody's header (dtk labels the whole 0x10 there a padding gap, but
    // this function genuinely reads and writes the first 0xC of it: a
    // lazily-initialized, one-shot table of three sound-effect IDs, found by
    // B7). It is a function-local static, not a raw offset from m_playerID
    // -- see the SHARED-BRIEF/B3 note above update(): once the class's own
    // .bss statics are defined ahead of it in this TU (as they are, at the
    // top of this file) and there are enough uses, MWCC anchors this local
    // static off the same base register as m_quakeTimer/m_quakeEffectFlag
    // by itself, from ordinary array syntax -- no hand-rolled pointer
    // arithmetic needed or wanted.
    static int seTable[3];

    if (plrNo == -1) return;
    if (type == 2) {
        dQuake_c::m_instance->shockMotor((s8)plrNo, dQuake_c::TYPE_7, 0, false);
        return;
    }

    m_quakeTimer[plrNo] = 5;
    m_quakeEffectFlag[plrNo] = 0;
    int count = 0;
    if (!dScStage_c::m_isStaffCredit) {
        for (int j = 0; j < 4; j++) {
            if (j == plrNo) continue;
            if (m_quakeTimer[j] != 0) {
                m_quakeTimer[j] = 5;
                count++;
            }
        }
    }

    if (count != 0) {
        if (!lbl_80429FD0) {
            seTable[0] = 0x152;
            seTable[1] = 0x151;
            seTable[2] = 0x150;
            lbl_80429FD0 = 1;
        }
        if ((u32)(count - 1) <= 2) {
            SndAudioMgr::sInstance->startSystemSe((unsigned long)seTable[count - 1], 1);
        }
        fn_80060DB0();
        return;
    }

    if (type == 1) {
        dQuake_c::m_instance->startShock((s8)plrNo, dQuake_c::TYPE_3, 3, 0, false);
    } else {
        dQuake_c::m_instance->shockMotor((s8)plrNo, dQuake_c::TYPE_4, 0, false);
    }
}

static void fn_80060DB0() {
    SndSceneMgr::sInstance->onPowerImpact();
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::m_quakeTimer[i] != 0 && daPyMng_c::m_quakeEffectFlag[i] == 0) {
            daPyMng_c::m_quakeEffectFlag[i] = 1;
            dAcPy_c *py = daPyMng_c::getPlayer(i);
            if (!py) continue;
            daPlBase_c *p = py;
            if (py->isStatus(daPlBase_c::STATUS_RIDE_YOSHI)) {
                p = py->getRideYoshi();
            }
            if (!p) continue;
            mVec3_c pos;
            pos.x = p->mPos.x;
            pos.y = p->mPos.y;
            pos.z = p->mPos.z;
            mEf::createEffect("Wm_mr_vshipattack", 0, &pos, 0, 0);
            pos.z = 3800.0f;
            mEf::createEffect("Wm_mr_vshipattack_ind", 0, &pos, 0, 0);
            dQuake_c::m_instance->startShock((s8)i, dQuake_c::TYPE_3, 3, 0x12, false);
        }
    }
}

void daPyMng_c::checkBonusNoCap() {
    mBonusNoCap = 0;
    if (mRest[0] >= 0x63) {
        mBonusNoCap = 1;
    }
}

// --- B8: 0x80060F20-0x80061304 ------------------------------------------

void daPyMng_c::initYoshiPriority(daPlBase_c *player) {
    u8 used[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
        if (m_yoshiID[i] != player->mUniqueID) {
            fBase_c *other = fManager_c::searchBaseByID((fBaseID_e) m_yoshiID[i]);
            if (other != nullptr) {
                used[((daPlBase_c *) other)->mPlayerLayer] = 1;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        if (!used[i]) {
            player->mPlayerLayer = i;
            setYoshiPriority(player);
            break;
        }
    }
}

void daPyMng_c::setYoshiPriority(daPlBase_c *player) {
    u8 oldPriority = player->mPlayerLayer;
    player->mPlayerLayer = 0;
    for (int i = 0; i < 4; i++) {
        if (m_yoshiID[i] == player->mUniqueID) {
            continue;
        }
        fBase_c *other = fManager_c::searchBaseByID((fBaseID_e) m_yoshiID[i]);
        if (other == nullptr) {
            continue;
        }
        u8 otherPriority = ((daPlBase_c *) other)->mPlayerLayer;
        if (oldPriority > otherPriority) {
            ((daPlBase_c *) other)->mPlayerLayer = otherPriority + 1;
        }
    }
}

bool daPyMng_c::isEffectStop(int plrNo) {
    if (infoField_0xafc(dInfo_c::getInstance())) {
        return false;
    }
    dAcPy_c *player = getPlayer(plrNo);
    if (player != nullptr && (dActor_c::mExecStop & player->mExecStopMask)) {
        return true;
    }
    return false;
}

bool daPyMng_c::isAcceptQuake(int plrNo) {
    return checkPlayer(plrNo);
}

bool daPyMng_c::isCreateBalloon(int plrNo) {
    PLAYER_TYPE_e type = mPlayerType[plrNo];
    if (mCreateItem[type] & 4) {
        return true;
    }
    if (mRest[type] > 0) {
        return false;
    }
    return true;
}

void daPyMng_c::checkCorrectCreateInfo() {
    for (int idx = 0; idx < 4; idx++) {
        if (mPlayerType[idx] > 3) {
            mPlayerType[idx] = (PLAYER_TYPE_e) 0;
        }
        PLAYER_POWERUP_e mode = mPlayerMode[mPlayerType[idx]];
        if (mode > 6) {
            mPlayerMode[mPlayerType[idx]] = (PLAYER_POWERUP_e) 0;
        }
        if (mCreateItem[mPlayerType[idx]] & 0xE) {
            mCreateItem[mPlayerType[idx]] = mode & 1;
        }
        if (mRest[mPlayerType[idx]] < 0 || mRest[mPlayerType[idx]] > scRestMax) {
            mRest[mPlayerType[idx]] = 5;
        }
    }
    if (mKinopioMode > 6) {
        mKinopioMode = (PLAYER_POWERUP_e) 0;
    }
    if (getCoinAll() > scCoinMax) {
        mCoin[mPlayerType[0]] = 0;
        mCoin[mPlayerType[1]] = 0;
        mCoin[mPlayerType[2]] = 0;
        mCoin[mPlayerType[3]] = 0;
    }
    if (mScore > scScoreMax) {
        mScore = scScoreMax;
    }
}
