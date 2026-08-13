# d_a_player_demo_manager.cpp — section bounds

TU: `daPyDemoMng_c`. Sits in the gap between the banked slices
`dol/bases/d_a_player_base.cpp` and `dol/bases/d_a_player_hio_ADJ.cpp`.

## Method

- Converted every given/derived address to a slice offset using the six
  section bases from the brief, then **independently re-derived all six
  bases from `original/wiimj2d.dol`'s own DOL section header** (the 18
  text/data slot table at file offset 0x00-0xD8, plus `bssAddr`/`bssSize`
  at 0xD8-0xE0). All six matched exactly:
  - `.text` slot 1: addr `0x80006780` ✓
  - `.ctors` slot 9: addr `0x802edce0`, size `0x2e0` ✓
  - `.rodata` slot 11: addr `0x802edfe0`, size `0x106c0`, end `0x802fe6a0` ✓ (== `.data` base)
  - `.data` slot 12: addr `0x802fe6a0`, size `0x532e0`, end `0x80351980` ✓ (== `.bss`/`bssAddr`)
  - `.bss`/`bssAddr` `0x80351980`, `bssSize 0xde59c` ✓
  - `.sdata` slot 13: addr `0x80427980`, size `0x2520`, end `0x80429ea0` — this is
    the **true `.sbss` base** (see below; not given in the brief, derived here).
  - `.sdata2` slot 14: addr `0x8042b360` ✓
- Read `bin/dtk/wiimj2d_symbols.txt` for the symbols flanking every candidate
  range (neighbours pin both ends).
- Ran `tools/datarefs.py original/wiimj2d.dol 8005B3A0 8005D7E0` (our `.text`
  range) to pull every r13/r2/ha-l data reference the TU's own code makes,
  and cross-checked every hit against the symbol table.
