# `m_pad.cpp` — shared brief

Authoritative fact sheet. Everything here is derived from the retail symbol map,
the target disassembly, and `slices/wiimj2d.json`, and cross-checked by the lead.
Where something is unverified it says so. **Report a contradiction rather than
reconciling it.**

Target disassembly: `scratch/gemini_round8/auto_03_8016F330_text.o.txt`
(functions 1–12) and `scratch/gemini_round8/auto_03_8016F808_text.o.txt`
(the compiler-generated tail). Read both.

## Bounds — verified, use as given

| section | virtual | offset | size |
|---|---|---|---|
| `.text` | `0x8016F330`–`0x8016F880` | `0x168bb0-0x169100` | `0x550` |
| `.ctors` | — | `0x21c-0x220` | `0x4` |
| `.bss` | `0x80377F88`–`0x803780C8` | `0x26608-0x26748` | `0x140` |
| `.sbss` | `0x8042A740`–`0x8042A760` | `0x8a0-0x8c0` | `0x20` |

No `.rodata`, no `.data`, no `.sdata`. One `.sdata2` constant is referenced but
**belongs to `m_mtx.cpp`**, not to us — see the float note below.

Checked for overlap against every slice: none. `.ctors` and `.bss` are adjacent
on **both** sides; `.text` is adjacent below to `m_mtx.cpp`.

**Two corrections to earlier pre-flight material, both mine to state:**

1. An earlier round recorded `m_pad.cpp` as containing **32 `mPrint::MyPrintBase`
   template methods** and a `.data` object `__vt__Q24mTex8edit4b_c`. **That is
   wrong.** `__ct__Q26mPrint14MyPrintBase<c>Fv` at `0x8016F880` is the first
   function of a *different* TU, and it is exactly where our `.text` ends. This
   unit has **16 functions and no `.data` at all.**
2. The upper `.text` bound is **not** adjacent to `m_vec.cpp` — there is an
   undecompiled `mPrint` TU between us. The bound is still right; it is fixed by
   where `mPrint` begins, not by adjacency.

## The 16 functions

| # | address | size | symbol |
|---|---|---|---|
| 1 | `0x8016F330` | `0x30` | `create__4mPadFv` |
| 2 | `0x8016F360` | `0x1E4` | `beginPad__4mPadFv` |
| 3 | `0x8016F550` | `0x14` | `endPad__4mPadFv` |
| 4 | `0x8016F570` | `0x24` | `setCurrentChannel__4mPadFQ24mPad4CH_e` |
| 5 | `0x8016F5A0` | `0x30` | `getBatteryLevel_ch__4mPadFQ24mPad4CH_e` |
| 6 | `0x8016F5D0` | `0x68` | `setWPADInfo__4mPadFQ24mPad4CH_eRC8WPADInfo` |
| 7 | `0x8016F640` | `0x44` | `clearWPADInfo__4mPadFQ24mPad4CH_e` |
| 8 | `0x8016F690` | `0x3C` | `initWPADInfo__4mPadFv` |
| 9 | `0x8016F6D0` | `0x3C` | `getWPADInfoCb` |
| 10 | `0x8016F710` | `0x64` | `getWPADInfoAsync__4mPadFQ24mPad4CH_e` |
| 11 | `0x8016F780` | `0x14` | `setGetWPADInfoInterval__4mPadFUl` |
| 12 | `0x8016F7A0` | `0x8` | `getGetWPADInfoInterval__4mPadFv` |
| 13 | `0x8016F7B0` | `0x58` | `__sinit_\m_pad_cpp` |
| 14 | `0x8016F810` | `0x4` | `__ct__Q24mPad19PadAdditionalData_tFv` |
| 15 | `0x8016F820` | `0x40` | `__dt__Q24mPad19PadAdditionalData_tFv` |
| 16 | `0x8016F860` | `0x1C` | `__arraydtor$13953` |

Numbers 13–16 are compiler-generated: nobody authors them directly. They appear
if and only if the declarations are right, which makes them a **free correctness
check on the header** rather than work.

`mPad` is a **namespace**, not a class — `__4mPad` is a 4-character namespace
name. `PadAdditionalData_t` is a struct inside it (`Q24mPad19PadAdditionalData_t`).

## Data inventory — complete, every byte accounted for

`.sbss`, `0x20`, exactly filling the claim:

