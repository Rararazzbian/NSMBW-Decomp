#include <game/bases/d_a_player_hio.hpp>
#include <game/bases/d_player_model_manager.hpp>

int dAcPy_HIO_Speed_c::ms_num_of_instance;
dAcPy_HIO_Speed_c dAcPy_HIO_Speed_c::sc_playerSpeedDt[2];

dAcPy_HIO_Speed_c::dAcPy_HIO_Speed_c() {
    if (ms_num_of_instance < 2) {
        init(ms_num_of_instance);
    }
    ms_num_of_instance++;
}

dAcPy_HIO_Speed_c::~dAcPy_HIO_Speed_c() {
    ms_num_of_instance--;
}

/// @note NOT byte-matching. Semantically almost certainly correct -- copies
/// the default per-player speed record into *this -- but the compiler picks a
/// different codegen strategy than the target: this compiles to two runtime
/// block-copy loops (26 instructions); the target is a single, fully unrolled
/// 157-instruction sequence with no loop, using FPU loads for the six directly
/// declared floats and raw GPR word loads for everything nested inside the
/// sPowerChangeSpeedData sub-structs, plus a _savegpr_14/_restgpr_14 frame
/// implying much higher register pressure than any tried variant produced. Six
/// structurally different formulations were tried (whole-object assign,
/// sub-object assign, array-member assign, an isolated threshold test, an
/// all-60-fields-flat assignment, and memcpy); none reproduced the target's
/// shape.
void dAcPy_HIO_Speed_c::init(int idx) {
    *this = sc_playerSpeedDt[idx];
}

/// @note dPyMdlBase_HIO_c's static per-animation default table. Its owning
/// class, dPyMdlBase_c, is not yet decompiled (include/game/bases/
/// d_player_model_base.hpp has none of its statics declared), so this is
/// referenced by its exact linker symbol name as a free extern rather than as
/// a class member. Field offsets 0x14/0x18 = rate/blendDuration, confirmed by
/// reading several entries out of .rodata.
struct dPyAnmDefault_s {
    u8 unk_00[0x14];
    float rate;          ///< 0x14
    float blendDuration; ///< 0x18
    u8 unk_1c[0x24 - 0x1c];
};
extern dPyAnmDefault_s scPyAnmData__12dPyMdlBase_c[];

dPyAnm_HIO_c::dPyAnm_HIO_c() {
    mID = 0;
    mRate = 0.0f;
    mBlendDuration = 0.0f;
}

/// @note NOT byte-matching. Logic and every load/store offset are byte-correct;
/// the only residual is which of two independent FPRs (f0/f1) each temporary
/// lands in. Declaration order, statement order, const-qualification and
/// pointer-vs-reference access were all tried; none flip it.
void dPyAnm_HIO_c::resetParam(int id) {
    mID = id;
    const dPyAnmDefault_s *p = &scPyAnmData__12dPyMdlBase_c[id];
    float rate = p->rate;
    float blendDuration = p->blendDuration;
    mRate = rate;
    mBlendDuration = blendDuration;
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
/// statics local to this TU (no class suffix in the symbol map), sitting
/// immediately after dAcPy_HIO_Speed_c::sc_playerSpeedDt in .rodata.
extern const dPyModelData_s scStoopOffset[3];
extern const dPyModelData_s scYoshiOffset[3][2];
extern const dPyModelData_s scCloudOffset[3];

/// @note NOT byte-matching. Field decomposition, statement order and every
/// stfs destination offset are byte-correct; the residual is a genuine
/// -ipa file whole-TU optimization: the target computes &scStoopOffset,
/// &scYoshiOffset and &scCloudOffset as sc_playerSpeedDt's own @ha/@l pair plus
/// a compile-time constant (+0x2b8/+0x2e8/+0x348), because those objects
/// happen to sit contiguously after it in .rodata, instead of three separate
/// relocations (target 53 instructions, this compiles to 55: three separate
/// lis/addi pairs instead of one lis/addi plus three addi-immediate deltas).
/// Reproducing the address sharing requires the real (non-zero) data for every
/// preceding table in the same TU -- confirmed by trying dummy zero data and
/// observing MWCC's optimizer merge the identical-content zero blobs instead,
/// which is not what the target does.
void dPyModel_HIO_c::resetParam(int id) {
    *(int *) &mData[0] = id; // stored as a raw int, not float-converted
    const dPyModelData_s &stoop = scStoopOffset[id];
    mData[1] = stoop.a;
    mData[2] = stoop.b;
    mData[3] = stoop.c;
    mData[4] = stoop.d;
    modelData[0] = scYoshiOffset[id][0];
    modelData[1] = scYoshiOffset[id][1];
    modelData[2] = scCloudOffset[id];
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