- Also ran it over `8005D7E0 8005DFD0` (ADJ's `.text`) for the sections where
  ADJ is the relevant neighbour, and used `bin/dtk-windows-x86_64.exe elf
  info` on `bin/dtkspl/obj/dol/bases/d_ac_py_key.o` (matching, authoritative)
  to confirm what `d_ac_py_key.cpp` really contributes.
- Read raw bytes out of the DOL directly (small python struct script) to
  decode the `.ctors` slot and the pointer tables sitting in our `.data`
  claim, rather than trusting offsets alone.

Confirmed **exactly one `__sinit_`** inside the `.text` range
(`__sinit_\d_a_player_demo_manager_cpp` at `0x8005D750`) — this is one TU,
not several.

---

## Ready-to-paste slice entry

```json
{
  "source": "dol/bases/d_a_player_demo_manager.cpp",
  "nonMatching": true,
  "memoryRanges": {
    ".text": "0x54c20-0x57060",
    ".ctors": "0x80-0x84",
    ".data": "0xb268-0xb3b8",
    ".rodata": "0xec0-0x1120",
    ".bss": "0x35a0-0x35b0",
    ".sbss": "0xd0-0xd8",
    ".sdata2": "0x998-0x9c8"
  }
}
```

(`nonMatching: true` is a placeholder — flip once the code lands and matches;
lead's call. No `.sdata` key — see below, it's a proven-empty claim.)

---

## Per-section derivation and evidence

### `.text`: `0x54c20-0x57060`  (addr `0x8005B3A0-0x8005D7E0`)

This is exactly the gap the brief already handed us between the two banked
neighbours. Independently confirmed via symbol continuity in
`wiimj2d_symbols.txt`:

- Last symbol before our start is `isSameName__25sFStateID_c<10daPlBase_c>CFPCc`
  ending at `0x8005B398`+pad, then `__ct__13daPyDemoMng_cFv` starts at
  `0x8005B3A0` — zero gap, matches base_player's committed end exactly.
- Our last two symbols are `__sinit_\d_a_player_demo_manager_cpp` (`0x8005D750`,
  size `0x70`, ends `0x8005D7C0`) and `__arraydtor$72504` (`0x8005D7C0`, size
  `0x1C`, ends `0x8005D7DC`); `__ct__17dAcPy_HIO_Speed_cFv` (ADJ's first
  function) starts at `0x8005D7E0` — 4 bytes of alignment pad, then ADJ's
  committed start exactly.
- Exactly one `__sinit_` in the range (see above) — single TU confirmed.

**Corroboration: double** (symbol continuity at both ends, matching the
already-committed neighbour boundaries on both sides) **+ single-sinit check.**

### `.ctors`: `0x80-0x84`  (addr `0x802edd60-0x802edd64`)

Base_player claims `0x7c-0x80`, ADJ claims `0x84-0x88` → exactly one 4-byte
slot open, `0x80-0x84`. Read the actual bytes at `0x802edd60` in
`original/wiimj2d.dol`:

```
offset 0x7c -> 0x800592c0   (base_player's sinit)
offset 0x80 -> 0x8005d750   (== __sinit_\d_a_player_demo_manager_cpp, the given anchor!)
offset 0x84 -> 0x8005df00   (== __sinit_\d_a_player_hio_ADJ_cpp)
```

**Corroboration: strong, direct.** The slot's raw content *is* the given
`__sinit` anchor address — this isn't inference, it's a literal read. This is
exactly the class of bug the brief warns is costly to get wrong, and it's
about as nailed-down as it gets.

### `.data`: `0xb268-0xb3b8`  (addr `0x80309908-0x80309a58`)

This is the wrinkle. ADJ claims **no** `.data` at all (given fact), and
`d_ac_py_key.cpp` — confirmed **matching** — was independently checked with
`bin/dtk-windows-x86_64.exe elf info bin/dtkspl/obj/dol/bases/d_ac_py_key.o`:
its ELF has only `.text`, `.comment`, `.note.split` sections — **zero**
`.data`. Since order is preserved across sections in this codebase (verified
against a dozen other consecutive-TU examples pulled from the slice file),
and both flanking TUs in `.text` order between us and `d_a_right_base.cpp`
contribute nothing to `.data`, the *entire* gap between base_player's
committed end (`0xb268`) and right_base's start (`0xb3b8`, confirmed both
from `slices/wiimj2d.json` and independently from
`bin/dtk/dtk_splits_wiimj2d.txt`'s stale-but-authoritative-for-undone split)
belongs to us.

Confirmed piece by piece via `datarefs.py`'s hits and the symbol table:
- `sc_ForceList__6dWmLib` (`.data:0x80309908`, size `0x24`, **scope:local**)
  — referenced at `0x8005c2c4`, inside `setHanabiEffect__13daPyDemoMng_cFv`
  (`0x8005C2B0-0x8005C40C`). `scope:local` = internal linkage = only the
  defining TU can reference it, and that's us.
- `@LOCAL@setHanabiEffect__13daPyDemoMng_cFv@scHanabiOffsetDt@8`
  (`0x8030992C`, size `0x24`) — referenced at `0x8005c2fc`, same function.
  Explicitly named after our own method.
- The string table + `@LOCAL@...@scHanabiEffectID@9` pointer array
  (`0x80309950-0x80309A18`) — `scHanabiEffectID`'s 4 pointers decode (read
  directly from the DOL) to the 4 preceding fireworks-colour strings; part
  of the same local-static group.
- `__vt__13daPyDemoMng_c` (`0x80309A18`, size `0xC`) — the given anchor.
  Read its raw bytes: `00000000 00000000 8005b3e0` — offset-to-top=0,
  RTTI=0 (stripped, standard for this codebase), single vtable slot =
  `0x8005B3E0` = `__dt__13daPyDemoMng_c` exactly. Confirms both the vtable's
  identity and its 0xC size.
- The tail (`0x80309A28-0x80309A58`): two more `scope:local` string
  symbols (`@81204` = `"Wm_mr_vshipattack"`, `@81206` =
  `"Wm_mr_vshipattack_ind"`, read directly from the DOL) plus 6 bytes of
  pad before right_base's vtable. I could not find the instruction that
  references these two strings (no D-form hit in our `.text`, no psq hit
  either, and a whole-binary scan for any 4-byte pointer to either address
  found **nothing** — i.e. not even table-indirect). They own no visible
  code reference. **I'm including them in our claim on elimination grounds
  only** (ADJ = 0 given, py_key = 0 confirmed via matching .o, order is
  preserved, boundary lands exactly on right_base's independently-known
  start with zero slack) — not on a direct reference. Flagging this
  explicitly per the "flag what you couldn't corroborate twice" instruction.

