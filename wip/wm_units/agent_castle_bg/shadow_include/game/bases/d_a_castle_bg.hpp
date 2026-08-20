#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/sLib/s_StateID.hpp>

/// @unofficial Background-layer controller shared by MIDDLE_BG_FOR_CASTLE_LUDWIG (used
/// directly, no subclass -- its own classInit's `new` targets THIS class, and its vtable
/// pointer is never re-stomped afterward) and BOTTOM_BG_FOR_CASTLE_LUDWIG (a thin subclass,
/// #daBottomBgForCastleLudwig_c below, that overrides ONLY the destructor).
///
/// Reconstructed from anonymous target symbols; class name and every member name are inferred
/// from codegen evidence, not from any mangled name. Derives from #dEn_c (confirmed:
/// `bl __ct__5dEn_cFv` in the ctor, `bl __dt__5dEn_cFv` in the destructor). `sizeof == 0x768`
/// (both classInits' own `li r3, 0x768`).
///
/// Owns two #dBg_ctr_c zones (`__construct_array`/`__destroy_arr` with `sizeof(dBg_ctr_c)==0xe4`,
/// count 2 -- confirmed against the real landed `dBg_ctr_c` in `d_bg_ctr.hpp`), a heap allocator,
/// and a model.
///
/// Declares NINE of its own new virtuals past `dEn_c`'s own last one (`yoshifumiEffect`,
/// `d_enemy.hpp:220`) -- confirmed by extracting and offset-indexing the FULL vtable dump
/// (`lbl_2_data_30998`, 0x2A4 bytes = 169 slots, programmatically, not eyeballed) at
/// `0x280/0x284/0x288/0x28c/0x290/0x294/0x298/0x29c/0x2a0`. Real names are placeholders (`vfXXX`
/// by vtable offset, not real names) -- no mangled symbol licenses better ones (the project's
/// own "a mangled name is a licence to name a type; a bare vtable index is not" rule).
///
/// BINDING NOTE (the lesson from landing DUMMY_DOOR): every one of these, even the three with an
/// empty body, is declared here WITHOUT an in-class body and DEFINED OUT OF LINE in the .cpp.
/// An in-class `{}` compiles WEAK and gets deferred to the end of the translation unit (LIFO) in
/// this compiler; the target's own `fn_2_F5C10`/`fn_2_F5970`/`fn_2_F5980` (the three empty ones)
/// sit at their own ordinary interleaved address positions, not clustered at the file's end, so
/// they must be GLOBAL/out-of-line to match.
class daMiddleBgForCastleLudwig_c : public dEn_c {
public:
    daMiddleBgForCastleLudwig_c();
    virtual ~daMiddleBgForCastleLudwig_c();

    // #getNullState (fn_2_F51B0). Confirmed content: `return &sStateID::null;` (a REAL, already
    // landed extern, `include/game/sLib/s_StateID.hpp:42`) -- no `this` use at all (r3 is
    // overwritten immediately, no frame), so modelled as `static`. Not found as a `bl` target
    // anywhere in this unit's own two `.text` objects; likely consumed by one of the
    // still-unauthored functions below (a `dEn_c::mDeathState`-shaped assignment is the obvious
    // candidate) or by code outside this unit's own claimed range.
    static const sStateID_c &getNullState();

    // Base overrides -- vtable slot 2/5/8/11 (offset 0x08/0x14/0x20/0x2C), confirmed via direct
    // read of BOTH target vtables (identical at these four slots in both -- shared by MIDDLE_BG
    // and BOTTOM_BG unmodified). NOT YET AUTHORED this round (fake stubs in the .cpp).
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    // The 9 new virtuals, in real vtable order (offset 0x280 through 0x2a0).
    virtual void vf280(); ///< fn_2_F5C10. Real content: empty body.
    virtual void vf284(daMiddleBgForCastleLudwig_c *other); ///< fn_2_F5430. NOT YET AUTHORED.
    virtual void vf288(); ///< fn_2_F5C00. Real content: forwards to #vf2a0.
    virtual void vf28c(); ///< fn_2_F5970. Real content: empty body.
    virtual void vf290(); ///< fn_2_F5990. Real content: forwards into #mModel's own vtable.
    virtual void vf294(); ///< fn_2_F5980. Real content: empty body.
    virtual void createModel(); ///< fn_2_F59A0 (vtable offset 0x298). Real content.
    virtual void vf29c(); ///< fn_2_F5AD0. NOT YET AUTHORED (0x128, unscouted).
    virtual void vf2a0(); ///< fn_2_F5890 (vtable offset 0x2a0). Real content: a matrix update.

    // #entryOrRelease (fn_2_F52D0). Confirmed content: `if (doEntry) mBgCtr[0].entry(); else
    // mBgCtr[0].release();` -- a tail call either way, touching ONLY zone 0, not #mBgCtr[1].
    // NOT in the vtable (no `fn_2_F52D0` entry found anywhere in either dump) -- an ordinary
    // helper, not an override.
    void entryOrRelease(bool doEntry);

    dHeapAllocator_c mAllocator; ///< @unofficial offset 0x524, size 0x1c.
    void *m_540; ///< @unofficial offset 0x540. Zeroed by the ctor; #createModel stores its own
                  ///< `getRes()` result here -- likely a cached resource-file handle.
    m3d::mdl_c mModel; ///< @unofficial offset 0x544, size 0x40 (probed from the gap to #mBgCtr).
    dBg_ctr_c mBgCtr[2]; ///< @unofficial offset 0x584, size 0x1c8 (2 * 0xe4).

    // @unofficial offset 0x74c (compiler-computed -- #mBgCtr[2] ends there), size 0x1c. Both
    // classInits' own `li r3, 0x768` alloc size requires this trailing gap (0x768-0x74c);
    // content NOT YET confirmed (no destructor code reaches into it, and this round has not
    // scouted #create/#execute/the other unauthored virtuals that might).
    u8 mPad74c[0x1c];
};

/// @unofficial Thin subclass for BOTTOM_BG_FOR_CASTLE_LUDWIG -- adds NOTHING over
/// #daMiddleBgForCastleLudwig_c (confirmed: its own classInit's `li r3, 0x768` alloc size is
/// IDENTICAL to the base's own), overrides ONLY the destructor (vtable slot 18/offset 0x48;
/// every other slot in `lbl_2_data_30998` is byte-identical to `lbl_2_data_30C3C`, confirmed by
/// direct comparison of both full dumps).
class daBottomBgForCastleLudwig_c : public daMiddleBgForCastleLudwig_c {
public:
    virtual ~daBottomBgForCastleLudwig_c();
};
