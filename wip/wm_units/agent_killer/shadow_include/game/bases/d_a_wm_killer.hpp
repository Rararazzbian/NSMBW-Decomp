#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

/**
* @brief The actor for the World 7 boss ship's cannon-fired killer on the World Map.
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name and every
* member name are inferred from codegen evidence and the fProfile::WM_KILLER slot this unit's
* profile object is placed at, not from any mangled name.
* @ingroup bases
*/
class daWmKiller_c : public dWmObjActor_c {
public:
    daWmKiller_c(); ///< @copydoc dWmObjActor_c::dWmObjActor_c
    ~daWmKiller_c(); ///< @copydoc dWmObjActor_c::~dWmObjActor_c

    // Vtable slots confirmed via check_vtable.py against lbl_2_data_452E0, not guessed from shape:
    // slot 2=create(fn_2_167AA0), 5=doDelete(fn_2_167D20), 8=execute(fn_2_167B10),
    // 11=draw(fn_2_167C40), 24=processCutsceneCommand(fn_2_168060).
    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    // Offset 0, width 8 (the low byte of mParam): read by #unk_167F20, then repositioned to
    // bits 8-15 (`<< 8`) so it does not collide with the 0-9 loop counter OR'd into the same
    // word -- confirmed from the target's single fused `clrlslwi r0, r0, 24, 8` instruction,
    // which is (u8)mParam << 8 in one op (clear-left-24 keeps the low byte, then shift-left-8).
    ACTOR_PARAM_CONFIG(SpawnKind, 0, 8);

    // Offset 16, width 16 (the whole upper half of mParam): selects between the two
    // createModel() variants via a plain `mParam >> 16` with no mask (size fills to bit 31,
    // so MWCC omits the andi). Confirmed content-wise: Kind==0 loads "cobKiller"/"cobKillerShot",
    // Kind==1 loads "cobRotary"/"cobRotaryShot" (real strings read from this unit's own .data,
    // lbl_2_data_45230, at file offset 0x1d0c00+addr).
    ACTOR_PARAM_CONFIG(Kind, 16, 16);

    // Called from #processCutsceneCommand when isFirstFrame && cutsceneCommandId==0x38, with a
    // literal `true` second argument; return value stored into #m_208. NOT YET AUTHORED --
    // placeholder signature (bool-returning, bool-taking) inferred from the call site only.
    bool unk_1684A0(bool);

    // #create's five sub-calls, in target address/call order. NOT YET AUTHORED -- placeholder
    // names/signatures inferred only from call position and argument count (all take just
    // `this`, matching a no-arg member function).
    void createModel(); ///< @unofficial fn_2_167D30, 0x1ec bytes, called first. Confirmed by
                         ///< content (shares the ghost/sandpillar createModel() shape exactly),
                         ///< not by position.
    void unk_167F20(); ///< @unofficial fn_2_167F20, 0x8c bytes -- a loop constructing 10
                         ///< WM_KILLERBULLET (0x276) children into an array at this+0x214.
    void unk_167C70(); ///< @unofficial fn_2_167C70, 0xa8 bytes -- not a vtable slot (confirmed),
                         ///< an ordinary member function.
    void unk_167FB0(); ///< @unofficial fn_2_167FB0, 0xb0 bytes, called last from #create.

    // The remaining 8 undeclared functions, in target address order, sizes noted for the next
    // round. NOT YET AUTHORED -- placeholder void/no-arg signatures, not measured. Positioned
    // here purely to keep definition order correct once stubbed in the .cpp (see AGENT_CONTEXT.md
    // "Function DEFINITION ORDER is part of the object").
    void unk_1680F0(); ///< @unofficial fn_2_1680F0, 0xc4 bytes.
    void unk_1681C0(); ///< @unofficial fn_2_1681C0, 0x94 bytes.
    void unk_168260(); ///< @unofficial fn_2_168260, 0x44 bytes.
    void unk_1682B0(); ///< @unofficial fn_2_1682B0, 0x20 bytes.
    void unk_1682D0(); ///< @unofficial fn_2_1682D0, 0x1c bytes.
    void unk_1682F0(); ///< @unofficial fn_2_1682F0, 0x84 bytes.
    void unk_168380(); ///< @unofficial fn_2_168380, 0x114 bytes.
    void unk_168590(); ///< @unofficial fn_2_168590, 0x9c bytes.

    u32 mUnk188; ///< @unused @unofficial offset 0x188, identical position to every other landed WM actor's mUnk188.
    dHeapAllocator_c mAllocator; ///< The allocator. @unofficial offset 0x18c.
    m3d::smdl_c mModel; ///< The model. @unofficial offset 0x1a8 -- NOTE: unlike castle/WM_START,
                         ///< there is no separate `nw4r::g3d::ResFile mResFile` member zeroed
                         ///< before this in the constructor; mModel sits directly at 0x1a8.
    m3d::anmChr_c mAnmChr; ///< The model animation. @unofficial offset 0x1b4 -- confirmed by the
                            ///< constructor's vtable-patch sequence (banm_c's own vtable written
                            ///< first via the `fanm_c` base constructor call, then anmChr_c's own
                            ///< vtable patched in), matching the same base-then-derived pattern
                            ///< already established for castle/WM_START's anim members.
    // Total size 0x23c (measured, classInit's `li r3, 0x23c` operand; also cross-checked via
    // sizeof(m3d::anmChr_c) == 0x38, probed with a `char[sizeof(X)]` compile).
    float mMotion[3]; ///< @unofficial offset 0x1ec -- read by #unk_167F20 as the spawn position
                        ///< for its constructed children. Name copied from WM_ITEM's own
                        ///< identically-positioned/shaped member (see agent_item/MAPPING.md).
    u8 mPad_1f8[0x10]; ///< @unofficial offset 0x1f8, size 0x10 -- placeholder padding up to the
                        ///< flag byte at 0x208, NOT individually verified. (Was originally
                        ///< mis-sized 0x1c, which pushed #mChildren to 0x220 instead of 0x214 --
                        ///< caught by unk_167F20's `stw r3, 0x214(r31)` target instruction.)
    bool m_208; ///< @unofficial offset 0x208. Zeroed by the constructor.
    u8 mPad_209[0xb]; ///< @unofficial offset 0x209, size 0xb -- placeholder padding, NOT
                        ///< individually verified.
    dWmActor_c *mChildren[10]; ///< @unofficial offset 0x214, size 0x28. Filled by #unk_167F20's
                                 ///< loop constructing 10 WM_KILLERBULLET (profile 0x276)
                                 ///< children, confirmed by the loop's `stw r3, 0x214(r31)` with
                                 ///< r31 advancing by 4 each of its 10 iterations.
};
