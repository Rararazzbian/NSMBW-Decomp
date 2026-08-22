// d_bg_actor_mng.cpp -- SECOND DRAFT (declarations only)
//
// Round 17 unit: wiimj2d.dol .text 0x8007E180..0x8007F7A0
//
// KEY BREAKTHROUGH: the target's __sinit_ registers each array via a
// TAIL-CALL `b __register_global_object` (not `bl`), and the trigger is a
// BgObjName_t that is an AGGREGATE (no ctor -- only a user-provided dtor) with
// aggregate-style `{ ... }` initializers whose mVec3_c/mVec2_c members are
// ctor-calls (non-foldable) so the floats zero in .data and are written at
// runtime by __sinit_ from sdata2 constants -- the target's exact pattern.
// probe_agg_mvec2.cpp proved this emits:
//   - __dt__Q217dBgActorManager_c11BgObjName_tFv (0x40 null/flag-check dtor)
//   - __sinit_ that writes floats via stfs and ends `b __register_global_object`
//   - __arraydtor$ per array (lis array, lis dtor, li 0x20, li count, b __destroy_arr)
//   - .ctors entry + one 0xC .bss register node per array

#include <types.h>
#include <string.h>
#include <game/bases/d_bg_actor_mng.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_base_actor.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_allocator_dummy_heap.hpp>
#include <lib/egg/core/eggHeap.h>
#include <game/bases/d_bg.hpp>

// dtor must be out-of-line (target __dt__Q217dBgActorManager_c11BgObjName_tFv
// at 0x8007F700 is 0x40 bytes: null-check + flag-check + __dl__).
dBgActorManager_c::BgObjName_t::~BgObjName_t() {}

dBgActorManager_c *dBgActorManager_c::ms_instance;

static dBgActorManager_c::BgObjName_t *l_pRailList; ///< .sbss:0x8042A0BC

dBgActorManager_c::BgObjName_t l_object_name[2] = {
    {0x35, 0x18E, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x0, 0x2EB, 0x0, mVec3_c(0, 0, 0), mVec2_c(0, 0), 0x0},
};

dBgActorManager_c::BgObjName_t l_Pa3_rail[28] = {
    {0x35, 0x18E, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x301, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x300, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x1},
    {0x302, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x2},
    {0x350, 0x18D, 0x0, mVec3_c(8, 0, 0), mVec2_c(24, 48), 0x3},
    {0x351, 0x18D, 0x0, mVec3_c(8, 0, 0), mVec2_c(32, 48), 0x4},
    {0x352, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x5},
    {0x353, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x6},
    {0x354, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x7},
    {0x356, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x8},
    {0x378, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x9},
    {0x3A3, 0x18D, 0x0, mVec3_c(16, 0, 0), mVec2_c(48, 48), 0xA},
    {0x3A1, 0x18D, 0x0, mVec3_c(16, 0, 0), mVec2_c(48, 48), 0xB},
    {0x371, 0x18D, 0x0, mVec3_c(16, -16, 0), mVec2_c(48, 48), 0xC},
    {0x374, 0x18D, 0x0, mVec3_c(0, -16, 0), mVec2_c(48, 48), 0xD},
    {0x377, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0xE},
    {0x376, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0xF},
    {0x387, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x10},
    {0x386, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x11},
    {0x310, 0x18D, 0x0, mVec3_c(8, -16, 0), mVec2_c(32, 48), 0x12},
    {0x311, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x13},
    {0x321, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x14},
    {0x313, 0x18D, 0x0, mVec3_c(8, -16, 0), mVec2_c(32, 48), 0x15},
    {0x314, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x16},
    {0x324, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x17},
    {0x315, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x18},
    {0x325, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x19},
    {0x0, 0x2EB, 0x0, mVec3_c(0, 0, 0), mVec2_c(0, 0), 0x0},
};

dBgActorManager_c::BgObjName_t l_Pa3_MG_house_ami_rail[19] = {
    {0x35, 0x18E, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x350, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x351, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x340, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x1},
    {0x342, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x1},
    {0x352, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x2},
    {0x343, 0x18D, 0x0, mVec3_c(8, -16, 0), mVec2_c(32, 48), 0x12},
    {0x344, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x13},
    {0x354, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x14},
    {0x346, 0x18D, 0x0, mVec3_c(8, -16, 0), mVec2_c(32, 48), 0x15},
    {0x3A3, 0x18D, 0x0, mVec3_c(16, 0, 0), mVec2_c(48, 48), 0xA},
    {0x3A1, 0x18D, 0x0, mVec3_c(16, 0, 0), mVec2_c(48, 48), 0xB},
    {0x371, 0x18D, 0x0, mVec3_c(16, -16, 0), mVec2_c(48, 48), 0xC},
    {0x374, 0x18D, 0x0, mVec3_c(0, -16, 0), mVec2_c(48, 48), 0xD},
    {0x377, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0xE},
    {0x376, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0xF},
    {0x387, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x10},
    {0x386, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x11},
    {0x0, 0x2EB, 0x0, mVec3_c(0, 0, 0), mVec2_c(0, 0), 0x0},
};

