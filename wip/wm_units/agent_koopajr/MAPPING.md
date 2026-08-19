# WM_KOOPAJR (`daWmKoopaJr_c`) — function inventory

`daWmKoopaJr_c : public dWmDemoActor_c`, `sizeof == 0x360` (confirmed twice:
`fn_2_16D290`'s `li r3, 0x360`, and independently by the destructor's member
teardown reaching exactly `+0x184` before falling into the base dtor chain).

Unit bounds: `.text` `0x16d290`-`0x16e540` (20 real functions, confirmed by
listing every `.fn` in that exact address range across both target dumps —
`gap_*`/`pad_*` entries excluded). This count matches the task's "20" headline
exactly.

Tools used: `wip/wm_units/agent_koopajr/build.py` (compile+disasm) and
`difftool.py` (raw per-function instruction diff against the target dump,
address-suffix differences only — **not** the project's real land-time
diff/link, see caveat at the bottom).

## Status legend

- **MATCH** — 0 differing lines via `difftool.py`.
- **MATCH\*** — every differing line is a symbol-NAME-only difference for a
  reference to this unit's own not-yet-separately-compiled code (this class's
  own vtable, or a `bl` to another of this unit's own member functions). The
  target dump names these `lbl_2_data_XXXXX`/`fn_2_XXXXXXX` because the unit
  hasn't been split into its own object yet; my draft names them by their real
  C++ symbol. Expected to resolve automatically once this file lands and gets
  compiled as its own object — **not independently verified against the
  project's true link-time diff**.
