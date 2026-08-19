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
    /// @unofficial SHADOW COPY (agent_hanachan): mangled GetPos__9daWmMap_cFPCc. NOT yet
    /// applied to the real header -- awaiting lead review. Evidenced by
    /// bin/dtk/wiimj2d_symbols.txt: `GetPos__9daWmMap_cFPCc = .text:0x80100380` sits
    /// immediately after `GetPos__9daWmMap_cFi = .text:0x80100310` (size 0x64), and
    /// daWmHanachan_c's own fn_2_1657E0 calls it directly (r3=hidden return ptr, r4=
    /// daWmMap_c::m_instance loaded via the class's own static instance pointer, r5=a
    /// name-string literal from lbl_2_data_44E20) -- the same by-value-return-via-hidden-
    /// pointer calling convention already established for GetPos(int), and the same
    /// name-vs-index overload pattern already precedented by GetNodePos above.
    mVec3_c GetPos(const char *name);
    dHeapAllocator_c mAllocator;
    dWmMapModel_c mModels[4];
    u8 mPad1[0x20c];
    int currIdx;
    u8 mPad2[0x4];
    dCsvData_c mCsvData[4];
    dWmConnect_c mWmConnect[4];

    static daWmMap_c *m_instance;
};
