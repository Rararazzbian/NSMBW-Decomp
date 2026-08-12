#include <game/bases/d_a_player_hio.hpp>
#include <game/bases/d_player_model_manager.hpp>

/// @note The order the classes are defined in below is fixed by the binary and
/// is not the order you would naturally write them: dAcPy_HIO_Speed_c, then
/// dPyMdlBase_HIO_c, then dPyAnm_HIO_c, dPyAnmMain_HIO_c, dPyModel_HIO_c and
/// dYoshiModel_HIO_c. Two independent things pin it. .text runs in source
/// order, and .sdata2 pools literals per function in source order -- putting
/// dPyMdlBase_HIO_c's constructor first is what emits -1.0f, 1.0f and 0.6f
/// ahead of dPyAnm_HIO_c's 0.0f and dYoshiModel_HIO_c::resetParam's four, which
/// is the order the shipped .sdata2 slice has them in.
///
/// The .rodata definitions likewise have to appear in address order --
/// the four sc_player_* records, then scUnkOffset, then the three
/// scStoopOffset/scYoshiOffset/scCloudOffset tables -- because MWCC emits them
/// in definition order and dPyModel_HIO_c::resetParam addresses the last three
/// by their compile-time distance from the first.

int dAcPy_HIO_Speed_c::ms_num_of_instance;

const sSpeedData dAcPy_HIO_Speed_c::sc_player_mame = {
    1.5f, 2.25f, 3.0f,
    { 0.035f, 0.05f, 0.07f, 0.15f, 0.07f, 0.07f, 0.043f, 0.035f, 0.035f },
    { 0.0131250005f, 0.01875f, 0.026250001f, 0.056250002f, 0.0175f, 0.0175f, 0.01075f, 0.00875f, 0.00875f },
    { 0.035f, 0.05f, 0.07f, 0.15f, 0.07f, 0.07f, 0.043f, 0.035f, 0.035f },
};

const sSpeedData dAcPy_HIO_Speed_c::sc_player_mame_star = {
    1.5f, 2.25f, 4.0f,
    { 0.035f, 0.05f, 0.07f, 0.15f, 0.07f, 0.07f, 0.043f, 0.035f, 0.035f },
    { 0.0131250005f, 0.01875f, 0.026250001f, 0.056250002f, 0.0175f, 0.0175f, 0.01075f, 0.00875f, 0.00875f },
    { 0.035f, 0.05f, 0.07f, 0.15f, 0.07f, 0.07f, 0.043f, 0.035f, 0.035f },
};

const sSpeedData dAcPy_HIO_Speed_c::sc_player_normal = {
    1.5f, 2.25f, 3.0f,
    { 0.035f, 0.05f, 0.07f, 0.1f, 0.1f, 0.03f, 0.06f, 0.029f, 0.021f },
    { 0.013f, 0.015f, 0.018f, 0.021f, 0.02f, 0.03f, 0.06f, 0.028f, 0.02f },
    { 0.025f, 0.03f, 0.06f, 0.08f, 0.1f, 0.03f, 0.06f, 0.029f, 0.021f },
};

const sSpeedData dAcPy_HIO_Speed_c::sc_player_normal_star = {
    2.0f, 3.25f, 4.0f,
    { 0.035f, 0.05f, 0.07f, 0.13f, 0.13f, 0.04f, 0.078f, 0.039f, 0.027f },
    { 0.013f, 0.015f, 0.0245f, 0.027f, 0.06f, 0.04f, 0.078f, 0.038f, 0.026f },
    { 0.025f, 0.03f, 0.07f, 0.13f, 0.13f, 0.04f, 0.078f, 0.039f, 0.027f },
};

/// @unofficial
/// @note Unidentified: 0xD8 of const floats sitting between the four speed
/// records and scStoopOffset (lbl_802EF2E0 in the symbol map), referenced by
/// nothing in the DOL. Its existence and exact size are not guesses -- they are
/// forced by dPyModel_HIO_c::resetParam, which addresses scStoopOffset as
/// sc_player_mame + 0x2b8 where the four records only account for 0x1e0. The
/// content splits cleanly into three 0x48 blocks, matching the three player
/// model types the neighbouring tables are indexed by, but what the 18 floats
/// in a block mean is unknown, hence the placeholder name.
/// @note The `extern` is load-bearing, not decoration: with the file-scope
/// internal linkage that plain `const` gives it, MWCC deadstrips an unreferenced
/// table and scStoopOffset moves 0xD8 too low. The original's own spelling is
/// unrecoverable -- anything with external linkage and this layout works.
extern const float scUnkOffset[3][18];
const float scUnkOffset[3][18] = {
    { -0.34f, 2.5f, 1.5f, 0.3f, -0.12f, -3.0f,
      -0.34f, -0.34f, -0.34f, -0.25f, -0.34f, -0.34f,
      -0.06f, -0.25f, -0.34f, -0.08f, -0.31f, -0.34f },
    { -0.09f, 2.5f, 1.5f, 0.0f, -2.0f, -2.0f,
      -0.09f, -0.09f, -0.09f, -0.06f, -0.09f, -0.09f,
      -0.04f, -0.06f, -0.09f, -0.06f, -0.09f, -0.09f },
    { -0.34f, 2.5f, 1.5f, 0.3f, -0.12f, -3.0f,
      -0.34f, -0.34f, -0.34f, -0.25f, -0.34f, -0.34f,
      -0.06f, -0.25f, -0.34f, -0.1f, -0.1f, -0.1f },
};

