#include <game/bases/d_actor.hpp>

// BRANCH -- a trivial dActor_c-derived placeholder actor. Coordinator-scoped:
// .text 0x676f0-0x677b0, 0xC0 bytes, NO .ctors entry, NO .bss, NO own .rodata --
// only one .data profile/vtable pair (g_profile_BRANCH at 0x176f8, vtable
// lbl_2_data_17704). Confirmed independently: python scout_unit.py d_basesNP
// 0x676f0 0x677b0 reports exactly this shape, AND a reverse relocation sweep
// (every (section,addr)->(section,addr) pair in the module) finds NO .text
// caller outside [0x676f0,0x677b0) branching in, and NO reader of 0x17704
// outside that same range -- sole owner both directions.
//
// Read directly from the vtable dump (target_auto_04_000132B0_data.txt,
// `lbl_2_data_17704`, 0xD4 bytes = 53 real mangled entries, the SAME size and
// shape as dummy_door's dActor_c-derived vtables): every slot resolves to a
// real, already-landed dActor_c-inherited name EXCEPT TWO --
//   * offset 0x08 (slot 2, dActor_c's own `create__7fBase_cFv` in the
//     unmodified base) is overridden here by `fn_2_67740`: `li r3, 0x2; blr`,
//     i.e. `return fBase_c::FAILED;` (PACK_RESULT_e -- confirmed from
//     include/game/framework/f_base.hpp: NOT_READY=0, SUCCEEDED=1, FAILED=2).
//   * offset 0x48 (slot 18, where `virtual ~fBase_c()` first enters the chain
//     -- same slot dummy_door's own destructor override used) is `fn_2_67750`,
//     the SAME one-slot `(this, shouldFree)` flag-argument destructor ABI as
//     dummy_door: `if (this) { dActor_c::~dActor_c(); if (shouldFree > 0)
//     fBase_c::__dl(this); } return this;` -- read directly off fn_2_67750's
//     own disasm, not assumed from the coordinator's general two-slot hint.
//
// `new daBranch_c()`'s own `li r3, 0x398` alloc size matches `sizeof(dActor_c)`
// exactly (the SAME constant dummy_door's own classInit used) -- daBranch_c
// adds no members of its own.
class daBranch_c : public dActor_c {
public:
    virtual int create();
    virtual ~daBranch_c();
};

// fn_2_676F0. classInit -- the STANDARD ACTOR_PROFILE expansion
// (`return new className();`). The target's extra `stw r3, 0x60(r31)` after
// `bl __ct__8dActor_cFv` is the implicit derived-class constructor's own
// vtable-pointer fixup, patching from dActor_c's own vtable (set by its own
// ctor) to daBranch_c's own -- same shape as dummy_door's classInit.
ACTOR_PROFILE(BRANCH, daBranch_c, 2);

// fn_2_67740. Always fails its own create step -- BRANCH is presumably driven
// entirely by some external owner (a tree/branch manager not in this unit)
// rather than completing its own create() pack.
int daBranch_c::create() {
    return FAILED;
}

// fn_2_67750. One-slot flag-argument destructor, empty body -- matches the
// target exactly via implicit chaining to dActor_c::~dActor_c() and, when
// shouldFree is set, fBase_c::__dl(this).
daBranch_c::~daBranch_c() {}
