#include <game/bases/d_actor.hpp>

// DUMMY_DOOR_PARENT -- a trivial placeholder actor, the sibling of
// DUMMY_DOOR_CHILD.
// .text 0x77ba0-0x77c50 (0xB0), .data 0x1aae8-0x1abc8 (0xE0: the 0xC profile
// followed by this class's own 0xD4 vtable, lbl_2_data_1AAF4).
// NO .ctors entry, NO .bss, NO own .rodata.
//
// SPLIT FROM d_a_dummy_door.cpp -- see the note in d_a_dummy_door_child.cpp
// for the evidence that these two actors are separate translation units.
//
// `lbl_2_data_1AAF4` is byte-identical in layout to CHILD's `lbl_2_data_1AA14`
// apart from the one destructor slot (offset 0x48 -> `fn_2_77BF0` here).
// Same shape as CHILD in every other respect: no added members
// (`li r3, 0x398`), no `create()` override, one out-of-line destructor with an
// empty body forced to GLOBAL binding by being defined out of line.

/// @unofficial Placeholder actor for ::DUMMY_DOOR_PARENT. Identical shape to
/// #daDummyDoorChild_c, with its own separate class/vtable/profile.
class daDummyDoorParent_c : public dActor_c {
public:
    virtual ~daDummyDoorParent_c();
};

// fn_2_77BA0. classInit for DUMMY_DOOR_PARENT -- same shape as CHILD's own,
// vtable `lbl_2_data_1AAF4`.
ACTOR_PROFILE(DUMMY_DOOR_PARENT, daDummyDoorParent_c, 0);

// fn_2_77BF0. PARENT's own destructor -- same shape as CHILD's own.
daDummyDoorParent_c::~daDummyDoorParent_c() {}