const sSpeedData dAcPy_HIO_Speed_c::sc_playerSpeedDt[2][2] = {
    { dAcPy_HIO_Speed_c::sc_player_mame, dAcPy_HIO_Speed_c::sc_player_mame_star },
    { dAcPy_HIO_Speed_c::sc_player_normal, dAcPy_HIO_Speed_c::sc_player_normal_star },
};

dAcPy_HIO_Speed_c::dAcPy_HIO_Speed_c() {
    if (ms_num_of_instance < 2) {
        init(ms_num_of_instance);
    }
    ms_num_of_instance++;
}

dAcPy_HIO_Speed_c::~dAcPy_HIO_Speed_c() {
    ms_num_of_instance--;
}

/// @note The `const` on sc_playerSpeedDt is what makes this match, and it is
/// worth stating why. MWCC copies a struct field by field; when it can see that
/// the source cannot change under it -- a const object, or a source reached
/// through a pointer it must assume aliases the destination -- it hoists every
/// load ahead of every store, which here means 30 live values, r14-r31 via
/// _savegpr_14, and ten spill slots. Drop the const and the identical source
/// emits the same 60 loads and stores interleaved one pair at a time, in 127
/// instructions with no saved registers. Nothing about the statements below
/// changes; only the qualifier on the table does.
void dAcPy_HIO_Speed_c::init(int idx) {
    if (idx < 2) {
        mDataNormal = sc_playerSpeedDt[idx][0];
        mDataStar = sc_playerSpeedDt[idx][1];
    }
}

dPyMdlBase_HIO_c::dPyMdlBase_HIO_c() {
    mPad[0] = 1;
    mPad[1] = 1;
    m_04 = -1.0f;
    m_08[0] = 1.0f;
    m_08[1] = 1.0f;
    m_08[2] = 1.0f;
    m_08[3] = 0.6f;
    m_08[4] = 1.0f;
    m_08[5] = 1.0f;
    m_08[6] = 1.0f;
    m_24 = 0xff;

    mPyAnm.resetParam();
    for (int i = 0; i < 3; i++) {
        mPyModel[i].resetParam(i);
    }
    mYoshiModel.resetParam();
}

dPyMdlBase_HIO_c::~dPyMdlBase_HIO_c() {}

/// @note Written as a plain if-statement, not a ternary -- the ternary form
/// does not reproduce the target's instruction shape.
u8 dPyMdlBase_HIO_c::changeHioType(u8 hioType) {
    if (hioType >= 3) {
        hioType = 2;
    }
    return hioType;
}

/// @note Written as a switch, not an if/else-if chain -- the chain form does
/// not reproduce the target's instruction shape.
float dPyMdlBase_HIO_c::getValue(dPyModelData_s model, u8 powerup) {
    switch (powerup) {
        case 3: return model.c;
        case 0: return model.b;
        case 5: return model.d;
        default: return model.a;
    }
}

/// @note dPyMdlBase_HIO_c's static per-animation default table. Its owning
/// class, dPyMdlBase_c, is not yet decompiled (include/game/bases/
/// d_player_model_base.hpp has none of its statics declared), so this is
/// referenced by its exact linker symbol name as a free extern rather than as
/// a class member. Field offsets 0x14/0x18 = rate/blendDuration, confirmed by
/// reading several entries out of .rodata.
/// @note `const` matters here as it does for sc_playerSpeedDt: it is what makes
/// resetParam below batch its two loads ahead of its two stores instead of
/// interleaving them.
struct dPyAnmDefault_s {
    u8 unk_00[0x14];
    float rate;          ///< 0x14
    float blendDuration; ///< 0x18
    u8 unk_1c[0x24 - 0x1c];
};
extern const dPyAnmDefault_s scPyAnmData__12dPyMdlBase_c[];

