#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/sLib/s_StateID.hpp>
#include <game/sLib/s_State.hpp>
#include <game/bases/d_game_com.hpp>
#include <lib/nw4r/g3d/g3d_obj.h>
#include <lib/nw4r/g3d/g3d_scnmdl.h>

// setNodeVisibility(m3d::bmdl_c*, int nodeId, int visible) -- not landed anywhere yet
// (grepped include/game/mLib/m_3d/*.hpp, not found). Declared via its own real mangled
// name so the linker resolves it directly, matching this project's own convention for an
// undocumented cross-TU call.
namespace d3d { void setNodeVisibility(m3d::bmdl_c *mdl, int nodeId, int visible); }

// sBgSetInfo -- an un-landed struct (grepped include/, not found) passed by const-pointer to
// dBg_ctr_c's own (real, but not in the landed header) `set(dActor_c*, const sBgSetInfo*, u8,
// u8, mVec3_c*)` overload. Layout confirmed from the target's own stack construction: 4 floats
// then 3 zero-initialized ints, size 0x1c.
struct sBgSetInfo {
    float m_0, m_4, m_8, m_c;
    int m_10, m_14, m_18;
};

// dBg_ctr_c::set(dActor_c*, const sBgSetInfo*, u8, u8, mVec3_c*) -- a REAL, already-compiled
// dBg_ctr_c member (confirmed by its own real mangled name in the target disasm) but NOT one of
// the two overloads in the landed d_bg_ctr.hpp. Declared via its exact mangled name so the
// linker resolves it directly, matching this project's own convention for a real member not yet
// in the landed header.
extern "C" void set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c(
    dBg_ctr_c *self, dActor_c *actor, const sBgSetInfo *info, u8 a, u8 b, mVec3_c *vec);

/// @unofficial Background-layer controller for MIDDLE_BG_FOR_CASTLE_LUDWIG (used directly, no
/// further subclass), base of #daBottomBGForCastleLudwig_c (BOTTOM_BG_FOR_CASTLE_LUDWIG).
///
/// **REAL CLASS NAME, not inferred**: found directly in `.data` as a state-ID name string
/// literal, \c "daMiddleBGForCastleLudwig_c::StateID_DemoWait" (capital `BG`, not `Bg` --
/// corrected this round from an earlier guess). The string sits immediately after this class's
/// own vtable object in \c lbl_2_data_30C3C (dtk merges adjacent no-gap objects under one
/// label), which is the SAME evidence that caught a real class-hierarchy bug this round: see
/// the .cpp's own note on the vtable-slot correction.
///
/// Derives from #dEn_c (confirmed: `bl __ct__5dEn_cFv` in the ctor, `bl __dt__5dEn_cFv` in the
/// destructor). `sizeof == 0x768` (both classInits' own `li r3, 0x768`).
///
/// Owns two #dBg_ctr_c zones (`__construct_array`/`__destroy_arr` with `sizeof(dBg_ctr_c)==0xe4`,
/// count 2), a heap allocator, and a model.
///
/// Declares NINE of its own new virtuals past `dEn_c`'s own last one (`yoshifumiEffect`,
/// `d_enemy.hpp:220`), at vtable offsets `0x280/0x284/0x288/0x28c/0x290/0x294/0x298/0x29c/0x2a0`
/// -- confirmed by extracting and offset-indexing the FULL vtable dump programmatically.
/// **CORRECTED this round**: a full slot-by-slot diff of BOTH classes' complete vtables (not
/// just an eyeballed prefix) found that only 4 slots actually differ between
/// #daMiddleBGForCastleLudwig_c and #daBottomBGForCastleLudwig_c -- the destructor (slot 18) AND
/// THREE of the nine new virtuals (`0x280`/`0x298`/`0x29c`), NOT the destructor alone. The other
/// six of the nine ARE genuinely shared/inherited unmodified. An earlier draft this round had
/// the three differing slots' functions attached to the WRONG class (backwards) -- caught before
/// being reported as done, per this project's own "diff anything you believe is boilerplate"
/// caution. Real names are placeholders (`vfXXX` by vtable offset) except `createModel`, which a
/// real \c getRes()/GetResMdl() call site licenses.
class daMiddleBGForCastleLudwig_c : public dEn_c {
public:
    daMiddleBGForCastleLudwig_c();
    virtual ~daMiddleBGForCastleLudwig_c();

    // #getNullState (fn_2_F51B0). Confirmed content: `return &sStateID::null;` (a REAL, already
    // landed extern, `include/game/sLib/s_StateID.hpp:42`).
    static const sStateID_c &getNullState();

