#include <game/bases/d_actor.hpp>

// DUMMY_DOOR_CHILD and DUMMY_DOOR_PARENT -- two trivial placeholder actors sharing this one
// translation unit (coordinator-scoped: .text 0x77af0-0x77c50, 0x160 bytes, NO .ctors entry,
// NO .bss, NO own .rodata -- only two .data profile/vtable pairs, lbl_2_data_1AA14 for CHILD
// and lbl_2_data_1AAF4 for PARENT).
//
// Confirmed directly from the vtable dumps (target_auto_04_000132B0_data.txt, `lbl_2_data_1AA14`
// / `lbl_2_data_1AAF4`, both 0xD4 bytes = 53 real mangled entries): EVERY slot resolves to a
// dActor_c-inherited function EXCEPT ONE -- vtable offset 0x48 (slot 18 by the coordinator's own
// `(offset-0x08)/4+2` formula), which is `fn_2_77B40` for CHILD and `fn_2_77BF0` for PARENT. That
// offset is exactly where `virtual ~fBase_c()` (include/game/framework/f_base.hpp) first enters
// the vtable and gets overridden down the chain (`~dBaseActor_c()`, `~dActor_c()`) -- so it is
// the ONE-SLOT, flag-argument "destructor" this codebase's ABI uses (see fn_2_77B40's own
// `(this, shouldFree)` signature below), not two separate slots. Neither class overrides
// `create()` (confirmed: vtable slot 2/offset 0x08 is `create__7fBase_cFv`, dActor_c's own
// unmodified base) -- so despite the PARENT/CHILD naming, there is no `createChild` call to
// author anywhere in this unit; that was the coordinator's own general heuristic, checked here
// and found not to apply to this specific pair (no contradiction of the coordinator's BOUNDS,
// just of the createChild expectation).
//
// Neither class adds a single member of its own: `new className()`'s own `li r3, 0x398` alloc
// size matches `sizeof(dActor_c)` (confirmed from a real landed precedent, `daObjCenter_c` in
// include/game/bases/d_a_en_door.hpp, whose own first added field starts at raw offset 0x392 --
// `dActor_c` itself ends there, and 0x398 is that same size rounded up to 8-byte alignment, not
// extra fields). So each class is exactly: an empty body plus one out-of-line destructor.
//
// BINDING, not just bytes (the coordinator's own explicit warning): an in-class default
// destructor compiles WEAK and gets deferred to the end of the translation unit (LIFO), but the
// target's own four functions are NOT deferred -- they appear in plain interleaved
// [CHILD-classInit, CHILD-dtor, PARENT-classInit, PARENT-dtor] address order, matching where a
// STRONG, out-of-line, in-declaration-order function would land. Both destructors are therefore
// declared here AND DEFINED OUT OF LINE (not merely default), to force GLOBAL binding.

/// @unofficial Placeholder actor for ::DUMMY_DOOR_PARENT. Identical shape to
/// #daDummyDoorChild_c below, its own separate class/vtable/profile (confirmed from
/// `lbl_2_data_1AAF4`, byte-identical layout to `lbl_2_data_1AA14` apart from the one destructor
/// slot). EXPERIMENT: declared before CHILD -- see the `.data` vtable-order note above the
/// function definitions below.
class daDummyDoorParent_c : public dActor_c {
public:
    virtual ~daDummyDoorParent_c();
};

/// @unofficial Placeholder actor for ::DUMMY_DOOR_CHILD. Reconstructed from anonymous target
/// symbols (fn_2_77AF0 classInit, fn_2_77B40 destructor) -- adds nothing over #dActor_c; every
/// other vtable slot is inherited unmodified (confirmed directly from `lbl_2_data_1AA14`'s own
/// 53-entry dump).
class daDummyDoorChild_c : public dActor_c {
public:
    virtual ~daDummyDoorChild_c();
};

// fn_2_77AF0. classInit for DUMMY_DOOR_CHILD -- the STANDARD `ACTOR_PROFILE` expansion
// (`return new className();`), nothing custom: the target's own extra `stw r3, 0x60(r31)` after
// the `bl __ct__8dActor_cFv` is the IMPLICIT compiler-generated derived-class constructor's own
// vtable-pointer fixup (patches from #dActor_c's own vtable, set by its ctor, to this class's
// own), inlined directly into `new` since #daDummyDoorChild_c never writes an explicit ctor.
ACTOR_PROFILE(DUMMY_DOOR_CHILD, daDummyDoorChild_c, 2);

// fn_2_77B40. The one-slot flag-argument destructor (`(this, shouldFree)` -- WEAK/inline would
// not match target's GLOBAL binding/address position, see the file-level note above). An empty
// body is everything the target needs: it implicitly chains to `dActor_c::~dActor_c()` and, when
// `shouldFree` is set, `fBase_c::__dl(this)` -- both compiler-generated from the empty body,
// matching the target's own shape exactly.
daDummyDoorChild_c::~daDummyDoorChild_c() {}

// fn_2_77BA0. classInit for DUMMY_DOOR_PARENT -- same shape as CHILD's own, vtable
// `lbl_2_data_1AAF4`.
ACTOR_PROFILE(DUMMY_DOOR_PARENT, daDummyDoorParent_c, 0);

// fn_2_77BF0. PARENT's own destructor -- same shape as CHILD's own.
daDummyDoorParent_c::~daDummyDoorParent_c() {}
