# `d_a_player_manager.cpp` (`daPyMng_c`) — section bounds and data-object inventory

Derived independently from `bin/dtk/wiimj2d_symbols.txt`, the extracted
neighbourhood files in `wip/player_manager/target_*.txt`, and (new source,
not mentioned in the brief) `bin/dtk/dtk_splits_wiimj2d.txt`, which lists
**officially-split** per-source-file address ranges for a subset of already
"split" TUs. Where a section boundary of ours is immediately adjacent to an
entry in that file, that is the strongest evidence available in this project
(equivalent to "bracketed by banked neighbours") because it is address data
about the *original* binary's layout, not a guess.

Three sibling TUs recur constantly as neighbours on every axis and are not
split in `dtk_splits_wiimj2d.txt` (so they still show up as "unclaimed" gaps
there even though two of them are landed source): `d_a_player_base.cpp`
(banked, always the neighbour *before* us), `d_a_player_hio_ADJ.cpp` (landed,
`dAcPy_HIO_Speed_c`), `d_a_player_demo_manager.cpp` (landed 51/51,
`daPyDemoMng_c`). `d_a_right_base.cpp`, `d_actor.cpp` and `d_a_sink_dokan.cpp`
are officially split and recur as the neighbour *after* us.

## 1. Per-section table