    // Base overrides -- vtable slot 2/5/8/11 (offset 0x08/0x14/0x20/0x2C), confirmed IDENTICAL
    // in both classes' vtables (a full slot-by-slot diff, not an eyeballed prefix). NOT YET
    // AUTHORED this round (fake stubs in the .cpp).
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    // The 9 new virtuals, in real vtable order (offset 0x280 through 0x2a0). SIX are shared
    // unmodified by #daBottomBGForCastleLudwig_c (confirmed by the full diff): vf284/vf288/
    // vf28c/vf290/vf294/vf2a0. THREE are overridden by it (vf280/vf298/vf29c) -- this class's
    // own bodies for those three are its OWN distinct implementation, confirmed NOT
    // byte-identical to the derived class's own override (different sizes: 0x98 vs 4 bytes,
    // 0x124 vs 0x124 -- see #createModel's own note -- and 0x18C vs 0x128).
    // fn_2_F5380. Confirmed content: for each of the 2 node names (#sc_nodeNames),
    // randomly rolls #m_764[i] (rndInt(100) < 10, a 10% chance -- exact bit-trick codegen
    // not yet reproduced, semantics confirmed by hand-tracing the cntlzw/slw/srwi sequence),
    // looks up the node via mModel's own ResMdl, and calls setNodeVisibility with the node's
    // own id (0 if not found) and #m_764[i].
    virtual void vf280();
    // fn_2_F5430. Confirmed content: copies #m_764[i] from \p other (byte-for-byte, same
    // index), then the SAME node-lookup/setNodeVisibility idiom as #vf280.
    virtual void vf284(daMiddleBGForCastleLudwig_c *other);
    virtual void vf288(); ///< fn_2_F5C00. Real content: forwards to #vf2a0.
    // DemoWait -- a virtual state (STATE_VIRTUAL_FUNC_DECLARE/DEFINE), per the coordinator's
    // own relocation lookup: exactly one state name exists anywhere in this unit's .data
    // ("daMiddleBGForCastleLudwig_c::StateID_DemoWait", read directly, not guessed), and its
    // three PMF fields (found immediately after this class's own vtable object in .data,
    // lbl_2_data_30C3C) point at vtable offsets 0x294/0x290/0x28c in exactly that order --
    // matching STATE_VIRTUAL_DEFINE's own argument order (&initializeState, &executeState,
    // &finalizeState). So: initializeState_DemoWait = the old vf294 (fn_2_F5980, empty body),
    // executeState_DemoWait = the old vf290 (fn_2_F5990, forwards into #mModel's own vtable),
    // finalizeState_DemoWait = the old vf28c (fn_2_F5970, empty body). Declared on THIS class;
    // the coordinator's own note that BOTH classes have the state is addressed on
    // #daBottomBGForCastleLudwig_c below.
    STATE_VIRTUAL_FUNC_DECLARE(daMiddleBGForCastleLudwig_c, DemoWait);

    // #createModel (fn_2_F5550 here -- THIS class's own override, 0x124 bytes, matching
    // #daBottomBGForCastleLudwig_c's own override at the SAME slot, fn_2_F59A0, ALSO 0x124
    // bytes exactly). Real content this round: the standard createFrmHeap/getRes/GetResMdl/
    // m3d::mdl_c::create/setSoftLight_Map/adjustFrmHeap idiom, using MIDDLE_BG's own arc/model
    // strings read DIRECTLY out of this unit's own `.data` (`lbl_2_data_30938`/`lbl_2_data_30954`,
    // decoded byte-for-byte: `"g3d/W7_shiroboss_bg_M.brres"` / `"W7_shiroboss_bg_M"`) -- not
    // guessed, and not the same string the derived class's own override uses (`..._D`, "Down" --
    // matches BOTTOM_BG's own "bottom" role, MIDDLE_BG's own "M" is presumably "Middle").
    virtual void createModel();

    virtual void vf29c(); ///< fn_2_F5680 (0x18C bytes -- the LARGEST function in the unit). NOT
                            ///< YET AUTHORED.
    virtual void vf2a0(); ///< fn_2_F5890. Real content: a matrix update.

    // #entryOrRelease (fn_2_F52D0). Confirmed content: `if (doEntry) mBgCtr[0].entry(); else
    // mBgCtr[0].release();` -- touches ONLY zone 0, not #mBgCtr[1]. NOT in the vtable.
    void entryOrRelease(bool doEntry);

    // fn_2_F52F0. Confirmed content: NOT a vtable slot. `mModel.setOption(1, arg?0:1);`
    // then, on the SAME single bool arg, either `mBgCtr[1].entry(); mBgCtr[0].entry();` or
    // `mBgCtr[1].release(); mBgCtr[0].release();` -- BOTH zones this time, unlike
    // #entryOrRelease's own zone-0-only shape.
    void activate(bool show);

