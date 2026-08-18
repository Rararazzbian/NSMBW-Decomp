#pragma once

#include <game/bases/d_wm_connect.hpp>
#include <game/bases/d_wm_csv_data.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_wm_map_model.hpp>

/// @unofficial SHADOW COPY, proposed addition to the real include/game/bases/d_a_wm_map.hpp.
/// Adds `GetPos(int, mVec3_c &)`, called by daWmAntlionMng_c::reviveOnRoute (fn_2_15BC30) as
/// `GetPos__9daWmMap_cFi` -- signature read from the call site (`r3`=hidden return slot,
/// `r4`=this, `r5`=index), not yet reconstructed/verified against a body.
class daWmMap_c : public dWmDemoActor_c {
public:
    int GetNodeCount(int); ///< @unofficial
    void GetNodePos(long nodeIdx, mVec3_c &pos);
    /// @unofficial Looks up a node by name instead of index. Evidenced by
    /// daWmSmallCloud_c::setPosFromCourseNode() (0x179F10) tail-calling
    /// GetNodePos__9daWmMap_cFPCcR7mVec3_c.
    void GetNodePos(const char *nodeName, mVec3_c &pos);
    /// @unofficial Proposed this round; see the file-level comment above.
    mVec3_c GetPos(int index);

    dHeapAllocator_c mAllocator;
    dWmMapModel_c mModels[4];
    u8 mPad1[0x20c];
    int currIdx;
    u8 mPad2[0x4];
    dCsvData_c mCsvData[4];
    dWmConnect_c mWmConnect[4];

    static daWmMap_c *m_instance;
};
