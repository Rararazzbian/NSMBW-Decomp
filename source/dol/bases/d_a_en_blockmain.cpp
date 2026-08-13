#include <game/bases/d_a_en_blockmain.hpp>
#include <game/bases/d_a_en_shell.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_actorcreate_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_block_mng.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_yoshi_mdl.hpp>
#include <game/mLib/m_effect.hpp>
#include <constants/sound_list.h>

/// @brief The block's rising/falling speed. @unofficial
/// @note Nothing in this TU loads it -- the RELs do -- but it is defined here,
/// and it is the FIRST .sdata2 object of the unit (0x8042B570). It must stay at
/// the top of the file or the whole .sdata2 pool shifts.
const float daEnBlockMain_c::c_YSPD = 2.0f;

/// @brief The bg-collision flag word inside @ref dBg_ctr_c.
/// @details `d_bg_ctr.hpp` still spells offset 0xD8 as padding; this reaches it
/// without moving anything in that shared header. @unofficial
#define BLOCK_BG_FLAGS(block) (*(u32 *)(block)->mBgCtr.mpPad5)

/// @brief Describes one "block was hit, spawn its item" request. @unofficial
struct sBlockItemInfo {
    mVec3_c mPos;   ///< [0x00] Spawn position; also the sound position.
    s8 mPlayerNo;   ///< [0x0C] The player who hit the block, or -1.
    int mIndex;     ///< [0x10] Index into the item-number tables, 0..17.
    u8 mAltTable;   ///< [0x14] Selects table B and sets bit 0x1000 in the params.
    u8 m_15;        ///< [0x15]
    u8 m_16;        ///< [0x16]
};

void block_item_set(daEnBlockMain_c *self, sBlockItemInfo *info, int alt);
void block_multi_item_set(daEnBlockMain_c *self, sBlockItemInfo *info, int num, u32 arg);

// The six state definitions. These generate the six baseID_* stubs that open
// this unit's .text at 0x800208B0, __sinit_\d_a_en_blockmain_cpp, and the eight
// trailing sFStateID_c / sFStateVirtualID_c instantiations. The ORDER is fixed
// by the .bss addresses of the StateIDs -- do not permute it.
STATE_VIRTUAL_DEFINE(daEnBlockMain_c, UpMove);
STATE_VIRTUAL_DEFINE(daEnBlockMain_c, DownMove);
STATE_VIRTUAL_DEFINE(daEnBlockMain_c, DownMoveEnd);
STATE_VIRTUAL_DEFINE(daEnBlockMain_c, UpMove_Diff);
STATE_VIRTUAL_DEFINE(daEnBlockMain_c, DownMove_Diff);
STATE_VIRTUAL_DEFINE(daEnBlockMain_c, DownMove_DiffEnd);


// ---------------------------------------------------------------- 0x80020910
void daEnBlockMain_c::common_callBackF_enemy(dActor_c *self, dActor_c *other) {
    daEnBlockMain_c *block = (daEnBlockMain_c *)self;
    if (other->mKind == STAGE_ACTOR_ENEMY) {
        dEn_c *enemy = (dEn_c *)other;
        if (enemy->PlayerCarryCheck(other)) {
            return;
        }
        if (block->m_689 != 0) {
            return;
        }
        if (block->m_67f == 0) {
            return;
        }
        other->mPlayerNo = block->getPlrNo();
        if (!(other->mActorProperties & 4)) {
            other->mActorProperties |= 4;
            other->mComboMultiplier++;
            if (other->mComboMultiplier >= 8) {
                other->mComboMultiplier = 8;
            }
            int comboCount = other->mComboMultiplier;
            enemy->mCombo.setScore(other, comboCount, other->getPlrNo());
        }
        other->mBlockHit = true;
        enemy->mDeathFallDirection = getTrgToSrcDir_Main(other->getCenterX(), self->getCenterX());
        if (other->mProfName == fProfile::EN_ITEM) {
            u16 itemNo = *(u16 *)((u8 *)other + 0xdca);
            if (itemNo == 0xe || itemNo == 0xb || itemNo == 1) {
                block->m_667 = 1;
            }
        }
    } else if (other->mProfName == fProfile::ICE_ACTOR) {
        if (block->m_689 != 0) {
            return;
        }
        if (block->m_67f == 0) {
            return;
        }
        other->mPlayerNo = block->getPlrNo();
        other->mBlockHit = true;
    }
}

