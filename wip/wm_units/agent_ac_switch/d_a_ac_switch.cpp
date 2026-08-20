#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_actor_state.hpp>

// AC_FLAGON / AC_4SWICHAND / AC_4SWICHOR / AC_RANDSWICH / AC_CHNGESWICH / AC_IFSWICH /
// AC_RNSWICH -- SEVEN profiles, not six. The coordinator's own dispatch scoped this unit at
// .text 0x7D400-0x7D5E0 (six classInits); scout_unit.py at that range is internally clean
// (single .data target 0x1BC30, no .bss/.rodata leakage, no .ctors) but a 7th classInit sits
// immediately adjacent: g_profile_AC_RNSWICH's own classInit resolves to fn_2_7D5E0 (confirmed
// via profile_map.py's own relocation walk), 0x4C bytes, ending at 0x7D62C with the usual 4-byte
// gap to 0x7D630 -- exactly the same shape/spacing as the other six (each is 0x4C + a 4-byte
// gap = 0x50 stride). Re-scouting 0x7D400-0x7D630 stays clean (still exactly one .data target,
// 0x1BC30, still no .ctors) while 0x7D400-0x7D650 (one stride further) picks up a `.bss` target
// and a `sec11` target that belong to fn_2_7D630 (the huge real member function immediately
// after), not to any classInit -- so 0x7D630 is the tight upper bound, not 0x7D5E0.
// Independent confirmation, unrelated to address arithmetic: include/game/bases/d_profile.hpp
// declares `g_profile_AC_FLAGON` through `g_profile_AC_RNSWICH` as one unbroken run of seven
// `extern fProfile::fActorProfile_c` lines (between EN_STAR_COIN_VOLT and EN_BKBLOCK) -- the
// header's own author already grouped these seven, not six.
//
// ONE class, not seven. Read directly off the target's OWN `.data` vtable object
// (lbl_2_data_1BC30, target_auto_04_000132B0_data.txt:10282-10465, dumped fresh this round --
// not reused from any earlier session):
//   - All 14 relocations into `.data` from this .text span (2 per classInit: `lis`/`addi`
//     @ha/@l) target the exact same address, 0x1BC30 -- one vtable, not seven.
//   - The seven `g_profile_AC_*` structs immediately preceding that vtable (0x1BBD8-0x1BC30,
//     dumped fresh alongside it) each store a DIFFERENT classInit pointer (fn_2_7D400,
//     fn_2_7D450, fn_2_7D4A0, fn_2_7D4F0, fn_2_7D540, fn_2_7D590, fn_2_7D5E0) but the SAME
//     properties word (0x00000000) and sequential order words (0x0040003E .. 0x00460044,
//     matching auto-incrementing `fProfile::AC_*`/`fProfile::DRAW_ORDER::AC_*` enum values) --
//     nothing in the profile struct itself carries a per-profile "which class" tag.
//   - The vtable's own trailing string-literal table -- decoded directly from the raw `.4byte`
//     words, not inferred -- reads (7 strings, all one class):
//       "daFlagObj_c::StateID_NonMove"        "daFlagObj_c::StateID_Swich4andMove"
//       "daFlagObj_c::StateID_Swich4orMove"    "daFlagObj_c::StateID_RandSwichMove"
//       "daFlagObj_c::StateID_ChngeSwichMove"  "daFlagObj_c::StateID_IfSwichMove"
//       "daFlagObj_c::StateID_RenzokuOnMove"
//     Seven `ClassName::StateID_Name` strings under ONE class name, and they line up 1:1 with
//     the seven profiles above (NonMove/FLAGON, Swich4and/4SWICHAND, Swich4or/4SWICHOR,
//     RandSwich/RANDSWICH, ChngeSwich/CHNGESWICH, IfSwich/IFSWICH, RenzokuOn/RNSWICH).
//   - EMPIRICAL cross-check, same method the sibling unit used: a probe TU
//     (`wip/wm_units/agent_ac_switch/probe.cpp`) that invokes the standard `ACTOR_PROFILE`
//     macro seven times for one shared class name fails to COMPILE at all --
//     `void *className##_classInit()` (f_profile.hpp:16) is keyed on the CLASS name, not the
//     profile name, so seven invocations for the same class collide:
//     "(10333) object 'daProbe_c_classInit()' redefined". That rules out the naive macro and
//     is why this draft below hand-expands `CUSTOM_ACTOR_PROFILE`'s body per profile instead
//     (matching the seven genuinely distinct classInit addresses the target has).
//     A second probe (`probe2.cpp`, manually-named classInit functions, matching this draft's
//     own shape) compiles the classInit bodies BYTE-SHAPE-IDENTICAL to the target (differing
//     only in the allocation-size immediate, expected -- the probe's placeholder class has no
//     added members) and, critically, resolves the vtable through an ORDINARY EXTERNAL
//     relocation (`__vt__11daFlagObj_c@ha`/`@l`) rather than emitting a local definition --
//     because create()/execute()/the destructor are DECLARED, not DEFINED, in this TU. That
//     is the expected, standard consequence of this class's key function living elsewhere.
//
// STRUCTURAL FINDING, not a detail: daFlagObj_c overrides exactly THREE of fBase_c's virtuals
// (cross-checked directly against the declaration order in include/game/framework/f_base.hpp)
// -- create() (vtable slot 2/offset 0x08 -> fn_2_7D630, 0x4D8 bytes), execute() (slot 8/offset
// 0x20 -> fn_2_7DB10), and ~daFlagObj_c() (slot 18/offset 0x48 -> fn_2_7EC90, the same one-slot
// flag-argument shape d_a_dummy_door.cpp already established for this codebase's ABI) -- plus
// seven STATE_FUNC_DECLARE-shaped (non-virtual) states whose init/execute/finalize triples run
// fn_2_7DD10 through fn_2_7E9E0. All of that lives from 0x7D630 to at least 0x7EC90+ (>0x2C90,
// >11KB) -- FAR outside this unit's own 0x7D400-0x7D630 classInit span, and it is that TU,
// wherever it lands, that owns lbl_2_data_1BC30 (a TU emits a class's vtable only where its key
// function -- ordinarily the first non-inline virtual, here effectively any of the three real
// overrides -- is DEFINED, not merely declared; confirmed empirically above via probe2.cpp's own
// external, unresolved reference). daFlagObj_c is not landed anywhere in source/ or include/
// today (grep -r "daFlagObj_c" source/ include/ is empty) -- so this classInit-only unit cannot
// be independently LINKED yet, only independently instruction-verified: it will need to land
// together with, or after, whichever unit implements daFlagObj_c's real body.
//
// sizeof(daFlagObj_c) == 0x3f8, read directly off every classInit's own `li r3, 0x3f8`.
// sizeof(dActorState_c) == 0x3d0, confirmed empirically (probe2.cpp, a same-shape class adding
// no members allocates 0x3d0). The 0x28-byte remainder is daFlagObj_c's own member data --
// completely unanalysed (its consumers, create()/execute()/the seven states, are all outside
// this unit) -- so it is reserved here as opaque padding, sized only, not laid out.
//
// classInit itself needs nothing about that state machine: it is the standard
// `CUSTOM_ACTOR_PROFILE` body, `return new className();`, hand-expanded seven times because
// the macro's own generated name collides across seven invocations of ONE class (see probe.cpp
// above). `daFlagObj_c` never writes an explicit constructor, so `new` compiles to
// `operator new` + the compiler's OWN implicit derived-default-ctor, inlined here exactly as
// d_a_dummy_door.cpp's own note describes: `dActorState_c`'s ctor runs, then the vtable pointer
// is patched from dActorState_c's own vtable to daFlagObj_c's.
class daFlagObj_c : public dActorState_c {
public:
    virtual int create();
    virtual int execute();
    virtual ~daFlagObj_c();

