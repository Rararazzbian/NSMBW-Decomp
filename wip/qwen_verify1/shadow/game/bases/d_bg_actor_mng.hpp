#pragma once
// SHADOW COPY of a proposed header for dBgActorManager_c (d_bg_actor_mng.hpp).
// PROPOSED additions:
//   - dBgActorManager_c class with nested BgObj_c / BgObjName_t structs.
//   - BgObjName_t is an AGGREGATE (no ctor -- only a user-provided dtor).
//     This is deliberate and load-bearing: the aggregate shape + aggregate
//     { ... } initializers is what makes MWCC emit `b __register_global_object`
//     (tail call) in __sinit_, plus one __arraydtor$ thunk and one 0xC .bss
//     register node per array. probe_agg.cpp confirmed this byte-for-byte.
#include <types.h>
#include <game/mLib/m_vec.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_cd_data.hpp>

class dBgActorManager_c {
public:
    /// @unofficial
    struct BgObj_c {
        BgObj_c() {} ///< forces the `new[]` to emit __construct_new_array

        u16 mRailIdx; ///< index into l_rail_list; 0xFFFF = free slot
        u16 mX;
        u16 mY;
        u8 mType;
        u32 mActorId;

        void init();
        void clear();
        void set(u16 railIdx, u16 x, u16 y, u8 type);
        ulong createActor(ulong actorId, mVec3_c &pos);
        void deleteActor();
        mVec3_c getOffset();
        mVec2_c getSize();
    };

    /// @unofficial
    /// @note AGGREGATE on purpose: no ctor (so `{ ... }` initializers fold the
    /// int fields into .data), only the dtor (which forces the arraydtor +
    /// __register_global_object registration). The mVec3_c/mVec2_c members are
    /// ctor-calls in the initializer (non-foldable) so the floats zero in .data
    /// and are written at runtime by __sinit_ -- the target's exact pattern.
    struct BgObjName_t {
        ~BgObjName_t();

        u32 mUnit;   ///< course-file unit number this rail belongs to
        u16 mName;   ///< 0x2EB marks the terminator entry
        u16 mFlag;
        mVec3_c mOffset; ///< @unofficial
        mVec2_c mSize;   ///< @unofficial
        u32 mParam;
    };

    dBgActorManager_c();
    virtual ~dBgActorManager_c();

    void initialize();
    void create();
    int CreateHeap();
    void execute();
    void ProcMain();
    bool addObj(u16 a, u16 b, u16 c, u8 d);
    int createObjList(bool add);

    dHeapAllocator_c mAllocator; ///< +0x04 (after the vptr at +0x00)

    mVec3_c mMin; ///< +0x20 view rect min (left, bottom, 0)
    mVec3_c mMax; ///< +0x2C view rect max (right, top, 0)
    BgObj_c *m_pObjList; ///< +0x38
    int m_objNum;        ///< +0x3C (signed in the binary: cmpw/cmpwi)
    u32 m_area;          ///< +0x40

    static dBgActorManager_c *ms_instance; ///< .sbss:0x8042A0B8
};