| address | size | symbol | note |
|---|---|---|---|
| `0x8042A740` | `0x4` | `g_padMg__4mPad` | `EGG::CoreControllerMgr *` |
| `0x8042A744` | `0x4` | `g_currentCoreID__4mPad` | |
| `0x8042A748` | `0x4` | `g_currentCore__4mPad` | `EGG::CoreController *` |
| `0x8042A74C` | `0x4` | `g_IsConnected__4mPad` | map says `data:byte`, size 4 — **a `bool[4]`, not a `u32`** |
| `0x8042A750` | `0x4` | `g_PadFrame__4mPad` | |
| `0x8042A754` | `0x4` | `s_WPADInfoAvailable__4mPad` | no data type in map; likely `bool[4]` |
| `0x8042A758` | `0x4` | `s_GetWPADInfoInterval__4mPad` | `setGetWPADInfoInterval` takes `Ul` |
| `0x8042A75C` | `0x4` | `s_GetWPADInfoCount__4mPad` | |

`.bss`, `0x140`:

| address | size | symbol | note |
|---|---|---|---|
| `0x80377F88` | `0x10` | `g_core__4mPad` | `EGG::CoreController *[4]` |
| `0x80377F98` | `0x10` | **UNCLAIMED** | see below |
| `0x80377FA8` | `0x60` | `g_PadAdditionalData__4mPad` | `PadAdditionalData_t[4]`, so **`sizeof == 0x18`** |
| `0x80378008` | `0x60` | `s_WPADInfo__4mPad` | `WPADInfo[4]`, SDK `WPADInfo` is `0x18` |
| `0x80378068` | `0x60` | `s_WPADInfoTmp__4mPad` | `WPADInfo[4]`, ends exactly on the high bound |

**The `0x10` hole at `0x80377F98` is a finding, not noise.** No symbol claims it,
and the section bounds are hard. An object nobody references still has to be
emitted or the section comes up short. The most likely shape is a second
four-element pointer or `u32` array declared next to `g_core`. Whoever gets there
first should identify it from the disassembly and say so; if nothing references
it, say that too, because then it is a declaration whose only job is to occupy
space and it still has to exist.

Remember: **`.bss` object alignment follows SIZE, not type alignment.** A `0x60`
object gets 8-aligned placement regardless of its members.

## The float, and why it is not ours

`beginPad` loads `@14502_8042E010` via `@sda21`. That constant lives in
`m_mtx.cpp`'s `.sdata2` range and **we must not emit it**. If your draft emits a
`.sdata2` constant, the declaration that produced it is wrong. This is the same
class of trap as a `const` file-scope object folding away — see `AGENT_CONTEXT.md`.

## External symbols the TU calls

`WPADGetInfoAsync`, `getNthController__Q23EGG17CoreControllerMgrFi`,
`init__Q23EGG10CoreStatusFv`, `sceneReset__Q23EGG14CoreControllerFv`,
`sInstance__Q23EGG17CoreControllerMgr`, plus `_savegpr_25` / `_restgpr_25`.

`getWPADInfoCb` is **unmangled**, so it is not an ordinary C++ file-scope
function — it is the callback handed to `WPADGetInfoAsync`. Work out from the
call site what linkage and signature reproduce that name; do not assume.

## The header

`include/game/mLib/m_pad.hpp` is a 19-line stub declaring only `create`,
`beginPad`, `endPad`, the `CH_e` enum, and two globals — one of which
(`g_currentCore`) has the wrong name against the symbol map
(`g_currentCore__4mPad` exists, and `g_currentCoreID__4mPad` alongside it).

**Propose header changes; do not edit the header.** The lead lands them one at a
time and verifies all five binaries after each. Shadow-copy into your own scratch
include directory to test.

## Rules

- **Never run `ninja`, `configure.py`, `progress.py`, `land.py`.**
- Never edit `slices/wiimj2d.json`, `syms.txt`, or any shared header.
- Compile and diff only through `tools/auto_decomp/harness.py`. Extract by
  ADDRESS and assert `instruction_count * 4` against `bin/dtk/wiimj2d_symbols.txt`
  before writing any C++.
- **Name your draft file `m_pad.cpp`.** Anonymous-namespace symbols mangle with
  the filename in them; a draft compiled under any other name diffs forever on
  those lines for reasons unrelated to your source.
- Report a negative result rather than manufacturing a positive one.
