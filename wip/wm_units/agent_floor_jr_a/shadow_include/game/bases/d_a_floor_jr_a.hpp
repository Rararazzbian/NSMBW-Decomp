#pragma once

#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/bases/d_res_mng.hpp>

// FLOOR_JR_A. `.text 0x834ac-0x8405c`, 0xBB0 bytes, own `.ctors` (0x144 ->
// __sinit at 0x83de0, split into its own object auto_fn_2_83DE0_text.o), own
// `.bss`, 45-target `.rodata`. sizeof(daFloorJrA_c) == 0x8a8, confirmed from
// its own classInit (fn_2_83630)'s `li r3, 0x8a8` alloc, and independently
// from FLOOR_JR_B's own classInit allocating the SAME constant (FLOOR_JR_B
// adds no members).
//
// Real class name confirmed directly from data, not guessed: the STATE
// framework bakes fully-qualified names into its StateID objects, and this
// unit's own vtable-adjacent pool (`lbl_2_data_1C7E8`) ends with three ASCII
// strings -- "daFloorJrA_c::StateID_DemoWait", "daFloorJrA_c::StateID_Wait",
// "daFloorJrA_c::StateID_DieFall" -- naming the class outright.
//
// Member layout read directly off the constructor (fn_2_83660) and destructor
// (fn_2_836E0):
//   0x524  dHeapAllocator_c   (bl __ct__16dHeapAllocator_cFv)
//   0x540  int (unknown use)  (explicit `li r0,0; stw r0,0x540(r31)`)
//   0x544  m3d::mdl_c         (bl __ct__Q23m3d5mdl_cFv)
//   0x584  dBg_ctr_c          (bl __ct__9dBg_ctr_cFv; sizeof(dBg_ctr_c) ==
//                              0xe4 counted directly from its own landed
//                              header, so this ends at 0x668, not 0x678)
//   0x668  12 bytes unaccounted (no ctor call touches this span)
//   0x674  int (unknown use, written by fn_2_83780's own setter)
//   0x678  mEf::effect_c[2]   (bl __construct_array, elem size 0x114,
//                              ctor/dtor fn_2_41D20/fn_2_41D60 -- confirmed
//                              by reading THEIR OWN vtable,
//                              `lbl_2_data_11B40`, which is `mEf::effect_c`'s
//                              real, already-landed vtable
//                              (`include/game/mLib/m_effect.hpp`) -- ends at
//                              0x8a0)
//   0x8a0  8 bytes trailing padding to the confirmed total 0x8a8
class daFloorJrA_c : public dEn_c {
public:
    daFloorJrA_c();
    virtual ~daFloorJrA_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    STATE_FUNC_DECLARE(daFloorJrA_c, DemoWait);
    STATE_FUNC_DECLARE(daFloorJrA_c, Wait);

    // NOT STATE_VIRTUAL_FUNC_DECLARE: dEn_c already declares DieFall's three
    // state methods virtual (STATE_VIRTUAL_FUNC_DECLARE(dEn_c, DieFall) in
    // d_enemy.hpp), so overriding them here needs no re-statement of the
    // sFStateVirtualID_c/baseID_DieFall<T> base-lookup machinery -- that
    // machinery is for a DERIVED class that wants to CHAIN to an ancestor's
    // existing StateID by name. This class instead builds its OWN fresh,
    // ordinary sFStateID_c<daFloorJrA_c> (same shape as StateID_DemoWait/
    // StateID_Wait, sharing their own vtable and destructor in .data/.text),
    // confirmed directly from __sinit's target disassembly: all three state
    // ctor blocks are byte-for-byte the SAME shape (9-word copy, plain
    // __ct__10sStateID_cFPCc, vtable patch, __register_global_object) --
    // none of the three calls a baseID_DieFall<T> lookup, which
    // STATE_VIRTUAL_DEFINE's expansion would require if used here.
    virtual void initializeState_DieFall();
    virtual void executeState_DieFall();
    virtual void finalizeState_DieFall();
    static sFStateID_c<daFloorJrA_c> StateID_DieFall;

    // Three brand-new virtuals past dEn_c's own final slot
    // (`yoshifumiEffect`) -- confirmed by `check_vtable.py` SLOT COUNT
    // against FLOOR_JR_B's own vtable during that unit's authoring. Real
    // names/signatures still placeholder; this unit's own body-authoring
    // pass is what should replace them once each is read.
    virtual void createMdl();
    virtual int resetToBasePos();
    virtual void unk_83B00();

    void setUnk_674_348(int val674, u8 val348);  ///< fn_2_83780
    void setupBgCtr();                            ///< fn_2_83970
    void playCrumbleEffects();                    ///< fn_2_83A10
    void unk_83A90();                             ///< fn_2_83A90

    dHeapAllocator_c mHeapAllocator;      // 0x524
    nw4r::g3d::ResFile mResFile;           // 0x540
    m3d::mdl_c mModel;                     // 0x544
    dBg_ctr_c mBgCtr;                      // 0x584
    mVec3_c mBasePos;                      // 0x668
    int m_674;                             // 0x674
    mEf::effect_c mEffects[2];             // 0x678
    u8 mUnknown8A0[0x8a8 - 0x8a0];         // 0x8a0
};
