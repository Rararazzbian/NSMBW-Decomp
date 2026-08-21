#include <game/bases/d_actor.hpp>

// DUMMY_DOOR_CHILD -- a trivial placeholder actor.
// .text 0x77af0-0x77ba0 (0xB0), .data 0x1aa08-0x1aae8 (0xE0: the 0xC profile
// followed by this class's own 0xD4 vtable, lbl_2_data_1AA14).
// NO .ctors entry, NO .bss, NO own .rodata.
//
// SPLIT FROM d_a_dummy_door.cpp. The CHILD and PARENT actors were originally
// authored as a single translation unit, but retail's .data lays them out as
// profile / vtable / profile / vtable, and MWCC emits a TU's vtables at the
// END of the object, always -- measured on three declaration orders, all of
// which produced profile / profile / vtable / vtable. An interleave of that
// shape is only reachable with TWO translation units, one per actor, which is
// also exactly what the .text split at 0x77ba0 and the two equal 0xE0 .data
// blocks say. Landing them as two units makes d_basesNP.rel verify.
//
// Confirmed directly from the vtable dumps (`lbl_2_data_1AA14`, 0xD4 bytes =
// 53 entries): EVERY slot resolves to a dActor_c-inherited function EXCEPT
// ONE -- vtable offset 0x48, `fn_2_77B40`. That offset is exactly where
// `virtual ~fBase_c()` (include/game/framework/f_base.hpp) first enters the
// vtable and gets overridden down the chain (`~dBaseActor_c()`,
// `~dActor_c()`) -- so it is the ONE-SLOT, flag-argument "destructor" this
// codebase's ABI uses, not two separate slots. `create()` is NOT overridden
// (slot 2 / offset 0x08 is `create__7fBase_cFv`, dActor_c's own unmodified
// base) -- so despite the PARENT/CHILD naming there is no `createChild` call
// to author here.
//
// The class adds no members of its own: `new daDummyDoorChild_c()`'s
// `li r3, 0x398` alloc size matches `sizeof(dActor_c)` (confirmed against the
// landed `daObjCenter_c` precedent in include/game/bases/d_a_en_door.hpp,
// whose first added field starts at raw offset 0x392 -- dActor_c itself ends
// there, and 0x398 is that size rounded up to 8-byte alignment).
//
// BINDING: an in-class default destructor compiles WEAK and gets deferred to
// the end of the translation unit, but the target's two functions are NOT
// deferred -- classInit then destructor, in plain declaration order, which is
// where a STRONG, out-of-line function lands. The destructor is therefore
// declared here AND DEFINED OUT OF LINE, to force GLOBAL binding.

/// @unofficial Placeholder actor for ::DUMMY_DOOR_CHILD. Reconstructed from
/// anonymous target symbols (fn_2_77AF0 classInit, fn_2_77B40 destructor) --
/// adds nothing over #dActor_c; every other vtable slot is inherited
/// unmodified (confirmed from `lbl_2_data_1AA14`'s own 53-entry dump).
class daDummyDoorChild_c : public dActor_c {
public:
    virtual ~daDummyDoorChild_c();
};

// fn_2_77AF0. classInit -- the STANDARD `ACTOR_PROFILE` expansion
// (`return new className();`), nothing custom: the target's extra
// `stw r3, 0x60(r31)` after the `bl __ct__8dActor_cFv` is the implicit
// compiler-generated derived-class constructor's own vtable-pointer fixup,
// inlined directly into `new` since #daDummyDoorChild_c never writes an
// explicit constructor.
ACTOR_PROFILE(DUMMY_DOOR_CHILD, daDummyDoorChild_c, 2);

// fn_2_77B40. The one-slot flag-argument destructor (`(this, shouldFree)`).
// An empty body is everything the target needs: it implicitly chains to
// `dActor_c::~dActor_c()` and, when `shouldFree` is set, `fBase_c::__dl(this)`.
daDummyDoorChild_c::~daDummyDoorChild_c() {}
