# `d_line_mng` landing plan

Prepared **without running the build** — another agent owned the build when this
was written. Everything here is derived from `bin/dtk/dtk_splits_wiimj2d.txt`,
`bin/dtk/wiimj2d_symbols.txt`, `wip/line_mng_shared/target.txt` and a
`harness.compile_draft` of the live draft. The `.text` range is exact; the rest
is **bounded, not pinned** — see section 4 before using it.

State at time of writing: **181/182 functions, 7531/7631 words = 98.7% by bytes.**
Only `line_cross_chk2` (100w) is unmatched, and it is a documented bounded
negative (schedule of one pool load; ~35 measured variants). The unit is
landable as-is; landing does not require closing it.

## 1. Files

| From | To |
|---|---|
| `wip/fix_bigtwo/d_line_mng.cpp` | `source/dol/bases/d_line_mng.cpp` |
| `wip/fix_bigtwo/shadow_include/game/bases/d_line_mng.hpp` | `include/game/bases/d_line_mng.hpp` |

**The header story is clean.** Diffing the whole shadow tree against `include/`
reports exactly one entry, `NEW game/bases/d_line_mng.hpp`. No shared header is
modified, so AGENT_CONTEXT rule 2 ("verify a shared-header change alone") does
not apply to this landing. The draft already compiles with the real `include/`
tree plus this one file, which is what `tally.py` has been doing all along.

## 2. Linkage

`fn_800C3B20`, `fn_800C3B60`, `fn_800C31C0` and `fn_800C1EE0` ship **`static`**.
The `.fn ..., global` tag in `target.txt` was previously read as contradicting
this; it does not — every one of the 182 `.fn` lines carries it and it is
provably wrong for weak symbols. See AGENT_CONTEXT, "the `global` tag in
`target.txt` is not a linkage signal".

## 3. `.text` — EXACT

Every instruction address in `target.txt` was enumerated: 7,631 words spanning
**0x800C0DC0 .. 0x800C8994** inclusive, contiguous apart from 137 inter-function
alignment gaps of 4–12 bytes (16-byte function alignment). The next landed unit,
`dol/bases/d_lytbase.cpp`, starts at **0x800C89A0**, so the 8 bytes after our
last instruction are trailing padding.

Slice offsets are relative to `.text` base `0x80006780` (from
`slices/wiimj2d.json` → `meta.sections`):

    ".text": "0xba640-0xc2220"        # VA 0x800C0DC0 - 0x800C89A0

Existing slices abut (`d_lytbase` ends exactly where `d_lyttextbox` begins), so
claiming through to `0xc2220` follows the file's own convention. If a build
rejects it, `0xc2218` (VA 0x800C8998, our last instruction only) is the
conservative alternative.

## 3a. ATTEMPTED AND REVERTED — every slice range is CONFIRMED CORRECT

The landing was attempted and reverted; the tree is green. **The slice arithmetic
in this document was proven right and needs no further work.** All seven ranges
below were computed independently and every one of the six shared with
`d_lytbase` abuts its start exactly — a coincidence across six sections is not
possible:

    ".text":   "0xba640-0xc2220"     VA 0x800C0DC0-0x800C89A0
    ".ctors":  "0x118-0x11c"         VA 0x802EDDF8-0x802EDDFC
    ".rodata": "0x3308-0x3338"       VA 0x802F12E8-0x802F1318
    ".data":   "0x18600-0x19098"     VA 0x80316CA0-0x80317738
    ".sbss":   "0x3d0-0x3d1"         VA 0x8042A270-0x8042A271
    ".bss":    "0x7780-0x7de0"       VA 0x80359100-0x80359760
    ".sdata2": "0x17b8-0x1878"       VA 0x8042CB18-0x8042CBD8

How each was pinned, so none of this has to be redone:
- `.text` — enumerated all 7,631 instruction addresses.
- `.ctors` — 11 `__sinit` functions in the un-landed hole for exactly 11 free
  4-byte slots; `.ctors` follows link order, and `d_line_mng` is last.
- `.rodata` — the two static `d_unit` arrays from `is_unit_circle2x2` /
  `is_unit_circle4x4` were found byte-for-byte in the retail DOL at 0x802F12E8.
