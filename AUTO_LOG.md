# Autonomous work log

Append one short entry per unit, newest at the bottom. Never overwrite.

    ## <unit> — <N>/<N> — LANDED-READY | PARKED   <2-3 lines on what happened>

---

## scratch/trial: dPosShake_c — 2/3 — PARKED (trial run)
init and startShake at DIFFS 0 and byte-verified. move is 61/61 words with 20
diffs, all one f1/f2 register mirror between the long-lived local and the
secondary values; branch polarity, cror forms, fneg placement and scheduling
already identical. One declaration-order lever from closing.

## scratch/trial: dRotShake_c — 1/2 — PARKED (trial run)
init at DIFFS 0, including the scrambled eight-argument s16 mapping, derived
unaided. move is 63/63 words with 52 diffs, same register-mirror character.
