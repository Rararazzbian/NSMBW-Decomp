# Trial link of `d_a_player_manager.cpp` — it LINKS

Run with the unit installed, its slice entry added, and its 32 `syms.txt` pins
removed. **The unit linked and produced a `wiimj2d.dol`.** Then parked back into
`wip/` so the tree stays green; nothing about the result is lost.

## What the link found that nothing else could

**Six undefined symbols**, none of which any per-function diff, section-bound
check or emission audit had surfaced:

```
PauseManager_c::m_instance        -> m_instance__14PauseManager_c = 0x8042A2B8
dScStage_c::m_isCourseIn          -> m_isCourseIn__10dScStage_c   = 0x8042A4FC
dAttention_c::__vt                -> __vt__12dAttention_c         = 0x8030F310
dAttention_c::~dAttention_c()     -> __dt__12dAttention_cFv       = 0x80069110
dPyEffectMng_c::dPyEffectMng_c()  -> __ct__14dPyEffectMng_cFv     = 0x800D2D10
dPyEffectMng_c::~dPyEffectMng_c() -> __dt__14dPyEffectMng_cFv     = 0x800D2D70
```

All six are consequences of decisions made earlier **and correct at the time**:
`m_isCourseIn` and `PauseManager_c` were declared but their TUs are
undecompiled, and the `dAttention_c` / `dPyEffectMng_c` constructors and
destructors were deliberately declared *without inline bodies* to stop MWCC
synthesising weak copies. Both choices were right; both create an undefined
symbol until pinned. **This is precisely the class of blocker trial-linking
exists to find**, and it is the same one that caught the previous unit
(`dActorMng_c::envAllWaterCheck`).

With those six pinned, **the link succeeds.**

## Section sizes: compiled object vs claim

| Section | Compiled | Claim | Delta |
|---|---|---|---|
| `.text` | `0x2AA0` | `0x2A10` | **+0x90 OVER** |
| `.rodata` | `0x1A0` | `0x1A0` | **EXACT** |
| `.sdata2` | `0x38` | `0x38` | **EXACT** |
| `.ctors` | `0x4` | `0x4` | exact |
| `.data` | `0x2A` | `0x30` | −6, linker alignment |
| `.bss` | `0xEAC` | `0xEB0` | −4, linker alignment |
| `.sdata` | `0xC` | `0x10` | −4, linker alignment |
| `.sbss` | `0x51` | `0x58` | −7, linker alignment |

**`.rodata` and `.sdata2` are exact**, which retires the contradiction that two
agents disagreed over — the corrected `0x1A0` claim across six objects is right,
and the `extern const` on `lbl_802EF478` was necessary. The four small
shortfalls are alignment padding the linker adds; they are not defects.

## The one real overflow, and it is already being worked

`.text` is `0x90` over. **`0x80` of that is exactly the two stray destructors** —
`__dt__Q23EGG8Vector2fFv` and `__dt__Q23EGG8Vector3fFv`, 16 instructions each,
neither of which appears anywhere in the retail symbol map. Remove those and
`.text` is `0x10` over, which is within reach of the 23 remaining near-misses.

That is a much sharper statement of the problem than "two cosmetic symbols":
they are **the majority of the only section that overflows.**

## The third variant of the two-way `syms.txt` trap

Marking the slice `nonMatching` to park it does NOT work. The source file is
still compiled and linked, so its symbols collide with the filler object taken
from the original DOL:

```
multiply-defined: 'daPyMng_c::addNum(int)' in filler_12.o
  Previously defined in d_a_player_manager.o
```

To park a unit you must **remove the source file**, not flag the slice. Noted
because the flag looks like the obvious way to do it.

## To resume

1. Copy `wip/player_manager/assembled.cpp` to
   `source/dol/bases/d_a_player_manager.cpp`.
2. Insert the slice block (see git history for the exact 13-line insertion —
   insert it as TEXT before `d_ac_py_key.cpp`; do not rewrite the JSON
   programmatically, that reformats 1500 lines).
3. Remove the 32 `daPyMng_c` pins from `syms.txt`; add the six above.
