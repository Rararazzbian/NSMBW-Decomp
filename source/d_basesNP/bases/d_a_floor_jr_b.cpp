#include <game/bases/d_a_floor_jr_a.hpp>

// FLOOR_JR_B. Coordinator-scoped: .text 0x841e0-0x84290, 0xB0 bytes, NO
// .ctors entry, NO .bss, NO own .rodata -- one .data profile/vtable pair
// (g_profile_FLOOR_JR_B at 0x1cc30, vtable lbl_2_data_1CC3C at 0x1cc3c).
// Confirmed independently:
//   python wip/wm_units/scout_unit.py d_basesNP 0x841e0 0x84290
//   python wip/wm_units/ctors_map.py  d_basesNP FLOOR_JR_B
// Plus a reverse relocation sweep (own script): no `.text` caller outside
// [0x841e0,0x84290) branches in, and no reader of 0x1cc3c sits outside that
// range either -- sole owner both directions, exactly as scouted.
//
// NOT a dActor_c-direct unit, unlike dummy_door/BRANCH -- FLOOR_JR_B derives
// from FLOOR_JR_A (own class daFloorJrA_c; see
// shadow_include/game/bases/d_a_floor_jr_a.hpp for the full evidence and the
// resulting LANDING-ORDER DEPENDENCY that leaves this unit unable to link
// until FLOOR_JR_A itself is authored elsewhere). This shows up directly in
// the disassembly: FLOOR_JR_B's own constructor calls `fn_2_83660`, not
// `__ct__8dActor_cFv` -- an address inside FLOOR_JR_A's OWN 0x834ac-0x8405c
// span, confirmed by `scout_unit.py d_basesNP 0x834ac 0x8405c` (own
// `.ctors`/`.bss`/big `.rodata`, `profiles whose classInit falls inside:
// g_profile_FLOOR_JR_A`).
//
// FLOOR_JR_B itself adds nothing: `new daFloorJrB_c()`'s own `li r3, 0x8a8`
// alloc matches `sizeof(daFloorJrA_c)` exactly (see the shadow header for how
// that was confirmed against FLOOR_JR_A's OWN classInit, `fn_2_83630`).
//
// The vtable (`lbl_2_data_1CC3C`, 0xA3 slots / 0x28C bytes -- much bigger
// than dummy_door/BRANCH's plain dActor_c-derived 51 slots, because this
// class chain runs through dEn_c) carries exactly ONE slot that is THIS
// unit's own: offset 0x48 (the same "where ~fBase_c first enters the chain"
// slot dummy_door/BRANCH both used), `fn_2_84230`. Every other overridden
// slot (create/doDelete/execute/draw/the DieFall triple) is FLOOR_JR_A's own,
// inherited unmodified -- see the shadow header for the full slot-by-slot
// accounting. This IS the dummy_door CHILD/PARENT pattern (derived class
// overrides ONLY its own destructor, reusing every other inherited slot
// as-is), just with the shared base one level further away and, this time,
// not itself landed.
class daFloorJrB_c : public daFloorJrA_c {
public:
    virtual ~daFloorJrB_c();
};

// fn_2_841E0. classInit -- the standard ACTOR_PROFILE expansion
// (`return new className();`). The target's extra `stw r3, 0x60(r31)` after
// `bl fn_2_83660` is the implicit derived-class constructor's own
// vtable-pointer fixup, patching from FLOOR_JR_A's own vtable (set by ITS
// ctor) to FLOOR_JR_B's own -- same shape as dummy_door/BRANCH's classInit,
// just chaining through FLOOR_JR_A instead of dActor_c directly.
ACTOR_PROFILE(FLOOR_JR_B, daFloorJrB_c, 0);

// fn_2_84230. One-slot flag-argument destructor, empty body -- matches the
// target exactly via implicit chaining to daFloorJrA_c::~daFloorJrA_c()
// (fn_2_836E0) and, when shouldFree is set, fBase_c::__dl(this). Same ABI as
// dummy_door/BRANCH's own destructors, just chaining one level further.
daFloorJrB_c::~daFloorJrB_c() {}
