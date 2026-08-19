#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_a_wm_item.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_actor.hpp>
#include <game/mLib/m_heap.hpp>
#include <cstring>

// @unofficial The six per-item-type name tables and the fixed strings that
// follow g_profile_WM_ITEM in .data are measured directly from the retail
// REL (original/d_basesNP.rel, .data 0x45050-0x451b8) plus its relocation
// stream (module-2 self-relocations resolved with tools/relfile.py) -- see
// MAPPING.md for the full derivation. Declared in an anonymous namespace,
// in the exact order the target's .data lays them out, since MWCC preserves
// declaration order for static objects within one TU.
ACTOR_PROFILE(WM_ITEM, daWmItem_c, 0);

// @unofficial NEGATIVE, kept for the record: createModel()'s target
// addresses every one of the per-item-type tables below as
// `g_profile_WM_ITEM + literal offset` through ONE dedicated register
// (r30), not through each table's own symbol -- proving the real source
// reaches them via pointer arithmetic from the profile object itself, not
// via separately-named globals like sItemArcNames[] below. Rewriting the
// accesses through WM_ITEM_TABLE/WM_ITEM_STR macros below DID make the
// compiler anchor r30 = &g_profile_WM_ITEM (measured: same `lis
// r29,g_profile_WM_ITEM@ha` shape target uses), confirming the addressing
// axis -- but it established that anchor LAZILY, at the first use partway
// through the function, where the target hoists BOTH its persistent
// anchors (profile AND lbl_2_rodata_8988) to the very top, ahead of
// `createFrmHeap`, and needs 5 saved registers (`_savegpr_27`) where the
// macro'd draft only needed 4. Net effect measured: 116 -> 165 differing,
// i.e. worse, so this unit's createModel() keeps the named-table form
// below. The macros are kept unused rather than deleted, since they may
// be the right fix once the eager-hoisting trigger is understood.
#define WM_ITEM_TABLE(off) (*(const char *const (*)[7])((const u8 *)&g_profile_WM_ITEM + (off)))
#define WM_ITEM_STR(off) ((const char *)((const u8 *)&g_profile_WM_ITEM + (off)))

namespace {
    // @unofficial lbl_2_rodata_8988 (6 floats) and lbl_2_rodata_89A0 (6 more)
    // read directly from original/d_basesNP.rel .rodata (file offset
    // 0x1c6600 + address). dtk splits these into two separate symbols, but
    // the target's own code addresses everything through ONE base register
    // with offsets past the first object's declared size (e.g. `lfs f0,
    // 0x1c(r31)` where r31 = &lbl_2_rodata_8988 and the object is only
    // 0x18 bytes) -- i.e. the real source has a single 12-float table and
    // dtk's split is an artifact. Declared as one array so the compiler
    // pools every reference off the same base, matching the target.
    static const float sConstTable[12] = {
        -32.0f, 200.0f, 3.2f, 0.65f, 0.0f, 1000.0f,
        1.0f, 0.65f, 2.0f, 3.2f, -32.0f, 200.0f,
    };

    // @unofficial lbl_2_data_4503C, patched at load by the guarded __sinit
    // block below from sConstTable[0..3]. The trailing word (offset 0x10,
    // size 0x14 total) is never referenced by any of this unit's 12
    // functions; kept as raw bytes.
    struct SinitTable_t {
        float a, b, c, d;
        u32 unused;
    };
    SinitTable_t sSinitTable = { 0.0f, 0.0f, 0.0f, 0.0f, 0x01010000 };

    struct SinitTrigger_t {
        s8 mDone;
        u8 pad_unofficial[7];

        SinitTrigger_t() {
            if (!mDone) {
                sSinitTable.a = sConstTable[0];
                sSinitTable.b = sConstTable[1];
                sSinitTable.c = sConstTable[2];
                sSinitTable.d = sConstTable[3];
                mDone = true;
            }
        }
    };
    static SinitTrigger_t sSinitTrigger;

    // Item-name strings, "I_kinoko" family -- table1 (profile+0x68) source.
    static const char s_I_kinoko[] = "I_kinoko";
    static const char s_I_fireflower[] = "I_fireflower";
    static const char s_I_propeller[] = "I_propeller";
    static const char s_I_iceflower[] = "I_iceflower";
    static const char s_I_penguin[] = "I_penguin";
    static const char s_I_star[] = "I_star";

    static const char *const sItemArcNames[7] = {
        s_I_kinoko, s_I_fireflower, s_I_propeller, s_I_iceflower,
        s_I_penguin, s_I_kinoko, s_I_star,
    };

    static const char s_I_propeller_model[] = "I_propeller_model";

    static const char *const sItemModelNames[7] = {
        s_I_kinoko, s_I_fireflower, s_I_propeller_model, s_I_iceflower,
        s_I_penguin, s_I_kinoko, s_I_star,
    };

    // SI_-prefixed animation-archive names -- table3 (profile+0x108).
    static const char s_SI_kinoko[] = "SI_kinoko";
    static const char s_SI_fireflower[] = "SI_fireflower";
    static const char s_SI_propeller[] = "SI_propeller";
    static const char s_SI_iceflower[] = "SI_iceflower";
    static const char s_SI_penguin[] = "SI_penguin";
    static const char s_SI_star[] = "SI_star";