    /// @unofficial Opaque, size-only. 0x3f8 (classInit's own `li r3, 0x3f8`) minus
    /// sizeof(dActorState_c) (0x3d0, confirmed empirically) == 0x28. Real layout is out of
    /// scope for this unit -- every read/write site lives in create()/execute()/the seven
    /// states, all far outside 0x7D400-0x7D630.
    u8 mUnknown3D0[0x28];
};

// fn_2_7D400. classInit for AC_FLAGON.
static void *classInit_AC_FLAGON() { return new daFlagObj_c(); }
// fn_2_7D450. classInit for AC_4SWICHAND.
static void *classInit_AC_4SWICHAND() { return new daFlagObj_c(); }
// fn_2_7D4A0. classInit for AC_4SWICHOR.
static void *classInit_AC_4SWICHOR() { return new daFlagObj_c(); }
// fn_2_7D4F0. classInit for AC_RANDSWICH.
static void *classInit_AC_RANDSWICH() { return new daFlagObj_c(); }
// fn_2_7D540. classInit for AC_CHNGESWICH.
static void *classInit_AC_CHNGESWICH() { return new daFlagObj_c(); }
// fn_2_7D590. classInit for AC_IFSWICH.
static void *classInit_AC_IFSWICH() { return new daFlagObj_c(); }
// fn_2_7D5E0. classInit for AC_RNSWICH.
static void *classInit_AC_RNSWICH() { return new daFlagObj_c(); }