dBgActorManager_c::BgObjName_t l_Pa3_daishizen[13] = {
    {0x35, 0x18E, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x301, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x0},
    {0x300, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x1},
    {0x302, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x2},
    {0x310, 0x18D, 0x0, mVec3_c(8, -16, 0), mVec2_c(32, 48), 0x12},
    {0x311, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x13},
    {0x321, 0x18D, 0x0, mVec3_c(16, -8, 0), mVec2_c(48, 32), 0x14},
    {0x313, 0x18D, 0x0, mVec3_c(8, -16, 0), mVec2_c(32, 48), 0x15},
    {0x314, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x16},
    {0x324, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x17},
    {0x315, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x18},
    {0x325, 0x18D, 0x0, mVec3_c(8, -8, 0), mVec2_c(32, 32), 0x19},
    {0x0, 0x2EB, 0x0, mVec3_c(0, 0, 0), mVec2_c(0, 0), 0x0},
};

dBgActorManager_c::BgObjName_t *l_rail_list[5] = {
    l_object_name, l_Pa3_rail, l_Pa3_rail, l_Pa3_daishizen,
    l_Pa3_MG_house_ami_rail,
};

// ---------------------------------------------------------------------------
// Method bodies (hand-authored against target disasm)
// ---------------------------------------------------------------------------

dBgActorManager_c::dBgActorManager_c() {
    m_pObjList = nullptr;
    m_objNum = 0;
    ms_instance = this;
}

dBgActorManager_c::~dBgActorManager_c() {
    for (int i = 0; i < m_objNum; i++) {
        if (m_pObjList[i].mRailIdx != 0xFFFF) {
            if (m_pObjList[i].mActorId != 0) {
                m_pObjList[i].deleteActor();
            }
            m_pObjList[i].clear();
        }
    }
    if (mAllocator.mpHeap != mAllocatorDummyHeap_c::getInstance()) {
        mAllocator.destroyHeap();
    }
    ms_instance = nullptr;
    l_pRailList = nullptr;
}

void dBgActorManager_c::initialize() {
    dCdFile_c *file =
        &dCd_c::m_instance->mFiles[*reinterpret_cast<u8 *>((u8 *)dScStage_c::m_instance + 0x120E)];
    file = (file->mpAreas != nullptr) ? file : nullptr;
    sTilesetData *tilesets = file->mpTilesetNames;
    if (strcmp(tilesets->mTileset3, "Pa3_rail") == 0) {
        m_area = 1;
    } else if (strcmp(tilesets->mTileset3, "Pa3_rail_white") == 0) {
        m_area = 2;
    } else if (strcmp(tilesets->mTileset3, "Pa3_daishizen") == 0) {
        m_area = 3;
    } else if (strcmp(tilesets->mTileset3, "Pa3_MG_house_ami_rail") == 0) {
        m_area = 4;
    } else {
        m_area = 0;
    }
    l_pRailList = l_rail_list[m_area];
}

void dBgActorManager_c::create() {
    mAllocator.createFrmHeapToCurrent(0x10000, EGG::Heap::getCurrentHeap(),
                                      "dBgActorManager_c::m_allocator", 0x20,
                                      (mHeap::AllocOptBit_t)0);
    CreateHeap();
    mAllocator.adjustFrmHeapRestoreCurrent();
}

int dBgActorManager_c::CreateHeap() {
    m_objNum = createObjList(false);
    if (m_objNum > 0) {
        m_pObjList = new BgObj_c[m_objNum];
        for (int i = 0; i < m_objNum; i++) {
            m_pObjList[i].init();
        }
        createObjList(true);
    }
    return 1;
}

void dBgActorManager_c::execute() {
    dBgParameter_c *param = dBgParameter_c::ms_Instance_p;
    const f32 zero = 0.0f;
    f32 sizeY;
    f32 x = param->mPos.x;
    mMin.x = x;
    sizeY = param->mSize.y;
    f32 posY = param->mPos.y;
    mMin.y = posY - sizeY;
    mMin.z = zero;
    mMax.x = x + param->mSize.x;
    mMax.y = param->mPos.y;
    mMax.z = zero;
    ProcMain();
}