**Corroboration: strong for the first ~0x110 bytes (direct code refs, named
after our own methods), elimination-only for the last ~0x30 bytes
(`0x80309A28-0x80309A58`, the two `vshipattack` strings + pad).** The
end boundary itself (matching right_base's start exactly, confirmed two
independent ways) is solid; what's *not* independently confirmed is that
those specific 0x30 bytes are ours rather than some misattribution — the
elimination logic is airtight given the givens, but a direct reference would
have been better. If this is ever going to bite, it's here.

### `.rodata`: `0xec0-0x1120`  (addr `0x802eeea0-0x802ef100`)

Perfectly packed on both ends, no padding assumptions required at all:
- Starts exactly at base_player's committed end (`0x802eeea0` = offset
  `0xec0`), first symbol `@75115` (size `0x10`) — referenced directly at
  `0x8005b8c8`/`8005b8d4`/`8005b8dc`/`8005b8e4` in our `.text` (function
  `getPoleBelowPlayer__13daPyDemoMng_cFi`, `0x8005B840-0x8005B894`, range
  check confirms).
- `@75140` (`0x802EEEB0`, size `0x10`) — referenced at `0x8005b994`
  etc., same style, next function down (`setGoalDemoList`/`executeGoalDemo_Pole`
  region).
- `@LOCAL@setHanabiEffect__13daPyDemoMng_cFv@scHanabiOffset_1` through `_9`
  (`0x802EEEC0-0x802EF0EC`) — all nine explicitly named after our own
  method. Their addresses (`eec0, eed0, eee8, ef10, ef40, ef80, efc8,
  f020, f080`) exactly match the 9 pointer values stored in
  `scHanabiOffsetDt` (our `.data` claim above) — read directly from the DOL,
  byte for byte.
- `@75781` (`0x802EF0F0`, size `0x10`) ends at exactly `0x802EF100` —
  ADJ's given `.rodata` start (`0x1120`), zero gap, and ADJ's own code
  (scanned `0x8005D7E0-0x8005DFD0`) references `0x802ef100` directly
  eight times (`sc_player_mame__17dAcPy_HIO_Speed_c`, ADJ's own symbol).

**Corroboration: very strong, double.** Direct code references at both ends
from *both* neighbouring TUs' own disassembly, plus the `.data` pointer
table cross-check (independent third confirmation for the middle 0x22c
bytes), plus zero-gap exact alignment to both committed neighbours.

### `.bss`: `0x35a0-0x35b0`  (addr `0x80354f20-0x80354f30`)

- `@72505` (`.bss:0x80354F20`, size `0xC`, scope:local) — referenced
  directly at `0x8005d784` (`addi ... 80354f20 ha/l`), inside our `.text`,
  right next to `__sinit_` (`0x8005D750-0x8005D7C0`) — this is the storage
  for the local static array whose destructor is `__arraydtor$72504`
  (id-paired name, and `__arraydtor$72504` is itself inside our `.text`
  range, `0x8005D7C0`).
- Remaining 4 bytes (`0x80354F2C-0x80354F30`) are unnamed pad before ADJ's
  committed start (`0x80354f30`, matches ADJ's given `.bss` start `0x35b0`
  exactly) — attributed to us by elimination (nothing else can own it).

**Corroboration: strong.** Direct reference + id-paired ctor/dtor symbol
pairing + exact zero-gap match to ADJ's given start.

### `.sbss`: `0xd0-0xd8`  (addr `0x80429F70-0x80429F78`)

Watch this one — I initially mis-derived the `.sbss` base by assuming
base_player's claimed end (`0xd0`) landed exactly on `mspInstance`
(the given anchor). It doesn't. The **true** `.sbss` base, read off the DOL's
own section table, is `0x80429EA0` (= `.sdata`'s slot-13 end address,
confirmed independently as the address of the very first `.sbss` symbol in
the symbol table, `ms_res_allocator__Q23d2d18ResAccMultLoader_c`). With that
base:
- offset `0xc8` → `0x80429F68` (base_player's claimed start — matches a
  `@GUARD@` byte exactly)
- offset `0xd0` → `0x80429F70` = `c_StartPointKinokoHouseID__6dWmLib`
  (size `0x4`, **scope:local**) — base_player's claim *excludes* this
  address (range is exclusive at the end), so it's not base_player's.
  It's directly written by our own `__sinit_` at `0x8005d7ac`
  (`stw ... 80429f70 r13`, inside `0x8005D750-0x8005D7C0`) — this is the
  same per-TU-local-static idiom seen elsewhere in this codebase (the same
  symbol name recurs at a different `.sbss` address for a different TU,
  e.g. `0x80429F30`, `0x80429F70` are two distinct copies).
- offset `0xd4` → `0x80429F74` = `mspInstance__13daPyDemoMng_c` (the given
  anchor, size `0x4`).
- offset `0xd8` → `0x80429F78` = `ms_num_of_instance__17dAcPy_HIO_Speed_c`
  — ADJ's given `.sbss` start, exact match.

So the real claim is 8 bytes (`0xd0-0xd8`): `c_StartPointKinokoHouseID`
+ `mspInstance`, not just the 4-byte `mspInstance` slot I'd have gotten from
naive offset arithmetic.

**Corroboration: strong, and specifically double-checked** because the naive
derivation was wrong once already — the DOL-header-derived base plus the
direct `__sinit_` store instruction are two independent confirmations of the
corrected range.

### `.sdata2`: `0x998-0x9c8`  (addr `0x8042bcf8-0x8042bd28`)

Perfectly packed both ends:
- Starts exactly at base_player's committed end (`0x8042bcf8` = offset
  `0x998`). First symbol `@77479` referenced at `0x8005b4a0`
  (`lfs 8042bcf8 r2`, inside `init__13daPyDemoMng_cFv`).