- **BLOCKED (rodata pool)** — logic matches, but some `lis/lfs` immediate or
  displacement differs because the shared anonymous `.rodata` pool
  (`lbl_2_rodata_8BA0`..`0x8c90`) also holds constants that belong to
  `fn_2_16D940`/`fn_2_16E3A0` (proven: several pool slots, e.g.
  `0x8bc4/0x8bd4/0x8bd8/0x8be0/0x8bec`, never appear in any authored
  function's disassembly), so those functions must be authored first for this
  unit's own constants to land at the retail offsets.
- **NOT ATTEMPTED** — not authored at all.

## The six named functions (task priority)

| target | role | size | draft symbol | status |
|---|---|---|---|---|
| `fn_2_16D580` | `doDelete` | 0x8 | `doDelete__13daWmKoopaJr_cFv` | **MATCH** (0/2 lines) |
| `fn_2_16D530` | `draw` | 0x4C | `draw__13daWmKoopaJr_cFv` | **MATCH** (0/19 lines) |
| `fn_2_16D340` | destructor | 0xAC | `__dt__13daWmKoopaJr_cFv` | **MATCH** (0/43 lines) |
| `fn_2_16D870` | `processCutsceneCommand` | 0xB0 | `processCutsceneCommand__13daWmKoopaJr_cFib` | **MATCH\*** (3/44 lines, all `bl startAction` vs `bl fn_2_16D7F0`) |
| `fn_2_16D3F0` | `create` | 0x64 | `create__13daWmKoopaJr_cFv` | 3 lines MATCH\* (own-symbol calls) + **2 lines BLOCKED (rodata pool)** — the `lbl_2_rodata_8C0C` (250.0f, `mClipSphere` radius) `lis/lfs` pair |
| `fn_2_16D460` | `execute` | 0xD0 | `execute__13daWmKoopaJr_cFv` | 1 line MATCH\* (own-symbol call) + **5 lines BLOCKED (rodata pool)** — the `lbl_2_rodata_8BA0` base load and three displacements inside it (PTMF table base `+0x70`, `CalcShadow` constants at `+0x88`/`+0x8c`) |

**4 of 6 fully closed** (doDelete, draw, destructor byte-identical modulo
nothing; processCutsceneCommand byte-identical modulo own-symbol naming that
resolves at land time). **create and execute have fully correct, verified
logic** — every instruction, register, and branch matches — **but cannot reach
byte-identity until `fn_2_16D940`/`fn_2_16E3A0` are authored**, because their
constants share one pool with those two unwritten functions. This is not a
logic defect; see the class-declaration comment in the `.cpp` and the
"rodata pool" note above for the exact proof (specific pool slots with no
referencing function found anywhere in this unit's authored code).

## Supporting non-virtual members (needed as callees, not independently required)

| target | size | draft symbol | status |
|---|---|---|---|
| `fn_2_16D290` | 0x30 | `daWmKoopaJr_c_classInit__Fv` (ACTOR_PROFILE-generated) | **MATCH\*** (1/12 lines, own ctor symbol) |
| `fn_2_16D2C0` | 0x74 | `__ct__13daWmKoopaJr_cFv` (constructor) | **MATCH\*** (2/29 lines, own vtable symbol) |
| `fn_2_16D700` | 0xB0 | `calcModel__13daWmKoopaJr_cFv` | **MATCH** (0/44 lines) |
| `fn_2_16D7B0` | 0xC | `resetState__13daWmKoopaJr_cFv` | **MATCH\*** (1/3 lines, own-symbol tail call) |
| `fn_2_16D7C0` | 0x20 | `resetScaleAndProc__13daWmKoopaJr_cFv` | 2/8 lines — **BLOCKED (rodata pool)**, the 0.01f constant |
| `fn_2_16D7E0` | 0x4 | `procNone__13daWmKoopaJr_cFv` | **MATCH** (0/1 lines; empty body, the PTMF "idle" handler) |
| `fn_2_16D7F0` | 0x34 | `startAction__13daWmKoopaJr_cFi` | **MATCH\*** (1/13 lines, own-symbol call) |
| `fn_2_16D920` | 0x18 | `lookupAction__13daWmKoopaJr_cFi` | logic/shape matches; **NOT verified** — the 4-entry lookup table `lbl_2_rodata_8C38` content is unrecovered, so the table itself (currently `{0,0,0,0}`) is a guess. 2/6 lines differ (table symbol). |
| `fn_2_16D830` | 0x3C | `procMain__13daWmKoopaJr_cFv` | **MATCH\*** (2/15 lines, own-symbol calls to `runMain`/`resetScaleAndProc`) |
| `fn_2_16D590` | 0x170 | `createModel__13daWmKoopaJr_cFv` | **NOT verified / best-effort.** All strings recovered directly from the REL's `.data` (see below) and the call shape/argument order matches the disassembly read instruction-by-instruction, but register allocation differs (86/92 lines) — most likely driven by the same missing rodata-pool constants forcing a different local/register schedule. Not re-attempted a second time; parked. |

## Not attempted (out of scope per task instructions)

| target | size | note |
|---|---|---|
| `fn_2_16D940` | 0xA60 | The unit's largest function by far. Confirmed by vtable elimination to be a plain non-virtual member (none of the six overrides). Declared as `bool runMain()` with a `return false;` stub purely so `procMain()` compiles — **the stub body is not a claim about the real function**, just a placeholder. |
| `fn_2_16E3A0` | 0xE4 | Not declared at all. Likely (with `fn_2_16D940`) the owner of the unclaimed rodata pool slots. |
| `fn_2_16E490` | 0x84 | `__sinit_d_a_wm_koopajr_cpp` — the file's static initializer. Constructs a global object at `lbl_2_data_45DE8` using `dCsvData_c::c_CASTLE_ID`/`c_START_ID` (dynamic-init statics, hence needing `__sinit` rather than being compile-time constants) and three floats from the shared rodata pool, then calls `__register_global_object` with a destructor pointer `fn_2_16E520`. The identity/type of the constructed object is unresolved. Not required for any of the six named functions. |
| `fn_2_16E520` | 0x1C | The above global object's destructor wrapper (target of `__register_global_object`'s 2nd arg). Not declared. |

## Class layout — every offset is measured, not hand-counted

All of the following were read directly off a `bl __ct__.../__dt__...`,
a raw `stw`/`stfs` in the constructor/destructor, or a call argument in
`calcModel()`/`create()` whose signature pins the field — see the long
comment at the top of `d_a_wm_koopajr.cpp` for the exact instruction-level
citation of each one.

```
+0x184  int mUnk184                    -- raw store, ctor does NOT initialise it (family convention)
+0x188  dHeapAllocator_c mAllocator
+0x1a4  nw4r::g3d::ResFile mResFile    -- corrected from the scouting pass's "int(=0)", see below
+0x1a8  m3d::mdl_c mModel
+0x1e8  m3d::anmChr_c mAnimChrs[6]     -- 0x1e8 + 6*0x38 = 0x338
+0x338  int mUnk338                    -- untouched by ctor/dtor; existence proven by mProcState landing at +0x33c
+0x33c  int mProcState                 -- execute()'s PTMF index; fn_2_16D7F0/fn_2_16D7C0 also touch it
+0x340  int mUnk340                    -- set by lookupAction() from the (unrecovered) 4-entry table
+0x344  u8 pad344[0x18]                -- untouched; existence proven by mUnk35c landing at +0x35c
+0x35c  int mUnk35c                    -- set to -1 by resetState(); role otherwise unknown
```
`sizeof == 0x360` — 0x35c + 4 = 0x360, closes exactly, matching `daWmKoopaJr_c_classInit__Fv`'s `li r3, 0x360` verbatim (this line was checked and now matches byte-for-byte after the `pad344` fix below).

Also confirmed, all **inherited** (`dBaseActor_c`/`dWmActor_c`) fields, each
pinned by TWO independent call sites (koopajr's own code, and the
already-landed `dWmActor_c::preExecute()`/`preDraw()` for `mPos`/`mClipSphere`):
`mMatrix`@0x7c, `mPos`@0xac, `mScale`@0xdc, `mAngle`@0x100, `mClipSphere.mCenter`@0x128, `mClipSphere.mRadius`@0x134.

### Two real bugs found and fixed during this session (not hand-waved)

1. **`mProcState` was one field short of +0x33c** (landed at +0x338) until
   `mUnk338` was inserted — caught because `execute()`'s
   `lwz r4, 0x33c(r29)` diffed against my `0x338(r29)`. Confirms this is a
   REAL field boundary, not padding folded into `mProcState`.
2. **`mUnk35c` was 0x18 bytes short** (landed at +0x344) until `pad344[0x18]`
   was inserted — caught the same way via `resetState()`'s
   `stw r0, 0x35c(r3)` vs my `0x344(r3)`, and independently corroborated by
   `daWmKoopaJr_c_classInit__Fv`'s `li r3, 0x360` (which had been silently
   producing `0x348` before the fix — the exact size of the missing gap).