// ---------------------------------------------------------------- 0x80020AA0
daEnBlockMain_c *daEnBlockMain_c::common_callBackH(dActor_c *self, dActor_c *other) {
    if (!(other->mKind != STAGE_ACTOR_PLAYER && other->mKind != STAGE_ACTOR_YOSHI)) {
        daEnBlockMain_c *block = (daEnBlockMain_c *)self;
        u32 flags = BLOCK_BG_FLAGS(block);
        u32 hard = flags & 8;
        if (hard != 0 || (flags & 0x20) != 0) {
            if (hard != 0) {
                block->m_64c = 1;
            }
            if (block->m_660 != 0 && block->m_689 == 1) {
                block->m_68a |= 2;
            }
            s8 playerNo = other->getPlrNo();
            if (playerNo != -1) {
                dAcPy_c *player;
                dAcPy_c *ridePlayer;
                if (other != nullptr && ((daPlBase_c *)other)->isItemKinopio() &&
                    (player = daPyMng_c::getPlayer(playerNo)) != nullptr &&
                    (ridePlayer = player->getRidePlayer()) != nullptr) {
                    playerNo = ridePlayer->getPlrNo();
                }
                block->mHitPlayerH[playerNo] = playerNo;
                block->m_680 = 1;
                block->mPlayerNo = playerNo;
            }
            return block;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------- 0x80020BF0
void daEnBlockMain_c::callBackF(dActor_c *self, dActor_c *other) {
    if (!(other->mKind != STAGE_ACTOR_PLAYER && other->mKind != STAGE_ACTOR_YOSHI)) {
        daEnBlockMain_c *block = (daEnBlockMain_c *)self;
        if (other != nullptr) {
            daPlBase_c *player = (daPlBase_c *)other;
            if (block->m_689 == 0 && block->m_660 != 0) {
                if (player->isStatus(daPlBase_c::STATUS_HIP_ATTACK_FALL) ||
                    player->isStatus(daPlBase_c::STATUS_SPIN_HIP_ATTACK_FALL)) {
                    block->m_68a |= 1;
                }
            }
            s8 playerNo = other->getPlrNo();
            if (playerNo != -1) {
                block->mHitPlayerF[playerNo] = playerNo;
                block->m_685[playerNo] = 1;
            }
            bool small = false;
            if (player->mPowerup == POWERUP_NONE || player->mPowerup == POWERUP_MINI_MUSHROOM) {
                small = true;
            }
            if (player->isStatus(daPlBase_c::STATUS_SPIN_HIP_ATTACK_FALL)) {
                block->m_680 = 2;
            }
            if (block->m_68b == 1 && !small) {
                if (player->isStatus(daPlBase_c::STATUS_HIP_ATTACK_STAND_UP)) {
                    block->m_680 = 2;
                }
            } else if (player->isStatus(daPlBase_c::STATUS_HIP_ATTACK_FALL)) {
                block->m_680 = 2;
            }
        }
    }
    common_callBackF_enemy((daEnBlockMain_c *)self, other);
}

// ---------------------------------------------------------------- 0x80020D60
void daEnBlockMain_c::nomal_callBackF(dActor_c *self, dActor_c *other) {
    if (!(other->mKind != STAGE_ACTOR_PLAYER && other->mKind != STAGE_ACTOR_YOSHI)) {
        daEnBlockMain_c *block = (daEnBlockMain_c *)self;
        if (other != nullptr) {
            daPlBase_c *player = (daPlBase_c *)other;
            if (block->m_689 == 0 && block->m_660 != 0) {
                if (player->isStatus(daPlBase_c::STATUS_HIP_ATTACK_FALL) ||
                    player->isStatus(daPlBase_c::STATUS_SPIN_HIP_ATTACK_FALL)) {
                    block->m_68a |= 1;
                }
            }
            s8 playerNo = other->getPlrNo();
            if (playerNo != -1) {
                block->mHitPlayerF[playerNo] = playerNo;
                block->m_685[playerNo] = 1;
            }
            if (player->isStatus(daPlBase_c::STATUS_HIP_ATTACK_FALL) ||
                player->isStatus(daPlBase_c::STATUS_SPIN_HIP_ATTACK_FALL)) {
                block->m_680 = 2;
            }
        }
    }
    common_callBackF_enemy((daEnBlockMain_c *)self, other);
}

// ---------------------------------------------------------------- 0x80020E70
void daEnBlockMain_c::callBackH(dActor_c *self, dActor_c *other) {
    daEnBlockMain_c *block = common_callBackH(self, other);
    if (block != nullptr) {
        block->m_67f = 1;
    }
}

// ---------------------------------------------------------------- 0x80020EA0
void daEnBlockMain_c::nomal_callBackH(dActor_c *self, dActor_c *other) {
    daEnBlockMain_c *block = common_callBackH(self, other);
    if (block != nullptr) {
        block->m_67f = 1;
    }
}

// ---------------------------------------------------------------- 0x80020ED0
void daEnBlockMain_c::side_block_moveset(daEnBlockMain_c *self, dActor_c *other, u8 dir) {
    self->m_680 = 3;
    s8 playerNo = other->getPlrNo();
    if (playerNo != -1) {
        self->mHitPlayerW[playerNo] = playerNo;
        self->m_674 = playerNo;
    }
    self->m_68d = dir;
}

// ---------------------------------------------------------------- 0x80020F40
bool daEnBlockMain_c::shell_callBackW(dActor_c *self, dActor_c *other, u8 dir) {
    if (other->mKind == STAGE_ACTOR_ENEMY) {
        if (((dEn_c *)other)->mFlags & dEn_c::EN_IS_SHELL) {
            if (((daEnShell_c *)other)->isState(daEnShell_c::StateID_Slide)) {
                side_block_moveset((daEnBlockMain_c *)self, other, dir);
                return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------- 0x80021010
void daEnBlockMain_c::callBackW(dActor_c *self, dActor_c *other, u8 dir) {
    daEnBlockMain_c *block = (daEnBlockMain_c *)self;
    u32 flags = BLOCK_BG_FLAGS(block);
    if (flags & 0x28) {
        if (flags & 8) {
            block->m_64c = 1;
        }
        side_block_moveset(block, other, dir);
    } else if (!(other->mKind != STAGE_ACTOR_PLAYER && other->mKind != STAGE_ACTOR_YOSHI)) {
        s8 playerNo = other->getPlrNo();
        if (playerNo != -1) {
            block->m_681[playerNo] = dir + 1;
            block->mHitPlayerW[playerNo] = playerNo;
        }
    } else if (!shell_callBackW(self, other, dir)) {
        enemy_only_callBackW(block, other, dir);
    }
}

// ---------------------------------------------------------------- 0x800210F0
void daEnBlockMain_c::obj_callBackW(dActor_c *self, dActor_c *other, u8 dir) {
    callBackW(self, other, dir);
}

// ---------------------------------------------------------------- 0x80021100
void daEnBlockMain_c::enemy_only_callBackW(dActor_c *self, dActor_c *other, u8 dir) {
    daEnBlockMain_c *block = (daEnBlockMain_c *)self;
    if (block == nullptr) {
        return;
    }
    u32 flags = BLOCK_BG_FLAGS(block);
    if (flags & 2) {
        block->m_67d = 1;
        block->m_67e = dir;
    } else if (flags & 0x28) {
        block->m_67f = 1;
    }
}

// ---------------------------------------------------------------- 0x80021140
void daEnBlockMain_c::playeronly_callBackF(dActor_c *self, dActor_c *other) {}

// ---------------------------------------------------------------- 0x80021150
void daEnBlockMain_c::playeronly_callBackH(dActor_c *self, dActor_c *other) {}

// ---------------------------------------------------------------- 0x80021160
void daEnBlockMain_c::playeronly_callBackW(dActor_c *self, dActor_c *other, u8 dir) {
    callBackW(self, other, dir);
}

// ---------------------------------------------------------------- 0x80021170
bool daEnBlockMain_c::checkRevHead(dActor_c *self, dActor_c *other) {
    return false;
}

// ---------------------------------------------------------------- 0x80021180
bool daEnBlockMain_c::checkRevFoot(dActor_c *self, dActor_c *other) {
    return self->mSpeed.y > 0.0f;
}

// ---------------------------------------------------------------- 0x800211A0
bool daEnBlockMain_c::checkRevWall(dActor_c *self, dActor_c *other, u8 dir) {
    return false;
}

// ---------------------------------------------------------------- 0x800211B0
/// @brief @unofficial File-static helper of @ref clear_block_collcallback.
/// @details Pushes an actor that hit the "clear block" from below back down.
static void clear_block_pushback(dCc_c *self, dCc_c *other, float depth, u8 dir) {
    dActor_c *actor = other->mpOwner;
    if (actor == nullptr) {
        return;
    }
    if (actor->mSpeed.y > 0.0f && depth > 0.0f) {
        daEnBlockMain_c *block = (daEnBlockMain_c *)self->mpOwner;
        if (block->m_68c == 0) {
            block->m_662 = 1;
            block->m_694 = actor->getPlrNo();
            mVec3_c pos = actor->mPos;
            pos.y -= 2.0f * depth;
            actor->mPos.y = pos.y;
            actor->mSpeed.y = 0.0f;
        }
    }
}

// ---------------------------------------------------------------- 0x80021280
void daEnBlockMain_c::clear_block_collcallback(dCc_c *self, dCc_c *other) {
    dActor_c *actor = other->mpOwner;
    if (actor->mKind == STAGE_ACTOR_PLAYER) {
        clear_block_pushback(self, other, self->getYOffset(CC_KIND_PLAYER), 0);
    } else if (actor->mKind == STAGE_ACTOR_YOSHI) {
        clear_block_pushback(self, other, self->getYOffset(CC_KIND_YOSHI), 1);
    }
}

// ---------------------------------------------------------------- BATCH 2
// Background collision, the two PonCheck routines and the clear-sets.
// Canonical .text order: 0x800212C0, 0x80021470, 0x800214F0, 0x80021580,
// 0x80021600, 0x80021690, 0x80021740, 0x80021780, 0x800217B0.

// 0x800212C0 (424 B)
int daEnBlockMain_c::ObjBgHitCheck() {
    s8 no;
    int ret = 0;

    if (m_67f != 0 || m_64c != 0) {
        m_694 = 0;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            no = mHitPlayerH[i];
            if (no != -1) {
                m_694 = no;
                mBc.mOwningPlrNo = no;
            }
        }

        m_68d = 2;
        if (m_64c != 0) {
            ret = 3;
        } else {
            ret = 1;
        }
    }

    if (m_680 == 2) {
        m_694 = 0;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            no = mHitPlayerF[i];
            if (no != -1) {
                m_694 = no;
            }
        }

        m_68d = 3;
        ret = 2;
    } else if (m_680 == 3) {
        m_694 = 0;
        if (m_64c != 0) {
            ret = 3;
        } else {
            ret = 1;
        }

        m_68d = 2;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            no = mHitPlayerW[i];
            if (no != -1) {
                m_694 = no;
                mBc.mOwningPlrNo = no;
            }
        }
    }

    return ret;
}

/// @brief @unofficial Bounces the player standing on the block. 0x80021470.
static void blockPonJumpSet(daEnBlockMain_c *block, s8 playerNo) {
    daPlBase_c *player = daPyMng_c::getCtrlPlayer(playerNo);
    if (player != nullptr) {
        player->setJump(0.2815f + dAcPy_c::msc_JUMP_SPEED, player->mSpeedF, true, 0, 0);
        block->m_675[playerNo] = 1;
    }
}

// 0x800214F0 (136 B)
void daEnBlockMain_c::ObjBg_PonCheck() {
    if (mSpeed.y <= 0.0f) {
        return;
    }

    if (m_689 == 1) {
        return;
    }

    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (m_675[i] == 0 && mHitPlayerF[i] != -1) {
            blockPonJumpSet(this, mHitPlayerF[i]);
        }
    }
}

/// @brief @unofficial Second, byte-identical copy of @ref blockPonJumpSet.
/// 0x80021580.
static void blockPonJumpSet_jump(daEnBlockMain_c *block, s8 playerNo) {
    daPlBase_c *player = daPyMng_c::getCtrlPlayer(playerNo);
    if (player != nullptr) {
        player->setJump(0.2815f + dAcPy_c::msc_JUMP_SPEED, player->mSpeedF, true, 0, 0);
        block->m_675[playerNo] = 1;
    }
}

// 0x80021600 (136 B)
void daEnBlockMain_c::ObjBg_PonCheck_jump() {
    if (mSpeed.y <= 0.0f) {
        return;
    }

    if (m_689 == 1) {
        return;
    }

    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (m_675[i] == 0 && mHitPlayerF[i] != -1) {
            blockPonJumpSet_jump(this, mHitPlayerF[i]);
        }
    }
}

// 0x80021690 (172 B)
void daEnBlockMain_c::Block_CreateClearSet(float scale) {
    m_630 = scale;
    mScaleMin = 1.0f;
    mScaleStep = 0.0546875f;
    mScaleMax = 1.28125f;

    for (int i = 0; i < PLAYER_COUNT; i++) {
        m_675[i] = 0;
        mHitPlayerF[i] = -1;
        mHitPlayerH[i] = -1;
        mHitPlayerW[i] = -1;
        m_679[i] = 0;
    }

    m_68b = 0;
    m_674 = -1;
    m_67f = 0;
    m_64c = 0;
    m_667 = 0;
    m_67d = 0;
    m_680 = 0;
    m_648 = 0;
    m_660 = 8;
    m_689 = 0;
    m_68a = 2;
}

// 0x80021740 (60 B)
void daEnBlockMain_c::HopCoinBgcheckSet() {
    mHopCoinSensor.mFlags = SENSOR_IS_LINE | 0x80020000;
    mHopCoinSensor.mLineA = -0x7000;
    mHopCoinSensor.mLineB = 0x7000;
    mHopCoinSensor.mDistanceFromCenter = 0x10000;

    mBc.set(this, nullptr, mHopCoinSensor, nullptr);
}

// 0x80021780 (36 B)
bool daEnBlockMain_c::HopCoinBgcheck() {
    if (mBc.mpSensorHead == nullptr) {
        return false;
    }

    return mBc.checkHead(0);
}

// 0x800217B0 (64 B)
void daEnBlockMain_c::Block_ExecuteClearSet() {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        mHitPlayerF[i] = -1;
        mHitPlayerH[i] = -1;
    }

    m_674 = -1;
    m_67f = 0;
    m_64c = 0;
    m_680 = 0;
    m_67d = 0;
}

void daEnBlockMain_c::jumpdai_set() {
    static const float l_jumpdai_ef_ofs[2] = { 24.0f, -8.0f };
    static const float l_jumpdai_ofs[2] = { 16.0f, -16.0f };

    mVec3_c pos(mPos);
    pos.z = 1500.0f;
    if (m_690 == 2 || mLayer != 0) {
        pos.z = -2500.0f;
    }

    u8 idx = m_68d - 2;
    if ((s8) idx < 0) {
        idx = 1;
    }

    mVec3_c efPos(mPos.x, mPos.y + l_jumpdai_ef_ofs[idx], 5500.0f);
    mEf::createEffect("Wm_en_burst_s", 0, &efPos, nullptr, nullptr);

    pos.y += l_jumpdai_ofs[idx];
    dAudio::SoundEffectID_t(SE_OBJ_JUMPDAI_APP).playMapSound(pos, 0);
    dActor_c::construct(fProfile::EN_JUMPDAI, 0x10000000, &pos, nullptr, mLayer);
}

void daEnBlockMain_c::itemkey_set(u8 kind) {
    static const float l_itemkey_ef_ofs[2] = { 24.0f, -8.0f };
    static const float l_itemkey_ofs[2] = { 16.0f, -20.0f };

    mVec3_c pos(mPos);
    pos.z = 1500.0f;
    if (m_690 == 2 || mLayer != 0) {
        pos.z = -2500.0f;
    }

    u8 idx = m_68d - 2;
    if ((s8) idx < 0) {
        idx = 1;
    }

    if (kind != 3) {
        mVec3_c efPos(mPos.x, mPos.y + l_itemkey_ef_ofs[idx], 5500.0f);
        mEf::createEffect("Wm_en_burst_s", 0, &efPos, nullptr, nullptr);
        pos.y += l_itemkey_ofs[idx];
    } else {
        pos.y += 24.0f;
    }

    dActor_c::construct(fProfile::AC_ITEM_KEY, 0, &pos, nullptr, mLayer);
}

void daEnBlockMain_c::item_ivy_set(u8 a, u8 b) {
    mVec3_c pos(mPos);
    if (b == 2) {
        pos.y += 16.0f;
    }

    if (a != 0) {
        pos.z -= 32.0f;
    } else {
        pos.z = -32.0f;
    }

    dActor_c::construct(fProfile::EN_ITEM_IVY, (a << 8) | (b << 4), &pos, nullptr, mLayer);
}

bool daEnBlockMain_c::isYossyColor(u16 color) {
    static const int l_yoshi_color[4] = { 0, 2, 3, 1 };

    int want = l_yoshi_color[color];
    for (int i = 0; i < 4; i++) {
        daYoshi_c *yoshi = daPyMng_c::getYoshiDirectP(i);
        if (yoshi != nullptr && want == yoshi->getModel()->getColor()) {
            return true;
        }
    }
    return false;
}

u16 daEnBlockMain_c::yossy_color_search() {
    u16 color = dActorCreateMng_c::m_instance->mYoshiColor;
    for (int i = 0; i < 4; i++) {
        if (!isYossyColor(color)) {
            dActorCreateMng_c::m_instance->mYoshiColor = color;
            dActorCreateMng_c::m_instance->mYoshiColor = (dActorCreateMng_c::m_instance->mYoshiColor + 1) & 3;
            return color;
        }
        color = (color + 1) & 3;
    }
    return 0xFFFF;
}

// ---------------------------------------------------------------- 0x80021BD0
void daEnBlockMain_c::yossy_set(unsigned long dir) {
    u16 color = yossy_color_search();
    u32 param = color | (dir << 4);
    mVec3_c pos = mPos;

    if (dir == 2) {
        pos.y += 16.0f;
    }

    dAudio::SoundEffectID_t(SE_PLY_YOSHI_EGG_APPEAR).playMapSound(pos, 0);
    dActor_c::construct(fProfile::AC_YOSHI_EGG, param, &pos, nullptr, 0);
}

/// @brief Bits 31 and 28 of the egg's spawn parameter, per spawn slot.
/// @details These sit between isYossyColor's l_yoshi_color (0x802EE5C0) and
/// player_set's l_player_mode (0x802EE5F0) in .rodata, which is what fixes
/// their position here. @unofficial
static const u32 l_yossy_pos_bit31[4] = { 0, 1, 0, 1 };
static const u32 l_yossy_pos_bit28[4] = { 0, 0, 1, 1 };

/// @brief Spawns one Yoshi egg.
/// @note @p pos is taken BY VALUE deliberately: it is what gives the caller's
/// vector the z/y/x -> f0/f1/f2 numbering the original uses, and computing
/// @p param inside this body rather than in a named local at the call site is
/// what colours it into the right callee-saved register. @unofficial
inline void yossy_egg_create(mVec3_c pos, unsigned long dir, u16 color, int idx,
                             unsigned long extra) {
    unsigned long param = (l_yossy_pos_bit31[idx] << 31) |
                          ((l_yossy_pos_bit28[idx] << 28) |
                           (color | (dir << 4) | extra));
    if (dir == 2) {
        pos.y += 16.0f;
    }
    dAudio::SoundEffectID_t(SE_PLY_YOSHI_EGG_APPEAR).playMapSound(pos, 0);
    dActor_c::construct(fProfile::AC_YOSHI_EGG, param, &pos, nullptr, 0);
}



/// @brief Spawns one Yoshi egg, second-loop form.
/// @note Identical to yossy_egg_create except that the sound-object pointer is
/// DECLARED at the top of the body and assigned later. That placement is what
/// colours @p param into r21 and the sound object into r22 in
/// multi_yossy_set's second loop; with the declaration at its assignment the
/// two swap. @unofficial
inline void yossy_egg_create2(mVec3_c pos, unsigned long dir, u16 color, int idx,
                              unsigned long extra) {
    dAudio::SndObjctCmnMap_c *snd;
    unsigned long param = (l_yossy_pos_bit31[idx] << 31) |
                          ((l_yossy_pos_bit28[idx] << 28) |
                           (color | (dir << 4) | extra));
    if (dir == 2) {
        pos.y += 16.0f;
    }
    snd = dAudio::g_pSndObjMap;
    snd->startSound(SE_PLY_YOSHI_EGG_APPEAR, dAudio::cvtSndObjctPos(pos), 0);
    dActor_c::construct(fProfile::AC_YOSHI_EGG, param, &pos, nullptr, 0);
}
// ---------------------------------------------------------------- 0x80021C80
void daEnBlockMain_c::multi_yossy_set(unsigned long dir) {
    int num = playernumber_set() + 1;

    if (num - daPyMng_c::getYoshiNum() <= 0) {
        return;
    }

    int yoshiNum = daPyMng_c::getYoshiNum();
    int count = num - yoshiNum;
    int no = 0;
    u16 color;
    int i;
    int j;

    for (i = 0; i < count; i++) {
        color = yossy_color_search();
        if (color == 0xffff) {
            return;
        }
        yossy_egg_create(mPos, dir, color, i, 0x1000000);
        no = i;
    }

    if (daPyMng_c::getYoshiNum() == 0) {
        return;
    }

    no++;

    for (j = 0; j < yoshiNum; j++) {
        color = (color + 1) & 3;
        yossy_egg_create2(mPos, dir, color, no, 0x1002000);
        no++;
    }
}

// ---------------------------------------------------------------- 0x80021EA0
void daEnBlockMain_c::eggitem_set(unsigned long dir) {
    u16 color = yossy_color_search();
    u32 param = (dir << 4) | (color | 0x2000);
    mVec3_c pos = mPos;

    if (dir == 2) {
        pos.y += 16.0f;
    }

    dAudio::SoundEffectID_t(SE_PLY_YOSHI_EGG_APPEAR).playMapSound(pos, 0);
    dActor_c::construct(fProfile::AC_YOSHI_EGG, param, &pos, nullptr, 0);
}

// ---------------------------------------------------------------- 0x80021F50
void daEnBlockMain_c::multi_eggitem_set(unsigned long dir) {
    int num = playernumber_set() + 1;
    u16 color = yossy_color_search();

    for (int i = 0; i < num; i++) {
        color = (color + 1) & 3;
        yossy_egg_create(mPos, dir, color, i, 0x1002000);
    }
}

void daEnBlockMain_c::player_set(int mode, int dir) {
    static const int l_player_mode[6] = { 0, 0, 1, 4, 5, 2 };

    mVec3_c pos(mPos);
    int flag = 0;
    if (dir == 0) {
        pos.y += 16.0f;
    } else {
        flag = 1;
        pos.y -= 16.0f;
    }

    daPyMng_c::fn_8005f4d0(&pos, l_player_mode[mode], flag);
}

void daEnBlockMain_c::continue_star_check(int *mode, s8 playerNo) {
    if (*mode == 8) {
        if (daPyMng_c::getPlayer(playerNo)->isStar()) {
            *mode = 7;
        } else {
            *mode = 1;
        }
    }
}

bool daEnBlockMain_c::player_bigmario_check(s8 playerNo) {
    daPlBase_c *player = daPyMng_c::getPlayer(playerNo);
    if (player != nullptr) {
        s8 powerup = player->getPowerup();
        // POWERUP_PROPELLER_SHROOM..POWERUP_ICE_FLOWER, or POWERUP_MUSHROOM..POWERUP_FIRE_FLOWER
        if ((u8) (powerup - POWERUP_PROPELLER_SHROOM) <= 2 || (u8) (powerup - POWERUP_MUSHROOM) <= 1) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------- 0x800221E0
void multi_item_mode_set(daEnBlockMain_c *self, u32 itemNo) {
    int v = 8;
    if (itemNo == 1 || itemNo == 0x1b || itemNo == 7) {
        v = itemNo;
    }
    self->mItemMode[0] = v;
    self->mItemMode[1] = v;
    self->mItemMode[2] = v;
    self->mItemMode[3] = v;
    if (itemNo == 1 || itemNo == 0x1b || itemNo == 7) {
        return;
    }

    int count = 0;
    int num = self->playernumber_set() + 1;
    if (num == 1) {
        self->mItemMode[0] = itemNo;
        return;
    }
    for (int i = 0; i < num; i++) {
        if (self->player_bigmario_check(i)) {
            count++;
        }
    }
    if (itemNo == 0) {
        itemNo = 9;
    }
    if (count == 0) {
        self->mItemMode[0] = itemNo;
        return;
    }
    for (int j = 0; j < count; j++) {
        self->mItemMode[j] = itemNo;
    }
}

bool daEnBlockMain_c::propeller_kinoko_check(int mode, s8 playerNo) {
    if (mode == 4) {
        return player_bigmario_check(playerNo);
    }
    return false;
}

int daEnBlockMain_c::playernumber_set() {
    int num = daPyMng_c::getNumInGame();
    if (num == 0) {
        num = 1;
    }
    return num - 1;
}

// ---------------------------------------------------------------- 0x800223C0
bool daEnBlockMain_c::YoshiEggCreateCheck(int mode) {
    dBlockMng_c *mng;
    bool ok = false;

    if (mode == 0) {
        if (!daPyMng_c::deleteCullingYoshi()) {
            ok = true;
        }
    } else {
        mng = dBlockMng_c::m_instance;
        if (mng->YoshiDispOutDelete() >= daPyMng_c::getNumInGame()) {
            ok = true;
        }
    }

    if (ok) {
        if (mode == 0) {
            eggitem_set(2);
        } else {
            multi_eggitem_set(2);
        }
        return false;
    }

    if (mode == 0) {
        yossy_set(2);
    } else {
        multi_yossy_set(2);
    }
    return true;
}

// ---------------------------------------------------------------- 0x80022490
void daEnBlockMain_c::item_sound_set(mVec3_c &pos, int mode, s8 playerNo, u8 a, u8 b) {
    if (mode == 2 || mode == 4) {
        if (playerNo != -1) {
            dAudio::SoundEffectID_t(SE_OBJ_GET_COIN)
                .playMapSound(pos, dAudio::getRemotePlayer(playerNo));
        }
    } else if (b != 0 && dActorMng_c::m_instance->envObakeCheck()) {
        dAudio::SoundEffectID_t(SE_OBJ_ITEM_APPEAR_HAUNT).playMapSound(pos, 0);
    } else {
        if (mode == 0x15) {
            if (a != 0) {
                dAudio::SoundEffectID_t(SE_OBJ_ITEM_PRPL_APPEAR).playMapSound(pos, 0);
                return;
            } else if (player_bigmario_check(playerNo)) {
                dAudio::SoundEffectID_t(SE_OBJ_ITEM_PRPL_APPEAR).playMapSound(pos, 0);
                return;
            }
        }
        dAudio::SoundEffectID_t(SE_OBJ_ITEM_APPEAR).playMapSound(pos, 0);
    }
}

/// @brief Whether item @p n spawns as a multiplayer set. @unofficial
/// @note 24 entries, not 18: the trailing zeros are real and reproduce the
/// 0x18 gap to l_item_no_tbl. Read both by isMultiItem and by the item cores.
static const u8 l_item_create_tbl[24] = {
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

/// @brief Block index -> item number. @unofficial
static const u32 l_item_no_tbl[18] = {
    0xf, 0x2, 0x0, 0x0, 0x15, 0x11, 0x19, 0x1, 0x1b,
    0xc, 0x4, 0x7, 0xa, 0xd, 0x8, 0xe, 0x5, 0x6,
};

/// @brief As @ref l_item_no_tbl, but for blocks with the alternate flag set.
/// @details Differs in entries [2] and [3] only. @unofficial
static const u32 l_item_no_tbl_ex[18] = {
    0xf, 0x2, 0x8, 0x9, 0x15, 0x11, 0x19, 0x1, 0x1b,
    0xc, 0x4, 0x7, 0xa, 0xd, 0x8, 0xe, 0x5, 0x6,
};


// ---------------------------------------------------------------- 0x80022600
void block_item_create(daEnBlockMain_c *self, sBlockItemInfo *info) {
    int index = info->mIndex;
    if (index == 4) {
        if (self->propeller_kinoko_check(index, info->mPlayerNo)) {
            int num = self->playernumber_set();
            if (num == 0) {
                block_item_set(self, info, 0);
            } else {
                block_multi_item_set(self, info, num, 0);
            }
        }
    } else if (index == 0xe) {
        block_item_set(self, info, 0);
    } else if (l_item_create_tbl[index] != 0) {
        block_item_set(self, info, 0);
    }
}

// ---------------------------------------------------------------- 0x800226D0
void block_item_create_sub(daEnBlockMain_c *self, sBlockItemInfo *info) {
    int index = info->mIndex;
    if (index == 4) {
        if (self->propeller_kinoko_check(index, info->mPlayerNo)) {
            return;
        }
    } else if (index == 0xe) {
        block_item_set(self, info, 1);
    }
    if (l_item_create_tbl[info->mIndex] == 0) {
        block_item_set(self, info, 1);
    }
}

// ---------------------------------------------------------------- 0x80022770
// Unnamed in the symbol map (fn_80022770), 0x10 bytes, never called inside this
// TU -- the derived block actors in the RELs call it.  `this` is unused, the
// item id arrives in r4, so it is a NON-STATIC MEMBER of daEnBlockMain_c and
// therefore NEEDS ONE LINE ADDING TO THE SHARED HEADER; see the report.
// The return type must be u8, NOT bool: bool costs three extra words
// (neg/or/srwi) normalising the byte to 0/1.
//   u8 isMultiItem(int itemNo); ///< @unofficial 0x80022770
u8 daEnBlockMain_c::isMultiItem(int itemNo) {
    return l_item_create_tbl[itemNo];
}

// ---------------------------------------------------------------- 0x80022780
void block_multi_item_create(daEnBlockMain_c *self, sBlockItemInfo *info, int num, u32 arg) {
    int index = info->mIndex;
    int ok = 0;
    if (index == 4) {
        if (!self->propeller_kinoko_check(index, info->mPlayerNo)) {
            ok = 1;
        }
    } else {
        ok = 1;
    }
    if (ok != 0) {
        block_multi_item_set(self, info, num, arg);
    }
}

// ---------------------------------------------------------------- 0x80022810
void block_item_set(daEnBlockMain_c *self, sBlockItemInfo *info, int alt) {
    if (self->m_648 != 0) {
        return;
    }

    int sub;
    u32 itemNo;
    u32 tableBit = 0;
    if (info->mAltTable == 0) {
        itemNo = l_item_no_tbl[info->mIndex];
    } else {
        tableBit = 0x1000;
        itemNo = l_item_no_tbl_ex[info->mIndex];
    }

    if (itemNo == 0xf || itemNo == 6) {
        return;
    }

    if (info->mIndex == 0xe) {
        if (info->mPlayerNo == -1) {
            return;
        }
        if (self->playernumber_set() == 0) {
            if (alt == 0) {
                if (self->player_bigmario_check(info->mPlayerNo)) {
                    itemNo = 2;
                } else {
                    return;
                }
            }
        } else if (alt == 0) {
            return;
        }
    }

    sub = info->m_15;

    if (itemNo == 0xc) {
        self->YoshiEggCreateCheck(0);
        self->m_648 = 1;
        return;
    }
    if (itemNo == 0xd) {
        self->jumpdai_set();
        self->m_648 = 1;
        return;
    }
    if (itemNo == 5) {
        mVec3_c coinPos = self->mPos;
        if (self->m_68d == 2) {
            coinPos.y += 16.0f;
            dActorMng_c::m_instance->createJumpCoin(coinPos, 5, 0);
            dAudio::SoundEffectID_t(0x238).playMapSound(info->mPos, 0);
        } else {
            coinPos.y = coinPos.y - 8.0f;
            dActorMng_c::m_instance->createBlockDownCoin(coinPos, 5, 0);
        }
        self->m_648 = 1;
        return;
    }
    if (itemNo == 0xa) {
        self->item_ivy_set(1, self->m_68d);
        self->m_648 = 1;
        return;
    }
    if (itemNo == 2 || itemNo == 4) {
        sub = 0;
    }

    mVec3_c pos(info->mPos);
    pos.z = 0.0f;
    if (self->m_690 == 2 || self->mLayer != 0) {
        pos.z = -3580.0f;
    }

    s8 playerNo = info->mPlayerNo;
    if (playerNo == -1) {
        return;
    }

    u32 param;
    if (sub == 0) {
        u8 extra = info->m_16;
        param = (self->m_68d << 18) | ((playerNo << 16) | (itemNo | tableBit | 0x80000));
        if (extra != 0) {
            param |= extra << 20;
        }
    } else {
        param = (playerNo << 16) | (itemNo | tableBit | 0x0a000000);
    }

    dActor_c *item = dActor_c::construct(fProfile::EN_ITEM, param | 0x800, &pos, NULL, self->mLayer);
    if (item != NULL && info->m_16 != 0) {
        *(fBaseID_e *)((u8 *)item + 0xbd0) = self->mUniqueID;
    }
    self->item_sound_set(pos, itemNo, playerNo, 0, 0);
    if (itemNo != 4) {
        self->m_648 = 1;
    }
}

// ---------------------------------------------------------------- 0x80022B10
void block_multi_item_set(daEnBlockMain_c *self, sBlockItemInfo *info, int num, u32 arg) {
    if (self->m_648 != 0) {
        return;
    }

    u32 itemNo;
    u32 tableBit = 0;
    if (info->mAltTable == 0) {
        itemNo = l_item_no_tbl[info->mIndex];
    } else {
        tableBit = 0x1000;
        itemNo = l_item_no_tbl_ex[info->mIndex];
    }

    if (itemNo == 0xf || itemNo == 6) {
        return;
    }
    if (itemNo == 0xc) {
        self->YoshiEggCreateCheck(1);
        self->m_648 = 1;
        return;
    }
    if (itemNo == 0xd) {
        self->jumpdai_set();
        self->m_648 = 1;
        return;
    }
    if (itemNo == 5) {
        mVec3_c coinPos = self->mPos;
        if (self->m_68d == 2) {
            coinPos.y += 16.0f;
            dActorMng_c::m_instance->createJumpCoin(coinPos, 5, 0);
            dAudio::SoundEffectID_t(0x238).playMapSound(info->mPos, 0);
        } else {
            coinPos.y = coinPos.y - 8.0f;
            dActorMng_c::m_instance->createBlockDownCoin(coinPos, 5, 0);
        }
        self->m_648 = 1;
        return;
    }

    mVec3_c pos(info->mPos);
    pos.z = 0.0f;
    if (self->m_690 == 2 || self->mLayer != 0) {
        pos.z = -3580.0f;
    }

    s8 playerNo = info->mPlayerNo;
    if (playerNo == -1) {
        return;
    }

    multi_item_mode_set(self, itemNo);

    u32 base;
    if (self->m_68d == 2) {
        base = num != 0 ? 0x0b000000 : 0x0d000000;
        pos.y += 16.0f;
    } else {
        base = num != 0 ? 0x0c000000 : 0x0e000000;
        pos.y = pos.y - 8.0f;
    }

    int count = num + 1;
    dEnemyMng_c::m_instance->multi_item_set(&pos, (unsigned long *)self->mItemMode, base | tableBit,
                                            count, arg, playerNo, self->mLayer);
    if (count - 1 != 0) {
        for (int i = 0; i < count; i++) {
            self->item_sound_set(pos, self->mItemMode[i], playerNo, 1, 1);
        }
    } else {
        self->item_sound_set(pos, self->mItemMode[0], playerNo, 0, 1);
    }
    self->m_648 = 1;
}

void daEnBlockMain_c::block_scale_set(u8 mode) {
    int grow = 0;
    if (mode == 0) {
        if (mSpeed.y > 0.0f) {
            grow = 1;
        }
    } else {
        if (mSpeed.y < 0.0f) {
            grow = 1;
        }
    }

    if (grow) {
        mScale.x += mScaleStep;
        if (mScale.x >= mScaleMax) {
            mScale.x = mScaleMax;
        }
    } else {
        m_67f = 0;
        mScale.x -= mScaleStep;
        if (mScale.x <= mScaleMin) {
            mScale.x = mScaleMin;
        }
    }

    mScale.y = mScale.x;
}

void daEnBlockMain_c::initializeState_UpMove() {
    mSpeed.y = 2.0f;
    mMoveYAccel = 0.281f;
    initialize_upmove();
}

void daEnBlockMain_c::initialize_upmove() {}

void daEnBlockMain_c::finalizeState_UpMove() {}

void daEnBlockMain_c::executeState_UpMove() {
    mSpeed.y -= mMoveYAccel;
    if (mSpeed.y <= -2.0f) {
        mSpeed.y = 2.0f;
    }

    block_scale_set(0);
    dBaseActor_c::posMove();
    HopCoinBgcheck();
    block_upmove();
}

void daEnBlockMain_c::block_upmove() {}

void daEnBlockMain_c::initializeState_DownMove() {
    mSpeed.y = -2.0f;
    mMoveYAccel = 0.281f;
    initialize_downmove();
}

void daEnBlockMain_c::initialize_downmove() {}

void daEnBlockMain_c::finalizeState_DownMove() {}

void daEnBlockMain_c::executeState_DownMove() {
    mSpeed.y += mMoveYAccel;
    if (mSpeed.y >= 2.0f) {
        mSpeed.y = 2.0f;
    }

    block_scale_set(1);
    dBaseActor_c::posMove();
    HopCoinBgcheck();
    block_downmove();
}

void daEnBlockMain_c::block_downmove() {}

void daEnBlockMain_c::initializeState_DownMoveEnd() {
    mEndTimer = 6;
}

void daEnBlockMain_c::finalizeState_DownMoveEnd() {}

void daEnBlockMain_c::executeState_DownMoveEnd() {
    if (mEndTimer != 0) {
        mEndTimer--;
    }

    if (mEndTimer == 0) {
        block_downmove_end();
    }
}

void daEnBlockMain_c::block_downmove_end() {}

void daEnBlockMain_c::initializeState_UpMove_Diff() {
    mMoveDiff.x = 0.0f;
    mMoveDiff.y = 0.0f;
    mMoveDiff.z = 0.0f;
    mSpeed.y = 2.0f;
    initialize_upmove();
}

void daEnBlockMain_c::finalizeState_UpMove_Diff() {}

void daEnBlockMain_c::executeState_UpMove_Diff() {
    mSpeed.y -= 0.281f;
    if (mSpeed.y <= -2.0f) {
        mSpeed.y = 2.0f;
    }

    block_scale_set(0);

    mMoveDiff += mSpeed;

    HopCoinBgcheck();

    if (mMoveDiff.y <= 0.0f) {
        mMoveDiff.y = 0.0f;
        block_upmove_diff();
    }
}

void daEnBlockMain_c::block_upmove_diff() {}

void daEnBlockMain_c::initializeState_DownMove_Diff() {
    mMoveDiff.x = 0.0f;
    mMoveDiff.y = 0.0f;
    mMoveDiff.z = 0.0f;
    mSpeed.y = -2.0f;
    initialize_downmove();
}

void daEnBlockMain_c::finalizeState_DownMove_Diff() {}

void daEnBlockMain_c::executeState_DownMove_Diff() {
    mSpeed.y += 0.281f;
    if (mSpeed.y >= 2.0f) {
        mSpeed.y = 2.0f;
    }

    block_scale_set(1);

    mMoveDiff += mSpeed;

    HopCoinBgcheck();

    if (mMoveDiff.y >= 0.0f) {
        mMoveDiff.y = 0.0f;
        block_downmove_diff();
    }
}

void daEnBlockMain_c::block_downmove_diff() {}

void daEnBlockMain_c::initializeState_DownMove_DiffEnd() {
    mEndTimer = 6;
}

void daEnBlockMain_c::finalizeState_DownMove_DiffEnd() {}

void daEnBlockMain_c::executeState_DownMove_DiffEnd() {
    if (mEndTimer != 0) {
        mEndTimer--;
    }

    if (mEndTimer == 0) {
        block_downmove_diffend();
    }
}

void daEnBlockMain_c::block_downmove_diffend() {}

/// @note Defined out of line and last in the file. Nothing follows it in
/// .text before __sinit. @unofficial
daEnBlockMain_c::~daEnBlockMain_c() {}