    static const char *const sItemAnimArcNames[7] = {
        s_SI_kinoko, s_SI_fireflower, s_SI_propeller, s_SI_iceflower,
        s_SI_penguin, s_SI_kinoko, s_SI_star,
    };

    static const char s_g3dSlash[] = "g3d/";
    static const char s_dotBrres[] = ".brres";
    static const char s_g3dModelBrres[] = "g3d/model.brres";
    static const char s_stokWait_a[] = "stok_wait";
    static const char s_I_kinoko_switch[] = "I_kinoko_switch";

    static const char s_stokWait_b[] = "stok_wait";
    static const char s_stokWait2[] = "stok_wait2";
    static const char *const sCycleAnmNames[2] = { s_stokWait_b, s_stokWait2 };
}


daWmItem_c::daWmItem_c() {
    m_208 = 0;
    mItemType = mParam & 0xf;
}

daWmItem_c::~daWmItem_c() {}

int daWmItem_c::create() {
    // @unofficial measured: f0=sConstTable[6](1.0), f2=sConstTable[4](0.0),
    // f1=sConstTable[5](1000.0) -- mPos=(0,1000,0), mMotion=(1,1,1).
    mPos.x = sConstTable[4];
    mPos.y = sConstTable[5];
    mPos.z = sConstTable[4];
    mMotion[0] = sConstTable[6];
    mMotion[1] = sConstTable[6];
    mMotion[2] = sConstTable[6];

    createModel();

    if (mItemType == 5) {
        mScale.x = sConstTable[7];
        mScale.y = sConstTable[7];
        mScale.z = sConstTable[7];
    }

    m_208 = 1;
    m_209 = 0;
    m_20a = 0;
    m_20b = 0;
    m_200 = 2;
    m_204 = 2;
    return SUCCEEDED;
}

bool daWmItem_c::step0() {
    if (m_208 && m_209) {
        cycleAnm();
        mModel.play();
        m_20a = 1;
    }
    return true;
}

void daWmItem_c::step1() {
    if (m_208 && m_20a && !m_20b) {
        m_20a = 0;
        calcModel();
        mModel.entry();
    }
}

void daWmItem_c::step2() {
    if (m_208 && m_20a && m_20b) {
        m_20a = 0;
        calcModel();
        mModel.entry();
    }
}

int daWmItem_c::doDelete() {
    return 1;
}

void daWmItem_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char archiveName[0x64];
    memset(archiveName, 0, 0x64);
    strncat(archiveName, s_g3dSlash, 0x63);
    strncat(archiveName, sItemArcNames[mItemType], 0x63);
    strncat(archiveName, s_dotBrres, 0x63);

    mResFile = dResMng_c::m_instance->getRes(sItemArcNames[mItemType], archiveName);

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(sItemModelNames[mItemType]);

    if (mItemType == 0 || mItemType == 5) {
        mModel.create(resMdl, &mAllocator, 0x23, 1, nullptr);
    } else {
        mModel.create(resMdl, &mAllocator, 0x20, 1, nullptr);
    }

    mAnimResFile = dResMng_c::m_instance->getRes(sItemAnimArcNames[mItemType], s_g3dModelBrres);

    nw4r::g3d::ResAnmChr resAnmChr = mAnimResFile.GetResAnmChr(s_stokWait_a);
    mAnmChr.create(resMdl, resAnmChr, &mAllocator, nullptr);
    mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, sConstTable[6]);

    if (mItemType == 0 || mItemType == 5) {
        mResAnmTexPat = mResFile.GetResAnmTexPat(s_I_kinoko_switch);
        mAnmTexPat.create(resMdl, mResAnmTexPat, &mAllocator, nullptr, 1);
        float rate = (mItemType == 5) ? sConstTable[8] : sConstTable[4];
        mAnmTexPat.setAnm(mModel, mResAnmTexPat, 0, m3d::FORWARD_ONCE);
        mModel.setAnm(mAnmTexPat);
        mAnmTexPat.setFrame(rate, 0);
        mAnmTexPat.setRate(sConstTable[4], 0);
    }

    dActor_c::setSoftLight_Item(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmItem_c::cycleAnm() {
    if (m_200 != m_204) {
        m_200 = m_204;
        nw4r::g3d::ResAnmChr resAnmChr = mAnimResFile.GetResAnmChr(sCycleAnmNames[m_204]);
        mAnmChr.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
        mModel.setAnm(mAnmChr, sConstTable[6]);
    }
}

void daWmItem_c::calcModel() {
    const float *k = sConstTable;

    mScale.x = k[9];
    mScale.y = k[9];
    mScale.z = k[9];

    if (mItemType == 5) {
        float v = k[9] * k[7];
        mScale.x = v;
        mScale.y = v;
        mScale.z = v;
    }

    mScale.x *= mMotion[0];
    mScale.y *= mMotion[1];
    mScale.z *= mMotion[2];
    mModel.setScale(mScale);

    mVec3_c local;
    local.x = mPos.x;
    local.y = mPos.y + k[10] * mMotion[1];
    local.z = mPos.z;

    if (m_20b) {
        local.z = k[11];
    }

    mMatrix.trans(local);
    mModel.setLocalMtx(&mMatrix);
    mModel.calc(false);
}
