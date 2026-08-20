# MIDDLE_BG_FOR_CASTLE_LUDWIG + BOTTOM_BG_FOR_CASTLE_LUDWIG -- function inventory

Coordinator-scoped unit, one translation unit, two profiles. `.text 0xf5130-0xf6150`, 0x1020
bytes, spanning the coordinator's own stated bounds -- confirmed against
`bin/dtk/d_basesNP_symbols.txt` (33 consecutive symbols in range... `verify_anon.py` sees 32 of
them; `fn_2_F5C80`, the `.ctors`-registered `__sinit`, is confirmed present in the symbol table
at the coordinator's own stated address but does not appear in `verify_anon.py`'s own listing --
not yet understood why, flagged here rather than silently ignored). No bounds contradiction
found otherwise.

**This is a MUCH bigger unit than DUMMY_DOOR** -- a real `dEn_c`-derived class with a heap
allocator, a model, two `dBg_ctr_c` zones, and NINE of its own new virtuals past `dEn_c`'s own
vtable. This round reached **12/32 matched** with the class skeleton solid and both gates green,
but this is genuinely a multi-round unit, the same shape as WM_KILLERBULLET took several rounds
on -- NOT reachable to N/N this round. Reporting inventory + honest partial progress rather than
a false completion claim.

## Class structure (confirmed)

Two classes, one translation unit:
- **`daMiddleBgForCastleLudwig_c`** (`: public dEn_c`) -- used DIRECTLY by MIDDLE_BG's own
  classInit (no subclass, vtable pointer never re-stomped after the base ctor). Owns the
  vtable `lbl_2_data_30C3C`.
- **`daBottomBgForCastleLudwig_c`** (`: public daMiddleBgForCastleLudwig_c`) -- a THIN subclass
  used by BOTTOM_BG, overriding ONLY the destructor. Confirmed by direct diff of the two full
  vtable dumps: `lbl_2_data_30998` (BOTTOM_BG's own) is byte-identical to `lbl_2_data_30C3C`
  (base) at every one of 169 slots EXCEPT slot 18/offset 0x48 (the destructor). Also confirmed
  by both classInits' identical `li r3, 0x768` alloc size -- BOTTOM_BG adds not one byte.

`sizeof == 0x768` for both. Members (all offsets compiler-confirmed via the ctor/dtor disasm):
`dHeapAllocator_c mAllocator` @0x524 (size 0x1c), `void *m_540` @0x540, `m3d::mdl_c mModel`
@0x544 (size 0x40), `dBg_ctr_c mBgCtr[2]` @0x584 (size 0x1c8 = 2*0xe4, confirmed against the
real landed `dBg_ctr_c` in `include/game/bases/d_bg_ctr.hpp`), then a confirmed-necessary but
content-unconfirmed `u8 mPad74c[0x1c]` to reach the real `0x768` total (found by a REAL defect:
the first draft's classInit emitted `li r3, 0x750`, 0x18 short of target -- fixed by adding the
trailing pad; this is exactly the "classInit alloc-size gates struct size" lever, same idea as
`dActor_c`'s own size being provable from a subclass's trailing fields on DUMMY_DOOR).

Base class declares NINE new virtuals past `dEn_c`'s own last one (`yoshifumiEffect`,
`d_enemy.hpp:220`) at vtable offsets `0x280/0x284/0x288/0x28c/0x290/0x294/0x298/0x29c/0x2a0` --
found by extracting and offset-indexing the FULL 169-slot vtable programmatically, not by
eyeballing a truncated read (an earlier manual read of just the first ~70 slots would have
missed all nine). Named `vfXXX` by offset -- no mangled name licenses better names.

## MATCHED (12/32)

| draft name | target | size | notes |
|---|---|---|---|
| `daMiddleBgForCastleLudwig_c_classInit__Fv` | `fn_2_F5130` | 12/12 | MIDDLE_BG classInit, naming-only |
| `daBottomBgForCastleLudwig_c_classInit__Fv` | `fn_2_F5160` | 19/19 | BOTTOM_BG classInit, naming-only (own vtable symbol) |
| `getNullState__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F51B0` | 3/3 | EXACT. `return &sStateID::null;` (real landed extern) |
| `__ct__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F51C0` | 29/29 | EXACT. Shared base ctor |
| `__dt__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F5240` | 35/35 | EXACT. Base dtor |
| `entryOrRelease__27daMiddleBgForCastleLudwig_cFb` | `fn_2_F52D0` | 6/6 | EXACT |
| `vf2a0__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F5890` | 21/21 | EXACT. Matrix update (trans/setLocalMtx/setScale, no rotation, no calc) |
| `vf28c__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F5970` | 1/1 | EXACT. Empty body |
| `vf294__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F5980` | 1/1 | EXACT. Empty body |
| `vf288__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F5C00` | 4/4 | EXACT. Forwards to `vf2a0()` |
| `vf280__27daMiddleBgForCastleLudwig_cFv` | `fn_2_F5C10` | 1/1 | EXACT. Empty body |
| `__dt__27daBottomBgForCastleLudwig_cFv` | `fn_2_F5C20` | 22/22 | EXACT. BOTTOM_BG's own trivial derived dtor, byte-for-byte the DUMMY_DOOR pattern |

