#pragma once

#include <game/bases/d_wm_connect.hpp>
#include <game/bases/d_wm_csv_data.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_wm_map_model.hpp>

class daWmMap_c : public dWmDemoActor_c {
public:
    int GetNodeCount(int); ///< @unofficial
    void GetNodePos(long nodeIdx, mVec3_c &pos);
    /// @unofficial Looks up a node by name instead of index. Evidenced by
    /// daWmSmallCloud_c::setPosFromCourseNode() (0x179F10) tail-calling
    /// GetNodePos__9daWmMap_cFPCcR7mVec3_c.
    void GetNodePos(const char *nodeName, mVec3_c &pos);

    /// @brief Returns the world-space position of node @p nodeIdx. @unofficial PROPOSED
    /// (WM_NOTE round): mangled GetPos__9daWmMap_cFi, returned by value via hidden pointer
    /// (confirmed by the target passing a stack address in r3 ahead of `this`/nodeIdx).
    mVec3_c GetPos(int nodeIdx);

    dHeapAllocator_c mAllocator;
    dWmMapModel_c mModels[4];
    u8 mPad1[0x20c];
    int currIdx;
    u8 mPad2[0x4];
    dCsvData_c mCsvData[4];
    dWmConnect_c mWmConnect[4];

    static daWmMap_c *m_instance;
};
