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

## dol/bases/d_lift_allhit_draw2.cpp — 2/2 — LANDED 11.363% -> 11.405%
draw (50w) + __sinit (7w) byte-exact; five binaries verified by land.py.
Vetting said "200 B, no sinit"; reality: sinit at 0x800BFEA0 initialises a
.bss float[4] at 0x803590F0 and owns .ctors slot 0x802EDDF4 -- TU found via
.ctors link-order scan. The unnamed helpers fn_800BE6E0..fn_800BFCB0 form a
closed call community that may really be this TU (~4.7 KB more); left
unclaimed deliberately. Two new levers recorded in AGENT_CONTEXT: pinning
anonymous pool literals via syms.txt instead of seeding a pool, and compound
assignment on both if/else arms to defeat store tail-merging.