## PARKED, real content, close (1)

- **`vf290__27daMiddleBgForCastleLudwig_cFv`** (`fn_2_F5990`, 4/4 lines) -- 2/4 differing. A
  pure forwarding thunk into `mModel`'s own vtable at offset 0x1c (`lwzu r12,0x544(r3);
  lwz r12,0x1c(r12); mtctr r12; bctr` -- the target uses r12 exclusively via
  load-with-update addressing, adjusting `r3` itself to `&mModel` for the tail call). Two
  variants tried (inline expression; a named `m3d::mdl_c *m` local), both land on r4 instead of
  r12 for the same reason -- the compiler doesn't perform the update-addressing optimization
  from this C++ shape. Not a content problem; register-allocation residual, park.

## SCOUTED (vtable slot / role known), FAKE STUBS this round -- 6

All placed at their correct address slot so the order gate stays meaningful; return values /
bodies are clearly-marked placeholders, not confirmed content.

| draft name | target | size | role |
|---|---|---|---|
| `create` | `fn_2_F54D0` | 32 | vtable slot 2. Confirmed overridden, content unscouted. |
| `doDelete` | `fn_2_F58F0` | 20 | vtable slot 5. Confirmed overridden, content unscouted. |
| `execute` | `fn_2_F5810` | 31 | vtable slot 8. Confirmed overridden, content unscouted. |
| `draw` | `fn_2_F5940` | 12 | vtable slot 11. Confirmed overridden, content unscouted. |
| `vf284` | `fn_2_F5430` | 38 | vtable offset 0x284. PARTIALLY scouted: takes another instance of this same class, copies a 2-entry visibility byte array at `+0x764` from it, then `mModel.getResMdl().GetResNode(name)`/`setNodeVisibility` per entry against a 2-entry `lbl_2_data_30930` name table. Signature modelled; body still a stub. |
| `vf29c` | `fn_2_F5AD0` | 74 | vtable offset 0x29c. Unscouted (0x128 bytes target). |
| `createModel` | `fn_2_F59A0` | 73 | vtable offset 0x298. Real STRUCTURE confirmed (createFrmHeap/getRes/GetResMdl/mdl_c::create/setSoftLight_Map/adjustFrmHeap, matching the idiom already landed elsewhere) but the actual arc/model NAME strings are NOT read from this unit's own `.rodata`/`.data` pool yet -- `"CASTLE_BG"` is a clearly-marked placeholder. 61/73 differing as a result (wrong pool references, not wrong shape). |

## NOT YET TOUCHED THIS ROUND -- 13 functions, absent (not stubbed in the wrong slot)

| target | size (lines by verify_anon) | notes |
|---|---|---|
| `fn_2_F52F0` | 35 | Unscouted. |
| `fn_2_F5380` | 42 | Unscouted. |
| `fn_2_F5550` | 73 | Unscouted (0x124 bytes real size -- large). |
| `fn_2_F5680` | 99 | Unscouted (0x18C bytes real size -- the LARGEST function in the unit). |
| `fn_2_F5C80` | -- | `__sinit`, the `.ctors`-registered static initialiser. Confirmed present in `d_basesNP_symbols.txt` at `0xf5c80` per the coordinator's own citation, but does NOT appear in `verify_anon.py`'s own target listing for this range -- flagged, not yet understood. Do not attempt to author until this is resolved; whatever static state triggers it is presumably inside one of the still-unauthored functions above (`create`/`execute` are the obvious candidates), matching the "pool/`.ctors` can't be right until every contributor exists" pattern already established on other units this project. |
| `fn_2_F5DB0` | 22 | Unscouted. |
| `fn_2_F5E10` | 23 | Unscouted -- **this is the coordinator's own cited "one internal `.text` target"**, meaning something ELSE in this unit calls it directly; worth scouting its callers first next round. |
| `fn_2_F5E70` | 55 | Unscouted. |
| `fn_2_F5F50` | 56 | Unscouted. |
| `fn_2_F6030` | 34 | Unscouted. |
| `fn_2_F60C0` | 12 | Unscouted, small. |
| `fn_2_F60F0` | 12 | Unscouted, small. |
| `fn_2_F6120` | 12 | Unscouted, small -- last function before the unit's own upper bound `0xf6150`. |

## Gates

- **Function order**: GREEN. `order_sweep.py` reports `ok agent_castle_bg 12/32`.
  `check_fn_order.py` cannot check a real-named draft against anonymous targets (confirmed by
  running it and reading its own explanation) -- `build.py`'s own `verify_anon.py` tail is the
  real check here, per the coordinator's own required wiring.
- **`.ctors`**: GREEN. `ctors_map.py d_basesNP BOTTOM_BG_FOR_CASTLE_LUDWIG` reports
  `0x288 -> __sinit at .text 0xf5c80` -- matching the coordinator's own citation exactly, and
  correctly NOT yet triggered by this draft's own code (nothing authored this round references
  whatever the real `__sinit` initialises), the same "not reachable from authored code yet"
  state killerbullet's own `__sinit` was in for several rounds.

## Real defect found and fixed this round: struct size (`sizeof`)

First draft's `daMiddleBgForCastleLudwig_c` computed to `0x750` (both classInits emitted
`li r3, 0x750`, target wants `0x768`) -- 0x18 bytes short. This silently affects EVERY function
whose codegen depends on the class's actual size or the offset of anything placed relative to
its end (both classInits directly, and indirectly anything else that allocates or measures the
object). Fixed by adding a trailing `u8 mPad74c[0x1c]` after `mBgCtr[2]` (content unconfirmed,
size confirmed by the classInit's own alloc-size gate closing exactly). Both classInits went
from "1 differing" to EXACT/naming-only immediately after.

## Shadow header

`shadow_include/game/bases/d_a_castle_bg.hpp` -- new this round, no prior version. Every real
header it depends on (`dEn_c`, `dHeapAllocator_c`, `dBg_ctr_c`, `m3d::mdl_c`, `sStateID_c`) is
ALREADY LANDED; no shadow copy of any of them was needed or made.

**BINDING note applied throughout** (the DUMMY_DOOR lesson): all nine new virtuals, INCLUDING
the three with a genuinely empty body, are declared without an in-class body and defined out of
line in the `.cpp`. An in-class `{}` compiles WEAK in this compiler and would defer to the end
of the translation unit (LIFO); the target's own three empty ones (`fn_2_F5C10`/`fn_2_F5970`/
`fn_2_F5980`) sit at their ordinary interleaved address positions, matching GLOBAL/out-of-line
binding, not the deferred-weak shape -- confirmed correct by the clean ascending order-gate
result, not assumed.

## How to reproduce this tally

```
python wip/wm_units/agent_castle_bg/build.py
python wip/wm_units/agent_castle_bg/difftool.py \
    wip/wm_units/agent_castle_bg/target_auto_00_000F4FB0_text.txt \
    wip/wm_units/agent_castle_bg/draft.txt \
    fn_2_<addr> <draft_symbol>
```
(Functions at or past `0xf5db0` are in the SECOND text object,
`target_auto_00_000F5DA4_text.txt` -- both are fresh-dumped this round, never reused.)

## Plan for the next round

Smallest-first among what remains: `fn_2_F60C0`/`fn_2_F60F0`/`fn_2_F6120` (12 lines each,
unscouted) are the cheapest next wins, followed by `fn_2_F52F0`/`fn_2_F5380`/`fn_2_F5DB0`/
`fn_2_F5E10` (`E10` especially, being the one cross-referenced internal target). `create`/
`execute`/`doDelete`/`draw` are the highest-value remaining targets (they gate the `.ctors`
mystery and are what everything else in a `dEn_c` actor usually revolves around) but are also
likely the largest real content, alongside `fn_2_F5550`/`fn_2_F5680` (the two biggest functions
in the unit). `createModel`'s pool-string mismatch should close on its own, or close much
further, once this unit's own `.rodata`/`.data` string pool (already dumped fresh this round,
`target_auto_03_00001FE8_rodata.txt`/`target_auto_04_000132B0_data.txt`) is actually read for
the real arc/model names, rather than guessed.