| Section | Offset range | Absolute range | Size | How derived | Confidence |
|---|---|---|---|---|---|
| `.text` | `0x58220-0x5AC30` | `0x8005E9A0`–`0x80061310` | `0x2970` | Bracketed by banked neighbours on both sides: `dtk_splits_wiimj2d.txt` gives `d_ac_py_key.cpp .text end:0x8005E9A0` (our exact start) and `d_a_right_base.cpp .text start:0x800613B0` (our end + `__sinit`, `0x800613B0-0x80061310=0xA0`, vs. brief's stated `__sinit` size `0x9C` — 4-byte discrepancy, see §4). Re-derivation **confirms** the brief. | Exact |
| `.ctors` | `0x88-0x8c` | `0x802EDD68`–`0x802EDD6C` | `0x4` | By subtraction against a hard bracket: `d_a_player_base.cpp .ctors end:0x802EDD60`, `d_a_right_base.cpp .ctors start:0x802EDD6C`. That is a **3-slot** gap (`0x802EDD60/64/68`), not the "one free slot" the brief describes — our slot is confirmed as the *last* of the three (directly adjacent to `d_a_right_base.cpp`), but the other two are unaccounted for, most plausibly one each for `d_a_player_hio_ADJ.cpp`'s and `d_a_player_demo_manager.cpp`'s own `__sinit` (both are also unsplit here). See §4. | Exact for our own slot; the "one free slot" framing is a **contradiction** |
| `.rodata` | `0x1628-0x1638` | `0x802EF608`–`0x802EF618` | `0x10` | Bracketed by banked neighbours on both sides, confirmed twice over: (a) the extracted neighbourhood shows `daAcPy_HIO_Speed_c`/`d_a_player_hio_ADJ.cpp` objects ending at `0x802EF5F8+0x10=0x802EF608`, and our own `scModelTypeDt` is the terminal object before `daSinkDokan_c::draw()::cs_root_ofs`; (b) `dtk_splits_wiimj2d.txt` independently confirms with `... .rodata end:0x802EEEA0` (start of extracted neighbourhood) and `d_a_sink_dokan.cpp .rodata start:0x802EF618` (our exact end). **We own exactly one object.** | Exact |
| `.data` | — | `0x80309A28`–`0x80309A58` | `0x30` | **Open question, Codex's** — see §3. Upper bound is hard either way: `dtk_splits_wiimj2d.txt` gives `d_a_player_base.cpp .data end:0x80309908` and `d_a_right_base.cpp .data start:0x80309A58`, so nothing in `0x80309908-0x80309A58` is officially split; `0x80309908-0x80309A28` is independently attributable to `d_a_player_demo_manager.cpp` (its named vtable, `sc_ForceList`, and hanabi/fireworks statics — none referenced by our `.text`), leaving only the `0x30` at the tail contested. If Codex resolves it to us, this is our only `.data` claim; if not, we own **zero** bytes of `.data`. | N/A — open, evidence below |
| `.sdata` | `0x280-0x290` | `0x80427C00`–`0x80427C10` | `0x10` | Bracketed by banked neighbours on both sides: `dtk_splits_wiimj2d.txt` gives `d_a_en_togezo_base.cpp .sdata end:0x80427BE8` (start of extracted neighbourhood) and `d_actor.cpp .sdata start:0x80427C10` (our exact end). Within that neighbourhood, `@72502_80427BF0`/`@72503_80427BF8` (the two WPAD force-list ID strings) are referenced only from `sc_ForceList__6dWmLib`, which is not ours (see `.data`), so they belong to `d_a_player_demo_manager.cpp` too. Our claim is exactly the three named `@unnamed@d_a_player_manager_cpp@` statics. Re-derivation **confirms** the brief exactly. | Exact |
| `.sdata2` | `0xA18-0xA20` (contested tail) / `0x9E8-0xA20` (our best claim) | `0x8042BD48`–`0x8042BD80` | `0x38` (of which `0x8` is contested) | **Not fully derivable from bracketing** — see §4. Lower bound `0x8042BCF8` (start of extracted neighbourhood) is bracketed: `d_a_player_base.cpp .sdata2 end:0x8042BCF8`. Upper bound `0x8042BD80` is bracketed: `d_a_right_base.cpp .sdata2 start:0x8042BD80`. That whole `0x88`-byte span is **unsplit** and shared among the same three sibling TUs as everywhere else. Within it, every object from offset `0x50` (`0x8042BD48`) to the end is referenced by our named functions (`getPlayerSetPos`, `createCourseInit`, `deleteCullingYoshi`, `fn_8005F4D0`); nothing before offset `0x50` is referenced anywhere in our `.text`. Contents-driven, not a hard bracket. | Medium (contents-driven) for `0x8042BD48-0x8042BD78`; the last `0x8` is Codex's |
| `.bss` | `0x3790-0x4640` | `0x80355110`–`0x80355FC0` | `0xEB0` | Bracketed by banked neighbours on both sides, confirmed via `dtk_splits_wiimj2d.txt`: `d_a_player_base.cpp .bss end:0x80354F20` (start of extracted neighbourhood, though our first *named* object starts 0x1F0 further in — the space between is `d_a_player_hio_ADJ.cpp`'s `sc_playerSpeedDt`) and `d_a_right_base.cpp .bss start:0x80355FC0` (our exact end). Re-derivation **confirms** the brief exactly. | Exact |
| `.sbss` | `0xE0-0x138` | `0x80429F80`–`0x80429FD8` | `0x58` | Bracketed by banked neighbours on both sides: low side, `ms_num_of_instance__17dAcPy_HIO_Speed_c` (`d_a_player_hio_ADJ.cpp`) ends at `0x80429F7C`, then a 4-byte alignment gap to our 8-aligned start at `0x80429F80`; high side, `dtk_splits_wiimj2d.txt` gives `d_actor.cpp .sbss start:0x80429FD8` **exactly** where our last member + a 7-byte alignment gap lands. Section base computed as `0x80429EA0` (confirmed by `d_2d.cpp .sbss start:0x80429EA0`, the very first `.sbss` object in the whole map). **This directly contradicts the brief's stated `0xe0-0x110` (`0x30` bytes) — see §4, this is the headline finding.** | Exact — and it overturns the brief |
| `.sbss2` | — | — | `0x0` | By elimination: no symbol anywhere in `wiimj2d_symbols.txt` carries the `__9daPyMng_c` mangling under `.sbss2`, and nothing in our `.text` references any `.sbss2` address. **Weakest kind of claim** — per the project's own standing hazard (header statics/unreferenced-but-ours objects), absence of reference is not proof of absence of ownership, but there is no positive evidence either, and no plausible header-static candidate was found for this class (all-static, no vtable, no nested types with static instances). | Low (by elimination) but no counter-evidence found |

## 2. Data-object inventory

### `.rodata` — our only object (referenced)

| Address | Section | Symbol | Size | What | Referenced by us? |
|---|---|---|---|---|---|
| `0x802EF608` | `.rodata` | `@LOCAL@getCourseInPlayerModelType__9daPyMng_cFUc@scModelTypeDt` | `0x10` | `unsigned char[4]` function-local static table `{0,1,2,3}` | Yes — `getCourseInPlayerModelType__9daPyMng_cFUc` at `0x8005FBE0`, `lis`/`addi` `@ha`/`@l` at `0x8005FC10-14` |

### `.sdata` — our three named statics

| Address | Section | Symbol | Size | What | Referenced by us? |
|---|---|---|---|---|---|
| `0x80427C00` | `.sdata` | `scRestMax__32@unnamed@d_a_player_manager_cpp@` | `0x4` | anon-namespace `int` = `0x63` (99) | not directly checked, but named to our TU by the mangling itself |
| `0x80427C04` | `.sdata` | `scCoinMax__32@unnamed@d_a_player_manager_cpp@` | `0x4` | anon-namespace `int` = `0x63` (99) | same |
| `0x80427C08` | `.sdata` | `scScoreMax__32@unnamed@d_a_player_manager_cpp@` | `0x4` | anon-namespace `int` = `0x3B9AC9FF` (999999999) | same |
| `0x80427C0C` | `.sdata` | `gap_09_80427C0C_sdata` | `0x4` | zero-fill padding to reach `0x80427C10` | — |

These three carry `@unnamed@d_a_player_manager_cpp@` in their mangled name —
**self-attributing**, no ambiguity possible.

### `.bss` — four embedded instances + destructor-chain records + twelve small statics

| Address | Section | Symbol | Size | What | Owner |
|---|---|---|---|---|---|
| `0x80354F20` | `.bss` | `@72505_80354F20` | `0xC` | destructor-chain record | `d_a_player_hio_ADJ.cpp` (precedes `sc_playerSpeedDt`) |
| `0x80354F2C` | `.bss` | `gap_08_80354F2C_bss` | `0x4` | align pad | " |
| `0x80354F30` | `.bss` | `sc_playerSpeedDt__17dAcPy_HIO_Speed_c` | `0x1E0` | `dAcPy_HIO_Speed_c::sc_playerSpeedDt[2][2]` (see header note: `const` array with a dynamic initializer, lives in `.bss` not `.rodata`) | `d_a_player_hio_ADJ.cpp` |
| `0x80355110` | `.bss` | `m_playerID__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355120` | `.bss` | `m_yoshiID__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355130` | `.bss` | `mCourseInList__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355140` | `.bss` | `m_yoshiFruit__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355150` | `.bss` | `mPlayerEntry__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355160` | `.bss` | `mPlayerType__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355170` | `.bss` | `mPlayerMode__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355180` | `.bss` | `mCreateItem__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x80355190` | `.bss` | `mRest__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x803551A0` | `.bss` | `mCoin__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x803551B0` | `.bss` | `m_quakeTimer__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x803551C0` | `.bss` | `m_quakeEffectFlag__9daPyMng_c` | `0x10` | **ours** | daPyMng_c |
| `0x803551D0` | `.bss` | `@77033` (local) | `0xC` | destructor-chain record for `mDemoManager` | daPyMng_c |
| `0x803551DC` | `.bss` | `gap_08_803551DC_bss` | `0x4` | align pad | daPyMng_c |
| `0x803551E0` | `.bss` | `mDemoManager__9daPyMng_c` | `0x98` | embedded `daPyDemoMng_c` — hazard #1, size proven/landed | **ours** (member), type foreign |
| `0x80355278` | `.bss` | `@77034` (local) | `0xC` | destructor-chain record for `mMultiManager` | daPyMng_c |
| `0x80355284` | `.bss` | `mMultiManager__9daPyMng_c` | `0x5C` | embedded `dMultiMng_c` — size assumed per brief, NOT ours to prove | **ours** (member), sizeof is Codex's |
| `0x803552E0` | `.bss` | `@77035` (local) | `0xC` | destructor-chain record for `mAttention` | daPyMng_c |
| `0x803552EC` | `.bss` | `gap_08_803552EC_bss` | `0x4` | align pad | daPyMng_c |
| `0x803552F0` | `.bss` | `mAttention__9daPyMng_c` | `0x58` | embedded `dAttention_c` — size assumed per brief | **ours** (member), sizeof is Codex's |
| `0x80355348` | `.bss` | `@77036` (local) | `0xC` | destructor-chain record for `mEffectMng` | daPyMng_c |
| `0x80355354` | `.bss` | `mEffectMng__9daPyMng_c` | `0xC5C` | embedded `dPyEffectMng_c` — size assumed per brief | **ours** (member), sizeof is Codex's |
| `0x80355FB0` | `.bss` | `gap_08_80355FB0_bss` | `0x10` | trailing align pad to `0x80355FC0` (`d_a_right_base.cpp`'s start) | daPyMng_c (trailing) |

All twelve `0x10`-sized `m*`/`mCourseInList` members are visibly oversized for
their apparent contents (a `bool`/`int`/pointer would be `0x1`-`0x4`) — each
is almost certainly a small `X[4]` per-player array, consistent with function
bodies indexing them by player number 0-3. Not re-deriving the exact element
type here; it's a function-body concern, not a bounds concern.

### `.sbss` — the headline finding: 17 more members than the brief accounts for

| Address | Symbol | Size | Referenced by us? (function) |
|---|---|---|---|
| `0x80429F80` | `mNum__9daPyMng_c` | `0x4` | yes |
| `0x80429F84` | `mCtrlPlrNo__9daPyMng_c` | `0x4` | yes |
| `0x80429F88` | `mActPlayerInfo__9daPyMng_c` | `0x1` | yes |
| `0x80429F8C` | `m_yoshiColor__9daPyMng_c` | `0x4` | yes |
| `0x80429F90` | `m_star_time__9daPyMng_c` | `0x8` | yes |
| `0x80429F98` | `m_star_count__9daPyMng_c` | `0x8` | yes |
| `0x80429FA0` | `mScore__9daPyMng_c` | `0x4` | yes |
| `0x80429FA4` | `mKinopioMode__9daPyMng_c` | `0x4` | yes |
| `0x80429FA8` | `mTimeUpPlayerNum__9daPyMng_c` | `0x4` | yes |
| `0x80429FAC` | `mAllBalloon__9daPyMng_c` | `0x4` | yes |
| `0x80429FB0` | `mPauseEnableInfo__9daPyMng_c` | `0x4` | yes — **the brief's claimed `.sbss` upper bound (`0x80429FB0`) cuts off exactly here** |
| `0x80429FB4` | `mPauseDisable__9daPyMng_c` | `0x4` | yes |
| `0x80429FB8` | `mStopTimerInfo__9daPyMng_c` | `0x4` | yes |
| `0x80429FBC` | `mStopTimerInfoOld__9daPyMng_c` | `0x4` | yes |
| `0x80429FC0` | `mQuakeTrigger__9daPyMng_c` | `0x4` | yes |
| `0x80429FC4` | `mBgmState__9daPyMng_c` | `0x4` | yes |
| `0x80429FC8` | `mBonusNoCap__9daPyMng_c` | `0x4` | yes |
| `0x80429FCC` | `mKinopioCarryCount__9daPyMng_c` | `0x4` | yes |
| `0x80429FD0` | `lbl_80429FD0` (unnamed, `bool`-shaped) | `0x1` | yes — `setHipAttackQuake__9daPyMng_cFiUc` (`lbz`/`stb` at `0x80060D04`/`0x80060D30`) |
| `0x80429FD1` | (unlabeled) | `0x7` | align pad to `d_actor.cpp`'s `0x80429FD8` |

**All 17 named objects and the unnamed byte are referenced by name in our
`.text`.** This is not a "found unreferenced, still ours" case (the hazard
the brief warns about) — the opposite: every one of these has a live
reference, and the brief's own derivation simply stopped nine members early
(`0x80429FB0`, right at `mPauseEnableInfo`). See §4 for the specific
functions that reference the omitted nine.

### `.sdata2` — split between confirmed-ours and Codex's open question

| Address | Symbol | Size | Referenced by us? (function) |
|---|---|---|---|
| `0x8042BD48` | `@80186_8042BD48` (`0.0f`) | `0x4` | yes — `getPlayerSetPos`, `createCourseInit`, `deleteCullingYoshi` |
| `0x8042BD4C` | (unlabeled) | `0x4` | align pad for the following `double` |
| `0x8042BD50` | `@80189_8042BD50` (`double`, `0x4330000000000000`) | `0x8` | yes — `getPlayerSetPos` |
| `0x8042BD58` | `lbl_8042BD58` (`503.0f`) | `0x4` | yes — `createCourseInit` |
| `0x8042BD5C` | `@80386_8042BD5C` (`0.1f`) | `0x4` | yes — `createCourseInit` |
| `0x8042BD60` | `@80387_8042BD60` (`12.0f`) | `0x4` | yes — `createCourseInit` |
| `0x8042BD64` | `@80388_8042BD64` (`24.0f`) | `0x4` | yes — `createCourseInit` |
| `0x8042BD68` | `@80390_8042BD68` (`double`, `0x4330000080000000`) | `0x8` | yes — `createCourseInit` |
| `0x8042BD70` | `lbl_8042BD70` (`{25,26}`) | `0x8` | yes — `fn_8005F4D0` |
| `0x8042BD78` | `@80832_8042BD78` (`0.5f`) | `0x4` | yes, 3x — `fn_80060DB0` (`0x800602BC`, `0x80060498`, `0x80060AB8`) — **Codex's open question** |
| `0x8042BD7C` | `@81205_8042BD7C` (`~3701.5`) | `0x4` | yes — `fn_80060DB0` (`0x80060DDC`) — **Codex's open question** |

Everything at offset `< 0x50` in the extracted neighbourhood (`@77479`
through `@67945`, `0x8042BCF8-0x8042BD48`) has **no** reference anywhere in
our `.text`. Per pool-ID proximity (`67883-78251`, far below our
`80186-81206` cluster) these most plausibly belong to `d_a_player_base.cpp`
or `d_a_player.cpp` rather than to us, but this is not proven and is not
something I've settled — flagging it only as a secondary, lower-confidence
open item alongside Codex's.

## 3. `.data` open question — evidence for Codex, not a conclusion

The brief says Codex owns `0x80309A28-0x80309A58` (the two strings) and that
`d_a_player_hio_ADJ.cpp` is the leading alternative to `daPyMng_c`. Evidence
found while deriving bounds, reported without settling it:

- `fn_80060DB0` (`0x80060DB0-0x80060EE8`, unnamed, within our `.text`) is
  unambiguously a `daPyMng_c` method: it touches `m_quakeTimer__9daPyMng_c`,
  `m_quakeEffectFlag__9daPyMng_c`, calls `getPlayer__9daPyMng_cFi`,
  `isStatus__10daPlBase_cFi`, `getRideYoshi__7dAcPy_cFv`, and
  `startShock__8dQuake_cFScQ28dQuake_c12TYPE_SHOCK_eiib` in a loop over the
  four players — a hip-attack/quake-shock effect dispatcher.
- That same function loads **both** disputed strings directly:
  `lis r30, "@81204_80309A28"@ha` / `addi r3, r30, "@81204_80309A28"@l` at
  `0x80060DF0`/`0x80060E50`, and `lis r31, "@81206_80309A3C"@ha` / `addi r3,
  r31, "@81206_80309A3C"@l` at `0x80060DF4`/`0x80060E80`, both passed as the
  effect-name argument to
  `createEffect__3mEfFPCcUlPC7mVec3_cPC7mAng3_cPC7mVec3_c`.
- Pool-ID evidence, per the project's own cheapest-attribution technique:
  `@81204` (the first string), `@81205` (the `.sdata2` float discussed
  above, also read by this same function three lines later at `0x80060DDC`),
  and `@81206` (the second string) are **three consecutive pool IDs**, all
  three referenced from the same one function, which is unambiguously ours.

This is fairly strong evidence that `daPyMng_c` (not `d_a_player_hio_ADJ.cpp`)
owns both the `.data` strings and the `.sdata2` tail — but per the brief this
is Codex's call, not mine, and I have not touched `.data`/`.sdata2` claims
based on it.

## 4. Contradictions with the brief

1. **`.sbss` (major).** Brief: `0xe0-0x110` (`0x80429F80`–`0x80429FB0`,
   `0x30` bytes, 11 members: `mNum` through `mAllBalloon`). Re-derivation:
   `0xe0-0x138` (`0x80429F80`–`0x80429FD8`, `0x58` bytes, 17 members plus an
   unnamed byte plus alignment pad). The lower bound matches exactly; the
   upper bound is short by `0x28` bytes / 9 members
   (`mPauseEnableInfo`, `mPauseDisable`, `mStopTimerInfo`,
   `mStopTimerInfoOld`, `mQuakeTrigger`, `mBgmState`, `mBonusNoCap`,
   `mKinopioCarryCount`, and the unnamed `lbl_80429FD0`). Every one of the
   nine missing objects is referenced by name in our `.text` — this is not a
   subtle unreferenced-object miss, it is a plain truncation. The corrected
   upper bound is confirmed hard by `dtk_splits_wiimj2d.txt`
   (`d_actor.cpp .sbss start:0x80429FD8`), not just by elimination.
2. **`.ctors` (minor).** Brief: "one free slot" alongside ours. Re-derivation
   finds a 3-slot gap (`0x802EDD60/64/68`) between `d_a_player_base.cpp` and
   `d_a_right_base.cpp`, of which our slot (`0x802EDD68`) is confirmed as the
   last. The other two slots are unaccounted for — plausibly one each for
   `d_a_player_hio_ADJ.cpp` (which the header confirms has its own
   `__sinit`) and `d_a_player_demo_manager.cpp`. Doesn't change our own
   claim, but "one free slot" undersells what's actually unclaimed there.
3. **`__sinit` size (very minor).** Brief: `__sinit_d_a_player_manager_cpp`
   at `0x80061310`, size `0x9C`, i.e. ending `0x800613AC`.
   `dtk_splits_wiimj2d.txt` puts the next TU (`d_a_right_base.cpp`) at
   `0x800613B0` — a 4-byte gap past the brief's stated end, almost certainly
   ordinary function alignment padding, not a sizing error. Noted for
   completeness only.
4. **Function count.** Brief says "All 68 functions" are in
   `target_text.txt`. A direct count of non-`gap_` `.fn` entries in that file
   finds **67** (54 `gap_*` padding entries excluded). Listed in full in §5.
   Minor, but flagging since the brief was explicit about the number.

## 5. Canonical address order

### `.text` — 67 functions (54 `gap_*` padding entries omitted; full addresses
in `target_text.txt`, unchanged from extraction, not reproduced here since
the brief says this file is already correct and not to be re-derived)

```
0x8005E9A0  0xB8   createYoshi__9daPyMng_cFR7mVec3_ciP7dAcPy_c
0x8005EA60  0xA4   initGame__9daPyMng_cFv
0x8005EB10  0x180  initStage__9daPyMng_cFv
0x8005EC90  0x8    getCourseIn__10dScStage_cFv          [hazard #2: foreign weak inline]
0x8005ECA0  0x4    exitStage__9daPyMng_cFv
0x8005ECB0  0x30   courseIn__9daPyMng_cFv
0x8005ECE0  0xA4   setDefaultParam__9daPyMng_cFv
0x8005ED90  0xDC   getPlayerSetPos__9daPyMng_cFUcUc      [refs .sdata2 0x8042BD48, 0x8042BD50]
0x8005EE70  0x20   getFileP__5dCd_cFi                    [hazard #2: foreign weak inline]
0x8005EE90  0x50   getPlayerCreateAction__9daPyMng_cFv
0x8005EEE0  0x64   create__9daPyMng_cFiP7mVec3_ciUc
0x8005EF50  0x580  createCourseInit__9daPyMng_cFv        [refs .sdata2 0x8042BD48,58,5C,60,64,68]
0x8005F4D0  0x9C   fn_8005F4D0 (unnamed)                 [refs .sdata2 0x8042BD70]
0x8005F570  0x50   fn_8005f570__9daPyMng_cF16PLAYER_POWERUP_ei (unnamed)
0x8005F5C0  0x2B8  update__9daPyMng_cFv
0x8005F880  0x3C   isPlayerPauseEnable__9daPyMng_cFSc
0x8005F8C0  0x38   setPlayer__9daPyMng_cFiP7dAcPy_c
0x8005F900  0x14   getPlayer__9daPyMng_cFi
0x8005F920  0x64   decideCtrlPlrNo__9daPyMng_cFv
0x8005F990  0x64   setYoshi__9daPyMng_cFP10daPlBase_c
0x8005FA00  0x58   releaseYoshi__9daPyMng_cFP10daPlBase_c
0x8005FA60  0x9C   getYoshi__9daPyMng_cFi
0x8005FB00  0x6C   getYoshiNum__9daPyMng_cFv
0x8005FB70  0x14   getYoshiDirectP__9daPyMng_cFi
0x8005FB90  0x50   getCtrlPlayer__9daPyMng_cFi
0x8005FBE0  0x40   getCourseInPlayerModelType__9daPyMng_cFUc   [owns .rodata 0x802EF608]
0x8005FC20  0x1C   setCarryOverYoshiInfo__9daPyMng_cFUcUci
0x8005FC40  0xC    getYoshiColor__9daPyMng_cFUc
0x8005FC50  0x14   getYoshiFruit__9daPyMng_cFUc
0x8005FC70  0xA8   getActScrollInfo__9daPyMng_cFv
0x8005FD20  0x8C   getScrollNum__9daPyMng_cFv
0x8005FDB0  0x74   addNum__9daPyMng_cFi
0x8005FE30  0x7C   decNum__9daPyMng_cFi
0x8005FEB0  0x18   addNum__9daPyMng_cFv
0x8005FED0  0x18   decNum__9daPyMng_cFv
0x8005FEF0  0xB8   getNumInGame__9daPyMng_cFv
0x8005FFB0  0x58   getEntryNum__9daPyMng_cFv
0x80060010  0x74   getItemKinopioNum__9daPyMng_cFv
0x80060090  0x74   getItemKinopio__9daPyMng_cFv
0x80060110  0x5C   getPlayerIndex__9daPyMng_cF13PLAYER_TYPE_e
0x80060170  0x64   changeItemKinopioPlrNo__9daPyMng_cFRi
0x800601E0  0x14   getCourseInListPlrNo__9daPyMng_cFi
0x80060200  0x50   getCoinAll__9daPyMng_cFv
0x80060250  0x208  incCoin__9daPyMng_cFi
0x80060460  0x128  addRest__9daPyMng_cFiib
0x80060590  0x70   incRestAll__9daPyMng_cFb
0x80060600  0x90   decRest__9daPyMng_cFi
0x80060690  0x68   addScore__9daPyMng_cFii
0x80060700  0x1C   setCourseInStarBGM__9daPyMng_cFv
0x80060720  0x24   startStarBGM__9daPyMng_cFv
0x80060750  0x74   stopStarBGM__9daPyMng_cFv
0x800607D0  0x60   startMissBGM__9daPyMng_cFi
0x80060830  0x24   startYoshiBGM__9daPyMng_cFv
0x80060860  0x78   stopYoshiBGM__9daPyMng_cFv
0x800608E0  0x88   checkLastAlivePlayer__9daPyMng_cFv
0x80060970  0x98   executeLastPlayer__9daPyMng_cFv
0x80060A10  0x98   executeLastAll__9daPyMng_cFv
0x80060AB0  0x158  deleteCullingYoshi__9daPyMng_cFv      [refs .sdata2 0x8042BD48]
0x80060C10  0x1A0  setHipAttackQuake__9daPyMng_cFiUc     [refs .sbss lbl_80429FD0]
0x80060DB0  0x138  fn_80060DB0 (unnamed)                 [refs .data 0x80309A28/A3C — open; .sdata2 0x8042BD78/7C — open]
0x80060EF0  0x24   checkBonusNoCap__9daPyMng_cFv
0x80060F20  0xB8   initYoshiPriority__9daPyMng_cFP10daPlBase_c
0x80060FE0  0x98   setYoshiPriority__9daPyMng_cFP10daPlBase_c
0x80061080  0x5C   isEffectStop__9daPyMng_cFi
0x800610E0  0x24   isAcceptQuake__9daPyMng_cFi
0x80061110  0x48   isCreateBalloon__9daPyMng_cFi
0x80061160  0x1A4  checkCorrectCreateInfo__9daPyMng_cFv
[__sinit_d_a_player_manager_cpp at 0x80061310, 0x9C-0xA0, in the later auto object per brief]
```

### `.sbss` — 17 members + 1 unnamed byte, in declaration order (matches
address order; presumed to match the class's member declaration order)

```
mNum, mCtrlPlrNo, mActPlayerInfo, m_yoshiColor, m_star_time, m_star_count,
mScore, mKinopioMode, mTimeUpPlayerNum, mAllBalloon, mPauseEnableInfo,
mPauseDisable, mStopTimerInfo, mStopTimerInfoOld, mQuakeTrigger, mBgmState,
mBonusNoCap, mKinopioCarryCount, [unnamed bool @0x80429FD0]
```

### `.bss` — order given in §2's table (four embedded objects each preceded
by a `0xC` destructor-chain record, in class-member order: `mDemoManager`,
`mMultiManager`, `mAttention`, `mEffectMng`)

### `.sdata` — `scRestMax`, `scCoinMax`, `scScoreMax` (address order, all three
`0x4`, contiguous)

### `.rodata` / `.data` / `.sdata2` — see §2/§3 tables; `.data` and part of
`.sdata2` are open

## Files referenced

- `wip/player_manager/SHARED-BRIEF.md`
- `wip/player_manager/target_text.txt`, `target_rodata.txt`, `target_data.txt`,
  `target_bss.txt`, `target_sdata.txt`, `target_sdata2.txt`
- `bin/dtk/wiimj2d_symbols.txt` (primary symbol map)
- `bin/dtk/dtk_splits_wiimj2d.txt` (per-file officially-split address ranges —
  not mentioned in the brief; used here as independent ground truth)
- `include/game/bases/d_a_player_hio.hpp`, `source/dol/bases/d_a_player_hio_ADJ.cpp`
  (confirms `dAcPy_HIO_Speed_c`'s `.rodata`/`.bss`/`.sbss` ownership)
