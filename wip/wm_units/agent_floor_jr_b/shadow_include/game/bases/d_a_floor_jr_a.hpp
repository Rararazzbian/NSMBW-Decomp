#pragma once

#include <game/bases/d_enemy.hpp>

// SHADOW COPY, not a real header. FLOOR_JR_B (this unit's own subject) derives
// from FLOOR_JR_A, a separate, much larger, NOT-YET-LANDED unit: `.text
// 0x834ac-0x8405c`, 0xBB0 bytes, its OWN `.ctors` entry (0x144 -> __sinit at
// 0x83de0) and its own `.bss`/`.rodata` -- confirmed with
// `python wip/wm_units/scout_unit.py d_basesNP 0x834ac 0x8405c`. This is
// entirely out of THIS unit's scope; only the minimum shape needed for
// FLOOR_JR_B's own two functions (classInit, destructor) to compile and for
// FLOOR_JR_B's own vtable object to lay out its INHERITED slots at the right
// positions is modelled here.
//
// Evidence for every piece of shape below is `lbl_2_data_1CC3C`
// (FLOOR_JR_B's own vtable, target_auto_04_000132B0_data.txt) and
// `fn_2_83630` (FLOOR_JR_A's own classInit, confirmed at
// target_auto_00_000834AC_text.txt:143 -- SAME `li r3, 0x8a8` /
// `bl fn_2_83660` shape as FLOOR_JR_B's own fn_2_841E0, just without the
// trailing vtable-pointer fixup a further-derived class adds):
//
//  * `sizeof(daFloorJrA_c) == 0x8a8`, read directly off `fn_2_83630`'s own
//    `li r3, 0x8a8` alloc -- the SAME constant FLOOR_JR_B's own classInit
//    allocates, confirming FLOOR_JR_B adds NO members of its own.
//  * FLOOR_JR_A's own constructor is `fn_2_83660`, destructor `fn_2_836E0`
//    (both confirmed by address: `fn_2_83660` is inside FLOOR_JR_A's own
//    0x834ac-0x8405c span, NOT this unit's).
//  * Of the 51 named slots in `lbl_2_data_1CC3C`, all but ONE (this unit's
//    OWN destructor override, `fn_2_84230`) already carry a real, landed
//    `dEn_c`-inherited name EXCEPT SEVEN, all addresses inside FLOOR_JR_A's
//    own span: `fn_2_83790` (slot 2, `create`), `fn_2_83C80` (slot 5,
//    `doDelete`), `fn_2_83910` (slot 8, `execute`), `fn_2_83C50` (slot 11,
//    `draw`), and a THREE-slot run (`fn_2_83D40`/`fn_2_83D70`/`fn_2_83D60`)
//    sitting exactly between the `DieFumi` and `DieBigFall` state triples --
//    the position `include/game/bases/d_enemy.hpp:167-169`'s own declaration
//    order assigns to `DieFall` ("Falling out of the screen"), the one
//    `STATE_VIRTUAL_FUNC_DECLARE(dEn_c, ...)` slot between them.
//  * The vtable does not end where dEn_c's own virtual chain ends
//    (`yoshifumiEffect`, `include/game/bases/d_enemy.hpp:220`, the LAST
//    virtual dEn_c itself declares). THREE MORE slots follow past that point
//    -- `fn_2_83810`/`fn_2_838C0`/`fn_2_83B00`, confirmed via
//    `check_vtable.py` (first draft, without these, read "SLOT COUNT DIFFERS
//    by -3" against a 161-slot target for a 158-slot draft). A vtable can
//    only grow past its base's own final slot by the DERIVED class declaring
//    BRAND NEW virtuals of its own (never seen in any dEn_c-derived unit
//    landed so far on this project), so these three are FLOOR_JR_A's OWN
//    novel virtuals, not overrides of anything dEn_c declares. Their real
//    names/signatures are unrecoverable from this unit's own scope (never
//    called by FLOOR_JR_B's classInit or destructor) -- stubbed below purely
//    to hold the SLOT COUNT, which `check_vtable.py` now reports CLEAN.
//
// LANDING-ORDER DEPENDENCY, stated plainly (same shape as the recorded
// WM_KILLERBULLET-on-WM_KILLER precedent): the seven method bodies below are
// declared, never defined, here. FLOOR_JR_B's own vtable object will carry
// external, unresolved relocations to their mangled names
// (`create__11daFloorJrA_cFv` etc, and `__ct__11daFloorJrA_cFv` /
// `__dt__11daFloorJrA_cFv` for the ctor/dtor) until FLOOR_JR_A is authored as
// a real TU defining a class of this exact name and these exact signatures.
// FLOOR_JR_B's OWN two functions compile and verify byte-identically against
// this shape regardless -- neither classInit nor the destructor CALLS any of
// these seven methods directly, they only need the class's SIZE and its
// ctor/dtor addresses -- but the REAL 5-binary link will not succeed until
// that dependency is resolved. Not routed through the `R_2_1_` free-function
// convention: that convention names an explicit `bl`/data reference WE write
// ourselves, and cannot rename a C++ member function's compiler-chosen
// mangled symbol.
class daFloorJrA_c : public dEn_c {
public:
    // Declared, deliberately NOT defined -- an in-class default (or omitted)
    // destructor would compile WEAK and get synthesised INLINE inside THIS
    // unit's own TU (confirmed by a first draft: it produced a spurious
    // `__dt__12daFloorJrA_cFv, weak` function). A weak symbol defined only in
    // an un-landed region gets PLACED at link time (the same failure mode
    // HANDOFF.md records for sandpillar's own un-landed-region blocker) --
    // this unit must not accidentally supply FLOOR_JR_A's own destructor
    // body. Out-of-line declaration-only forces it external instead.
    virtual ~daFloorJrA_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    virtual void initializeState_DieFall();
    virtual void executeState_DieFall();
    virtual void finalizeState_DieFall();

    // Three brand-new virtuals FLOOR_JR_A itself introduces, past dEn_c's own
    // final slot -- see the file-level note above. Placeholder names/
    // signatures; only the COUNT and RELATIVE ORDER are load-bearing here.
    virtual void unk_83810();
    virtual void unk_838C0();
    virtual void unk_83B00();

    // Padding only -- FLOOR_JR_A's own real members are entirely out of this
    // unit's scope. `dEn_c`'s own data ends at 0x524 (`sizeof(dEn_c)` is
    // 0x528 but its data size is 0x524 -- confirmed precedent:
    // include/game/bases/d_a_en_blockmain.hpp:176-179, "MWCC reuses a base
    // class's tail padding"), so a derived class's own first member can start
    // at 0x524 exactly. This pads to the confirmed total 0x8a8 without
    // claiming to know FLOOR_JR_A's actual field layout.
    u8 mUnknown524[0x8a8 - 0x524];
};