void dBgActorManager_c::ProcMain() {
    if (m_pObjList == nullptr) {
        return;
    }
    dBg_c *bg = dBg_c::m_bg_p;
    mVec3_c viewMin = mMin;
    mVec3_c viewMax = mMax;
    int x0 = (int)(bg->m_8fe64 * 0.0625f);
    int y0 = (int)(bg->m_8fe6c * 0.0625f);
    for (int i = 0; i < m_objNum; i++) {
        if (m_pObjList[i].mRailIdx == 0xFFFF) {
            continue;
        }
        mVec3_c pos((f32)((int)((x0 + m_pObjList[i].mX) << 4)),
                    (f32)((int)((y0 - m_pObjList[i].mY) << 4)), 0.0f);
        pos.x += m_pObjList[i].getOffset().x;
        pos.y += m_pObjList[i].getOffset().y;
        mVec3_c mMin = pos;
        mMin.x -= m_pObjList[i].getSize().x * 0.5f;
        mMin.y -= m_pObjList[i].getSize().y * 0.5f;
        mVec3_c mMax = pos;
        mMax.x += m_pObjList[i].getSize().x * 0.5f;
        mMax.y += m_pObjList[i].getSize().y * 0.5f;
        if (m_pObjList[i].mActorId != 0) {
            if (!dGameCom::checkRectangleOverlap(&mMin, &mMax, &viewMin, &viewMax, 0.0f)) {
                m_pObjList[i].deleteActor();
            }
        } else {
            if (dGameCom::checkRectangleOverlap(&mMin, &mMax, &viewMin, &viewMax, 0.0f)) {
                m_pObjList[i].createActor(0u, pos);
            }
        }
    }
}

bool dBgActorManager_c::addObj(u16 a, u16 b, u16 c, u8 d) {
    int n = m_objNum;
    for (int i = 0; i < n; i++) {
        if (m_pObjList[i].mRailIdx != 0xFFFF) {
            continue;
        }
        m_pObjList[i].set(a, b, c, d);
        return true;
    }
    return false;
}

int dBgActorManager_c::createObjList(bool add) {
    dBg_c *bg = dBg_c::m_bg_p;
    u32 x0 = (u32)(bg->m_8fe64 * 0.0625f);
    u32 y0 = (u32)(-(bg->m_8fe6c) * 0.0625f);
    int x1 = (int)(bg->m_8fe68 - bg->m_8fe64);
    int y1 = (int)(bg->m_8fe6c - bg->m_8fe70);
    u16 x1u = (u16)x1;
    u16 y1u = (u16)y1;
    x1u = (x1u & 0xF) ? (x1u >> 4) + 1 : (x1u >> 4);
    y1u = (y1u & 0xF) ? (y1u >> 4) + 1 : (y1u >> 4);
    int count = 0;
    for (u16 j = 0; j < y1u; j++) {
        u16 gridY = (u16)((j + y0) << 4);
        for (u16 i = 0; i < x1u; i++) {
            u16 gridX = (u16)((i + x0) << 4);
            for (u8 layer = 0; layer < 3; layer++) {
                if (!bg->CheckExistLayer(layer)) {
                    continue;
                }
                u16 unit = bg->GetMaskedUnitNumber(gridX, gridY, layer);
                if (unit == 0xFFFF) {
                    unit = 0;
                }
                for (u32 k = 0; ; k++) {
                    BgObjName_t *entry = &l_pRailList[k];
                    if (entry->mName == 0x2EB) {
                        break;
                    }
                    if (entry->mUnit == unit) {
                        if (add) {
                            addObj(k, i, j, layer);
                        }
                        count++;
                        break;
                    }
                }
            }
        }
    }
    return count;
}

void dBgActorManager_c::BgObj_c::init() {
    mRailIdx = 0xFFFF;
    mX = 0;
    mY = 0;
    mActorId = 0;
}

void dBgActorManager_c::BgObj_c::clear() {
    init();
}

void dBgActorManager_c::BgObj_c::set(u16 railIdx, u16 x, u16 y, u8 type) {
    mRailIdx = railIdx;
    mX = x;
    mY = y;
    mType = type;
}

ulong dBgActorManager_c::BgObj_c::createActor(ulong actorId, mVec3_c &pos) {
    switch (mType) {
    case 0: {
        mVec3_c offset = getOffset();
        pos.z = offset.z;
        break;
    }
    case 1: {
        mVec3_c offset = getOffset();
        pos.z = -3500.0f + offset.z;
        break;
    }
    case 2: {
        mVec3_c offset = getOffset();
        pos.z = offset.z;
        break;
    }
    }
    BgObjName_t *entry = &l_pRailList[mRailIdx];
    dBaseActor_c *actor =
        dBaseActor_c::construct(entry->mName, entry->mParam | (mType << 8), &pos, nullptr);
    if (actor != nullptr) {
        mActorId = *(u32 *)actor;
    } else {
        mActorId = 0;
    }
    return mActorId;
}

void dBgActorManager_c::BgObj_c::deleteActor() {
    fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)mActorId);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorId = 0;
}

mVec3_c dBgActorManager_c::BgObj_c::getOffset() {
    BgObjName_t *entry = &l_pRailList[mRailIdx];
    return entry->mOffset;
}

mVec2_c dBgActorManager_c::BgObj_c::getSize() {
    BgObjName_t *entry = &l_pRailList[mRailIdx];
    return entry->mSize;
}