- `.data`, `.bss`, `.sbss`, `.sdata2` — from the addresses embedded in
  `target.txt`'s own symbol names, cross-checked against the emitted sizes.

**What actually blocked it:** `dLineMng_c::smc_UNIT_SIZE_X` is declared but never
defined, and adding the definition changes codegen. See AGENT_CONTEXT,
"the static-const-float trap". That is the only remaining blocker, and it is a
source question, not a slice question.

Also needed once that is solved: `__ct__7mVec2_cFv` must not be placed from this
object — retail takes it from 0x8007F800 in an un-landed unit. Expect a
`deadstrip` entry plus a `syms.txt` pin, the DOL analogue of the `alias_db.txt`
line that rescued `d_a_ac_switch`.

## 4. Every other section — superseded by 3a, kept for the derivation

`d_line_mng` is **not** the only un-landed unit in its region. The `.text` hole
between `d_enemy_state.cpp` and `d_lytbase.cpp` is 0x20290 bytes and also
contains at least `d_enemy_toride_kokoopa` (0x800A8710–0x800B0A20, Gemini's
unit) and `d_bg_actor_mng` (Qwen's). So the free span in each *data* section is
shared, and this unit's share cannot be read off the gap alone.

Free span between those two landed neighbours, and this draft's own emitted
size:

| section | free span (VA) | span size | draft `.o` size |
|---|---|---:|---:|
| `.ctors`  | 802EDDD0 – 802EDDFC | 0x2c   | 0x4   |
| `.rodata` | 802F0AC0 – 802F1318 | 0x858  | 0x30  |
| `.data`   | 80314360 – 80317738 | 0x33d8 | 0xa98 |
| `.bss`    | 80358438 – 80359760 | 0x1328 | 0x660 |
| `.sdata`  | 80427D10 – 80427F08 | 0x1f8  | —     |
| `.sdata2` | 8042C6E8 – 8042CBD8 | 0x4f0  | 0xbc  |
| `.sbss`   | — | — | 0x1 |

**One hard anchor exists in `.sdata2`:** the four pool constants this unit loads
are at 0x8042CB1C (0.0f), 0x8042CB48 (16.0f), 0x8042CB4C (−0.1f) and 0x8042CB50
(+0.1f), all decoded with `tools/auto_decomp/pool.py` and all matching the
draft's literals. They sit inside the free span, so the unit's `.sdata2` must
cover at least 0x8042CB1C–0x8042CB54. With an emitted size of 0xbc, a run
starting at 0x8042CB1C would end at 0x8042CBD8 — exactly the neighbour boundary.
That is a clean fit and the first candidate to try, but it is **inference, not
measurement**: do not commit it without a build.

## 5. Do NOT read the `.text` size as an overflow

The draft `.o` emits **0x7e68** of `.text` against a 0x7bd8 region. This is
expected and is not a link overflow. Unreferenced weak symbols — template
instantiations and inline bodies — occupy space in the object file and are never
placed by the linker. Judging a slice claim by the object's section size is a
known false alarm; judge it by what the link actually places.

`.mwcats.text` (0x628) is likewise not placed.

## 6. Sequencing

`tools/auto_decomp/land.py` does the file moves, the slice insertion (in `.text`
order, refusing if it cannot place it) and the verification, in one step:

    python tools/auto_decomp/land.py --unit dol/bases/d_line_mng.cpp \
        --cpp wip/fix_bigtwo/d_line_mng.cpp \
        --hpp wip/fix_bigtwo/shadow_include/game/bases/d_line_mng.hpp \
        --slice '{".text": "0xba640-0xc2220", ...}'

It runs `ninja && python progress.py --verify-bin` and restores every file it
touched if the binaries do not come out hash-identical. **It must not be run
while another agent owns the build.**

**Land this only after the tree is green.** Two REL verification failures
(`d_profileNP`, `d_basesNP`) were open when this was written and are being
worked separately; both were proven pre-existing rather than caused by recent
work. Adding a new DOL unit on top of an already-red tree would make it
impossible to attribute a new failure.
