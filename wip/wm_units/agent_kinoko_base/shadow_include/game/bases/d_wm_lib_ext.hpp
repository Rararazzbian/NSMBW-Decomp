#pragma once
#include <game/bases/d_wm_lib.hpp>

/// @unofficial PROPOSED ADDITIONS to game/bases/d_wm_lib.hpp, discovered while
/// authoring daWmKinokoBase_c::processCutsceneCommand(). None of these are
/// landed; they are declared here, in a shadow copy, per the ground rules.
/// Evidence for each is the target disassembly of fn_2_16B980 (this unit's
/// processCutsceneCommand) and fn_2_16B4E0/fn_2_16B620 (execute/createModel).
namespace dWmLib {
    /// @unofficial Called with no arguments, returns a value tested with
    /// `cmpwi r3, 0x0` / `beq`. Guards the "all courses complete" cutscene
    /// branches in processCutsceneCommand (mangled `IsAllComplete__6dWmLibFv`).
    bool IsAllComplete();

    /// @unofficial Called with no arguments, no return value used. Invoked
    /// once, inside the `cutsceneCommandId == 0x3c` branch, right before
    /// `getStartPointKinokoHouseKindNum()` (mangled `clearZoromeTime__6dWmLibFv`).
    void clearZoromeTime();

    /// @unofficial Takes a single `unsigned char`. Called with the low byte of
    /// `getStartPointKinokoHouseKindNum()`'s own return value
    /// (mangled `setStartPointKinokoHouseKindNum__6dWmLibFUc`).
    void setStartPointKinokoHouseKindNum(unsigned char kind);
}