3. **`resetScaleAndProc()`'s `mScale` reset used a temporary.** Writing
   `mScale = mVec3_c(0.01f, 0.01f, 0.01f);` compiled to a 5-extra-instruction
   stack-temp-construct-then-copy (13 lines vs the target's 8). Per the
   project's "stack-temp question" rule (no temp observed in target ⇒ direct
   field stores), rewriting as three explicit `mScale.x = mScale.y = mScale.z
   = 0.01f;` statements closed the size gap exactly (8/8) and left only the
   own-rodata-symbol-naming diff.

### `mResFile`@0x1a4 — corrected from the original scouting note

The scouting pass recorded this as a raw `int (= 0)` because the constructor
stores it with a plain `li r0,0; stw r0,0x1a4(r31)` — no `bl __ct__...`
visible. But `nw4r::g3d::ResFile`'s default constructor is a trivial in-class
one-liner (vague linkage), and `-inline noauto` still inlines an in-class
body, so a genuine `nw4r::g3d::ResFile mResFile;` member compiles to exactly
this pattern — indistinguishable from a raw int at the constructor alone.
Confirmed by `createModel()` (fn_2_16D590), which stores
`dResMng_c::m_instance->getRes(...)`'s return value directly into +0x1a4 via
a plain `stw` (consistent with ResFile's trivial 4-byte representation), then
calls `GetResMdl__Q34nw4r3g3d7ResFileCFPCc` with `this = &mResFile` — the
same shape and the SAME offset as the landed sibling
`daWmKinokoBase_c::mResFile`.

## Resource strings recovered for `createModel()` (from `original/d_basesNP.rel` directly)

Read directly out of the REL's `.data` (file offset `0x1D0C00 + addr`) at the
table based at `lbl_2_data_45DD8`, referenced by `fn_2_16D590` via
`r30 = lbl_2_data_45DD8`:

- `+0x78..+0x8c` (6x `const char*`, each relocated `Absolute` into `.data`):
  animation names `"wait"`, `"run"`, `"jump_st"`, `"jumpA"`, `"jump_ed"`,
  `"shock_wmap"` (verified by reading the raw bytes at each relocation
  target, e.g. `lbl_2_data_45E18` = `"wait\0"`).
- `+0x90` = `"g3d/koopaJr.brres"` (inline char array, no relocation — used
  directly as `r30+0x90` in the `getRes` call).
- `+0xa8` = `"koopaJr"` (inline; archive/model name, reused for both `getRes`
  and `GetResMdl`).
- `+0xb0` = `"mask"` (inline; `GetResNode` argument).
- `+0xb8` / `+0xc8` = `"character_SV"` / `"g3d/model.brres"` (inline;
  `CreateShadowModel` arguments — `r6` is a register copy of `r4`, so arg1
  and arg3 are the SAME string, `"character_SV"`, passed twice).

Not recovered: the `lbl_2_rodata_8C38` 4-entry int table read by
`lookupAction()` (`fn_2_16D920`), and the `GetResNode` flag-clear
(`rlwinm r0,r0,0,24,22`) — no header exposes a mutator for that ResNode flags
word. Both are left as explicit gaps in `createModel()`/`lookupAction()`
rather than guessed.

## Caveat on "MATCH" in this document

`difftool.py` is a raw textual diff of two `dtk elf disasm` dumps; it does
**not** perform the project's real symbol-pool canonicalisation (that lives
in `tools/auto_decomp/harness.py`'s `canonicalise()`, written for
`wiimj2d`/DOL-style 8-hex-digit symbols — `d_basesNP`'s REL-style
`fn_2_16D7F0`/`lbl_2_data_45F08` names don't match its regexes, so it would
not collapse these own-symbol differences either). Every "MATCH\*" above is a
reasoned claim (own vtable / own not-yet-split member, same address), not an
outcome verified against the project's actual link-time comparison — I have
not run, and was instructed never to run, `progress.py`/`land.py`.
