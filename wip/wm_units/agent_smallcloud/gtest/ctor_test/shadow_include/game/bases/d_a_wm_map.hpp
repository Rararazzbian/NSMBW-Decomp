#pragma once

#include <game/bases/d_wm_connect.hpp>
#include <game/bases/d_wm_csv_data.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_wm_map_model.hpp>

/// @unofficial SHADOW COPY for compiling wip/wm_smallcloud only. NOT for landing as-is; the only
/// difference from the real include/game/bases/d_a_wm_map.hpp is the new GetNodePos(const char*, ...)
/// overload below, proposed to the lead in MERGED.md.
class daWmMap_c : public dWmDemoActor_c {
public:
    int GetNodeCount(int); ///< @unofficial
    void GetNodePos(long nodeIdx, mVec3_c &pos);
    /// @unofficial @proposed looks up a node by name instead of index; evidenced by
    /// daWmSmallCloud_c::setPosFromCourseNode() (0x179F10) calling
    /// GetNodePos__9daWmMap_cFPCcR7mVec3_c. Proposed shared-header addition, see MERGED.md.
    void GetNodePos(const char *nodeName, mVec3_c &pos);

    dHeapAllocator_c mAllocator;
    dWmMapModel_c mModels[4];
    u8 mPad1[0x20c];
    int currIdx;
    u8 mPad2[0x4];
    dCsvData_c mCsvData[4];
    dWmConnect_c mWmConnect[4];

    static daWmMap_c *m_instance;
};