dPyAnm_HIO_c::dPyAnm_HIO_c() {
    mID = 0;
    mRate = 0.0f;
    mBlendDuration = 0.0f;
}

void dPyAnm_HIO_c::resetParam(int id) {
    mID = id;
    mRate = scPyAnmData__12dPyMdlBase_c[id].rate;
    mBlendDuration = scPyAnmData__12dPyMdlBase_c[id].blendDuration;
}

dPyAnmMain_HIO_c::dPyAnmMain_HIO_c() {
    resetParam();
}

void dPyAnmMain_HIO_c::resetParam() {
    for (int i = 0; i < 177; i++) {
        mAnm[i].resetParam(i);
    }
}

dPyModel_HIO_c::dPyModel_HIO_c() {
    resetParam(0);
}

/// @note dPyModel_HIO_c::resetParam's three lookup tables. Plain file-scope
/// statics local to this TU (no class suffix in the symbol map), sitting after
/// the four sc_player_* records and scUnkOffset in .rodata -- resetParam reaches
/// all three as sc_player_mame's @ha/@l pair plus 0x2b8/0x2e8/0x348, so their
/// order and every preceding byte are fixed.
static const dPyModelData_s scStoopOffset[3] = {
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.8f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f },
};

static const dPyModelData_s scYoshiOffset[3][2] = {
    { { -9.2f, -2.5f, 1.0f, -5.5f }, { -6.5f, -7.0f, -6.5f, -6.0f } },
    { { -9.7f, -3.0f, 1.5f, -9.7f }, { -6.0f, -7.0f, -6.5f, -6.0f } },
    { { -2.0f, -0.5f, 3.0f, -2.0f }, { -6.0f, -7.0f, -7.0f, -6.0f } },
};

static const dPyModelData_s scCloudOffset[3] = {
    { 3.0f, 9.0f, 11.0f, 4.0f },
    { 2.5f, 9.5f, 11.0f, 3.5f },
    { 8.5f, 12.0f, 12.5f, 8.0f },
};

/// @note NOT byte-matching, but down to a two-register permutation: 46 of the
/// 53 instruction words are byte-identical and the remaining 7 differ only in
/// that the target holds the shared .rodata base in r8 and id*16 in r7 where
/// this holds them the other way round. Everything else -- the shared @ha/@l
/// base with its +0x2b8/+0x2e8/+0x348 deltas, the lfsx/lfsux/add address forms,
/// the f30/f31/f13..f0 assignment and all 16 store offsets -- matches exactly.
/// @note What is already forced, so nobody re-derives it: the three lookup
/// tables must be read in the order stoop, cloud, yoshi (that is what puts
/// cloud in f11-f8 and yoshi in f7-f0, since MWCC colours FPRs in reverse of
/// value order); cloud must be bound to a named reference ahead of the stoop
/// reads (that is what gives it the explicit `add` and a plain `lfs 0x0`
/// instead of an `lfsx`); and yoshi must go through a pointer (that is what
/// turns its base computation into the single `lfsux`, worth the 53rd
/// instruction -- inline indexing costs 54).
/// @note What did not move it: ~400 compiled variants. Every combination of
/// inline / reference / pointer access for each of the three tables, crossed
/// with every legal placement of each declaration and of the id store; pointer
/// arithmetic instead of indexing; a flat scYoshiOffset[6] with an id*2 index;
/// a struct copy into mData[1]; external linkage on all three tables; the
/// destinations reached through a local pointer. All 53-instruction forms land
/// on r7, all r8 forms cost 54. The two properties have not been obtained
/// together, which suggests the register falls out of a virtual-register count
/// this source shape does not reproduce rather than out of statement order.
void dPyModel_HIO_c::resetParam(int id) {
    *(int *) &mData[0] = id; // stored as a raw int, not float-converted
    const dPyModelData_s &cloud = scCloudOffset[id];
    mData[1] = scStoopOffset[id].a;
    mData[2] = scStoopOffset[id].b;
    mData[3] = scStoopOffset[id].c;
    mData[4] = scStoopOffset[id].d;
    modelData[2] = cloud;
    const dPyModelData_s *yoshi = scYoshiOffset[id];
    modelData[0] = yoshi[0];
    modelData[1] = yoshi[1];
}

dYoshiModel_HIO_c::dYoshiModel_HIO_c() {
    resetParam();
}

void dYoshiModel_HIO_c::resetParam() {
    mData[0] = 2.0f;
    mData[1] = 1.4f;
    mData[2] = 10.0f;
    mData[3] = 4.0f;
}
