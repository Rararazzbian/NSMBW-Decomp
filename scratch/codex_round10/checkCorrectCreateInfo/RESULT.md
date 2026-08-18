# checkCorrectCreateInfo result

## Final source

The final source is the unchanged whole-TU shadow copy:

`scratch/codex_round10/checkCorrectCreateInfo/baseline.cpp`

The function body is:

```cpp
void daPyMng_c::checkCorrectCreateInfo() {
    for (int idx = 0; idx < 4; idx++) {
        if (mPlayerType[idx] > 3) {
            mPlayerType[idx] = (PLAYER_TYPE_e) 0;
        }
        PLAYER_POWERUP_e mode = mPlayerMode[mPlayerType[idx]];
        if (mode > 6) {
            mPlayerMode[mPlayerType[idx]] = (PLAYER_POWERUP_e) 0;
        }
        if (mCreateItem[mPlayerType[idx]] & 0xE) {
            mCreateItem[mPlayerType[idx]] = mode & 1;
        }
        if (mRest[mPlayerType[idx]] < 0 || mRest[mPlayerType[idx]] > scRestMax) {
            mRest[mPlayerType[idx]] = 5;
        }
    }
    if (mKinopioMode > 6) {
        mKinopioMode = (PLAYER_POWERUP_e) 0;
    }
    if (getCoinAll() > scCoinMax) {
        mCoin[mPlayerType[0]] = 0;
        mCoin[mPlayerType[1]] = 0;
        mCoin[mPlayerType[2]] = 0;
        mCoin[mPlayerType[3]] = 0;
    }
    if (mScore > scScoreMax) {
        mScore = scScoreMax;
    }
}
```

## Measurement

Whole-`assembled.cpp` compile, followed by disassembly and harness diff:

- Target: 105 instructions, 0x1A4 bytes
- Draft: 103 instructions, 0x19C bytes
- Result: NEAR-MISS

The constant loads are present and hoisted in the baseline. The meaningful
remaining code difference is in the clamp loop. The target loads the rest value
into `r0`, compares it, and recomputes the scaled player-type index in the
clamp store path. The draft retains the scaled index in `r0`, loads the rest
value into another register, and stores through the retained index. The target
therefore has two extra instructions per relevant loop structure overall.

Other reported differences are register allocation, unsigned-versus-signed
compare opcode selection, and the unlinked `m_playerID` relocation name. The
latter is the known naming artifact and is not counted as a defect.

## Constant-folding question

Resolved by measurement in the whole-TU compile:

- Plain non-`const` file-scope objects produce the target-style hoisted
  `lwz ...@sda21` loads. This is the current assembled source.
- `const int` objects fold to immediates and remove the target-style loads.
  The function shrinks to 99 instructions.
- `volatile int` objects force loads, but are too conservative. The compiler
  emits repeated loads instead of the target's hoisted/reused load and produces
  106 instructions.

Therefore the constants must remain plain non-`const` file-scope objects.
Changing their declarations does not resolve the two-instruction mismatch.

## Confidence

High confidence in the constant-folding conclusion and the measured near-miss
characterisation. Low confidence that a source-only change can force the exact
clamp-loop scheduling without changing surrounding declarations or compiler
context. No source change is proposed because the tested alternatives either
shrank the function or overshot it.