// g_profile_AC_* -- all `properties` 0, all `mpClassInit` distinct (confirmed against the
// fresh .data dump at 0x1BBD8-0x1BC30). AC_RNSWICH's own struct dumps 0x10 bytes, not 0xC like
// its six siblings; that extra 4 bytes lands exactly at 0x1BC30, the vtable's own required
// 16-byte alignment (0x1BC20+0xC=0x1BC2C, unaligned; +0x10=0x1BC30, aligned) -- read as
// compiler-inserted alignment padding on the LAST object before the vtable, not a real extra
// field, so the struct itself is declared identically to the other six below.
fProfile::fActorProfile_c g_profile_AC_FLAGON = {
    &classInit_AC_FLAGON, fProfile::AC_FLAGON, fProfile::DRAW_ORDER::AC_FLAGON, 0
};
fProfile::fActorProfile_c g_profile_AC_4SWICHAND = {
    &classInit_AC_4SWICHAND, fProfile::AC_4SWICHAND, fProfile::DRAW_ORDER::AC_4SWICHAND, 0
};
fProfile::fActorProfile_c g_profile_AC_4SWICHOR = {
    &classInit_AC_4SWICHOR, fProfile::AC_4SWICHOR, fProfile::DRAW_ORDER::AC_4SWICHOR, 0
};
fProfile::fActorProfile_c g_profile_AC_RANDSWICH = {
    &classInit_AC_RANDSWICH, fProfile::AC_RANDSWICH, fProfile::DRAW_ORDER::AC_RANDSWICH, 0
};
fProfile::fActorProfile_c g_profile_AC_CHNGESWICH = {
    &classInit_AC_CHNGESWICH, fProfile::AC_CHNGESWICH, fProfile::DRAW_ORDER::AC_CHNGESWICH, 0
};
fProfile::fActorProfile_c g_profile_AC_IFSWICH = {
    &classInit_AC_IFSWICH, fProfile::AC_IFSWICH, fProfile::DRAW_ORDER::AC_IFSWICH, 0
};
fProfile::fActorProfile_c g_profile_AC_RNSWICH = {
    &classInit_AC_RNSWICH, fProfile::AC_RNSWICH, fProfile::DRAW_ORDER::AC_RNSWICH, 0
};