    dHeapAllocator_c mAllocator; ///< @unofficial offset 0x524, size 0x1c.
    void *m_540; ///< @unofficial offset 0x540. Zeroed by the ctor; #createModel stores its own
                  ///< `getRes()` result here -- likely a cached resource-file handle.
    m3d::mdl_c mModel; ///< @unofficial offset 0x544, size 0x40 (probed from the gap to #mBgCtr).
    dBg_ctr_c mBgCtr[2]; ///< @unofficial offset 0x584, size 0x1c8 (2 * 0xe4).

    // @unofficial offset 0x74c (compiler-computed -- #mBgCtr[2] ends there), size 0x1c. Both
    // classInits' own `li r3, 0x768` alloc size requires this trailing gap (0x768-0x74c);
    // content NOT YET confirmed.
    u8 mPad74c[0x18]; ///< @unofficial offset 0x74c, size 0x18 -- still unconfirmed.

    // @unofficial offset 0x764, size 0x2. Per-node random/copied visibility flags read by
    // #vf280/#vf284 and passed to setNodeVisibility.
    u8 m_764[2];

    u8 mPad766[0x2]; ///< @unofficial offset 0x766, size 0x2 -- still unconfirmed.
};

/// @unofficial Subclass for BOTTOM_BG_FOR_CASTLE_LUDWIG. **CORRECTED this round**: NOT a
/// destructor-only override as an earlier draft assumed -- overrides FOUR slots (destructor
/// plus THREE of the nine new virtuals: `vf280`/`createModel`/`vf29c`), confirmed by a full
/// slot-by-slot vtable diff. Adds no members (both classInits' own `li r3, 0x768` alloc size is
/// identical).
class daBottomBGForCastleLudwig_c : public daMiddleBGForCastleLudwig_c {
public:
    virtual ~daBottomBGForCastleLudwig_c();

    // BOTTOM_BG's OWN DemoWait, confirmed real by direct __sinit bytes (fn_2_F5C80):
    // constructs an `sFStateVirtualID_c<daBottomBGForCastleLudwig_c>` at runtime, copying the
    // SAME name string and the SAME three PMF values as the base's own StateID_DemoWait.
    // STATE_VIRTUAL_DEFINE's OWN macro cannot be invoked a second time for the same state name
    // (its `baseID_##name` helper is a static file-scope function template shared across every
    // invocation, not the state itself) -- so this is declared normally via
    // STATE_VIRTUAL_FUNC_DECLARE, then its `StateID_DemoWait` OBJECT is hand-expanded in the
    // .cpp (matching the landed `d_a_ac_switch.cpp` precedent for the identical class of
    // problem, `ACTOR_PROFILE` colliding when invoked seven times for one class), passing
    // `daMiddleBGForCastleLudwig_c::StateID_DemoWait` directly as the base-state argument
    // (exactly what the macro's own `baseID_##name<StateIDBase_##name>()` would have resolved
    // to here, since the base class DOES declare this state).
    // Hand-declared (NOT the full STATE_VIRTUAL_FUNC_DECLARE macro): __sinit's own bytes
    // show BOTTOM_BG's construction COPYING its 3 PMF values from
    // daMiddleBGForCastleLudwig_c::StateID_DemoWait's own already-compiled fields at
    // runtime, rather than taking fresh addresses of separately-declared BOTTOM_BG
    // methods -- matching the standard C++ base-to-derived pointer-to-member conversion
    // (&Base::method implicitly convertible to a Derived::* PMF, needing this exact
    // runtime copy since the general PMF representation isn't a compile-time constant).
    // So BOTTOM_BG does NOT declare its own separate initializeState_DemoWait/
    // executeState_DemoWait/finalizeState_DemoWait at all -- just the static member.
    static sFStateVirtualID_c<daBottomBGForCastleLudwig_c> StateID_DemoWait;

    // fn_2_F5C10 (0x280). Real content: empty body -- confirmed genuinely different from the
    // base's own #daMiddleBGForCastleLudwig_c::vf280 (0x98 bytes there), not the same function
    // shared.
    virtual void vf280();

    // fn_2_F59A0 (0x298, THIS class's own #createModel override). Real content: the SAME
    // createFrmHeap/getRes/GetResMdl/create/setSoftLight_Map/adjustFrmHeap idiom as the base
    // class's own override, but using BOTTOM_BG's own arc/model strings, ALSO read directly out
    // of `.data` (`lbl_2_data_30968`/`lbl_2_data_30984`: `"g3d/W7_shiroboss_bg_D.brres"` /
    // `"W7_shiroboss_bg_D"`).
    virtual void createModel();

    virtual void vf29c(); ///< fn_2_F5AD0 (0x128 bytes). NOT YET AUTHORED (unscouted).
};