- All subsequent floats/double through `0x8042BD20` referenced directly at
  `0x8005bb88` through `0x8005d770` (multiple hits across
  `executeGoalDemo_Pole`, `calcGoalCenterPos`, and the `__sinit_` tail).
- 4 bytes of pad (`0x8042BD24-0x8042BD28`), then ADJ's first `.sdata2`
  symbol `@67883` at exactly `0x8042BD28` = ADJ's given start (`0x9c8`),
  zero gap.

**Note for whoever owns ADJ/py_key next:** there's a separate, *unclaimed*
0x38-byte `.sdata2` tail between ADJ's given end (`0x9e8`) and
`d_a_right_base.cpp`'s start (`0xa20`), containing `@81205` (a float) — part
of the same `@81204`/`@81205`/`@81206` id-sequence as the two `vshipattack`
strings in our `.data` claim. This gap is **not** adjacent to our TU (ADJ's
already-claimed block sits between us and it), so it's out of scope here,
but it's a strong hint that ADJ's committed `.sdata2` end is short by that
much, or that `d_ac_py_key.cpp` isn't quite as data-free as its matching `.o`
suggests. Flagging it since it's the same string family as the thing I
couldn't fully pin down in my own `.data` claim.

**Corroboration: very strong.** Direct references at both ends from our own
`.text`, zero-gap exact match on both sides.

### `.sdata`: no claim (empty)

Checked explicitly per the brief's instruction. Derived the real `.sdata`
section bounds from the DOL header: slot 13, `0x80427980-0x80429ea0`
(size `0x2520`). Every r13-relative address our own `.text` touches
(`datarefs.py` over `8005B3A0-8005D7E0`) is `>= 0x80429f70`, i.e. **all**
of them fall inside `.sbss` (which starts at `0x80429ea0`), and **none**
fall inside the `.sdata` range. No `.sdata` claim needed — this is a
genuine empty result, not a gap I didn't look for.

**Corroboration: direct** (exhaustive scan of every r13-relative reference
in our own code, checked against the header-derived section boundary).

---

## Summary of confidence

| Section | Confidence | Notes |
|---|---|---|
| `.text` | high | symbol continuity both ends + single-sinit check |
| `.ctors` | high | raw byte read matches given anchor exactly |
| `.rodata` | high | direct refs both ends, cross-checked via `.data` pointer table |
| `.bss` | high | direct ref + id-paired ctor/dtor |
| `.sbss` | high | corrected once via DOL-header base; now double-checked |
| `.sdata2` | high | direct refs both ends, zero gap |
| `.sdata` | high | exhaustive negative scan |
| `.data` | **medium for the tail** | first ~0x110 bytes solid (direct refs); last 0x30 bytes (`0x80309A28-0x80309A58`, the two `vshipattack` strings) are elimination-only — no direct code reference found anywhere in the binary. Boundaries themselves (start and end) are rock solid; ownership of those specific 48 bytes is the one thing I'd flag as worth a second pass if anything about this TU comes back with scattered diffs. |
