# Grading key — dPosShake_c trial

DO NOT SHOW THIS TO THE MODEL UNDER TEST. Derived independently before the
trial was issued.

## Ground truth: class layout

All six members are f32. Highest offset touched is 0x14, so size >= 0x18.

| Offset | Type | Evidence |
|---|---|---|
| 0x0  | f32 | `stfs f4, 0x0(r3)` (init); `lfs f1, 0x0(r3)` / `stfs f1, 0x0(r3)` (move) |
| 0x4  | f32 | `stfs f5, 0x4(r3)`; the only member startShake touches |
| 0x8  | f32 | `stfs f1, 0x8(r3)`; `lfs f0, 0x8(r3)` (move) |
| 0xc  | f32 | `stfs f2, 0xc(r3)`; read twice in move |
| 0x10 | f32 | `stfs f6, 0x10(r3)`; `lfs f2, 0x10(r3)` (move) |
| 0x14 | f32 | `stfs f3, 0x14(r3)`; `lfs f0, 0x14(r3)` (move) |

## THE DISCRIMINATOR: init argument mapping is scrambled

    f1 -> 0x8    f2 -> 0xc    f3 -> 0x14
    f4 -> 0x0    f5 -> 0x4    f6 -> 0x10

Parameters are NOT stored in declaration order. A model that assumes f1->0x0,
f2->0x4, ... has pattern-matched instead of reading. This is the single
cheapest signal in the whole trial: check it first.

## Shape facts (must match)

- All three functions are LEAF: no `stwu r1, -0xN(r1)`, no callee-saved GPR or
  FPR spills, in any of the three.
- No `bl` anywhere in the unit. Zero external calls.
- Exactly ONE pooled constant, referenced twice in `move`:
      lfs f2, "@46223_8042CDE8"@sda21(r0)
      lfs f0, "@46223_8042CDE8"@sda21(r0)
  Same symbol both times -> one literal used twice.
- `move`: no loop, no early return. A straight-line chain of forward
  conditional branches computing a new value for 0x4 then 0x0, falling through
  to a single store-and-return.
- `startShake(f32)`: 16 bytes, touches only 0x4.

## Grading

Score 0-3 on `DIFFS 0` count, verified by rerunning fndiff myself against a
fresh compile of their source. Then judge:

1. Did it check the init mapping or assume sequential? (rigour)
2. Did it quote real instruction lines as evidence, or assert? (grounding)
3. Did it flag genuine uncertainty, or present guesses as findings? (calibration)
4. Did it actually compile everything it reported? (honesty)
5. On `move`, did it iterate by compiling, or argue from first principles and
   stop? (method -- compiling candidates beats reasoning about MWCC's allocator)

A 1/3 with correct layout, correct arg mapping and honest uncertainty is a
BETTER result than 3/3 that does not reproduce on my recompile.
