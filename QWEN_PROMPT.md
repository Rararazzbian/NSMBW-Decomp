# Work order — dWmBgmSync_c, closing round

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** in the repository root (overwrite it).

---

## You are at 3 of 5, not 2 of 5 — and one of my instructions was wrong

You reported 2/5. The real number is **3/5**. `__arraydtor$53530` also matches:
I compared it instruction by instruction against yours and every single one is
identical. The only differences are generated symbol names — the serial
(`$53530` vs `$11200`), the pool constants (`"@58120_8042D468"` vs `"@16759"`),
and the address suffix on `sc_ForceList__6dWmLib`. All naming artifacts, none
real. `fndiff` did not pair the two because the serials differ, so it silently
compared four functions and said nothing about the fifth. Worth knowing: **a
function fndiff does not mention is unchecked, not passing.**

    execute        DIFFS 48    67 words / frame 0x10 / GPR [31]
    getAnmRate     DIFFS 0
    fn_80102F10    DIFFS 19    29 words / frame 0x20 / GPR [31]
    __sinit        DIFFS 0     (+10 naming artifacts)
    __arraydtor    matches instruction for instruction

**Your `#include` reading of the static initialiser was exactly right.** Both
`__sinit` and `__arraydtor` came out byte-exact without you writing a line of
either. That was the part of the unit I was least sure about.

### I gave you a wrong instruction and you were right to ignore it

My brief said the counter logic was nested:

    if (m_08 > 0) { m_08--; if (m_08 == 0) m_04++; }

**That was wrong.** You wrote it sequentially and your version is what retail
does. The proof is that the reload sits on a shared label reached from both
paths:

    80102E00  ble .L_80102E0C        <- skips the decrement
    80102E04  subi r0, r3, 0x1
    80102E08  stw r0, 0x8(r31)
    .L_80102E0C:
    80102E0C  lwz r0, 0x8(r31)       <- reached whether or not it decremented
    80102E10  cmpwi r0, 0x0
    80102E14  bne .L_80102E24        <- an independent second test

A nested `if` would put the second compare inside the first branch. **Keep your
two sequential `if`s exactly as they are.** None of the 48 diffs come from there.

---

## `execute` — three independent fixes

### Fix 1: `getBgmBeatTrg` returns `u8`, not `bool`

    target:  bl getBgmBeatTrg__6dAudioFv        draft:  bl getBgmBeatTrg__6dAudioFv
             clrlwi. r0, r3, 24                         cmpwi r3, 0x0
             beq .L_80102E24                            beq .L_000000DC

MWCC trusts a `bool` return to be already 0-or-1 and tests it directly. For any
other type it masks the low byte first, which is the `clrlwi.` retail shows.
Your own `getBgmAccentSign` is declared `u8` and is masked correctly — the two
should match.

    - bool getBgmBeatTrg();
    + u8 getBgmBeatTrg();

### Fix 2: no early return — one return at the end

    target:  bne .L_80102EA4        <- jumps to the SAME label the normal end uses
    draft:   beq .L_00000044
             li r3, 0x1
             b .L_000000F8          <- a second, duplicated return

Retail has a single return that both paths share. Your `return true;` inside the
`m_0e` check creates a second one. Invert the test and wrap the body instead:

    - m_0c = false;
    - if (m_0e) {
    -     return true;
    - }
    - if (dAudio::getBgmBeatTrg()) {
    -     ...
    - }
    - m_0d = dAudio::getBgmAccentSign();
    - return true;

    + m_0c = false;
    + if (!m_0e) {
    +     if (dAudio::getBgmBeatTrg()) {
    +         ...            // body unchanged, keep the two sequential ifs
    +     }
    +     ...                // see Fix 3
    + }
    + return true;

### Fix 3: `m_0d` is written by an explicit if/else, not a bool conversion

    target:  clrlwi. r0, r3, 24        draft:  clrlwi r4, r3, 24
             beq .L_80102E9C                   li r3, 0x1
             li r0, 0x1                        neg r0, r4
             stb r0, 0xd(r31)                  or r0, r0, r4
             b .L_80102EA4                     srwi r0, r0, 31
             li r0, 0x0                        stb r0, 0xd(r31)
             stb r0, 0xd(r31)

`neg` / `or` / `srwi` is MWCC's branchless "is this non-zero" idiom, which is
what a plain assignment produces. Retail branches and stores a literal on each
side, which is what a written-out if/else produces.

    - m_0d = dAudio::getBgmAccentSign();

    + if (dAudio::getBgmAccentSign()) {
    +     m_0d = true;
    + } else {
    +     m_0d = false;
    + }

---

## `fn_80102F10` — the two operands are the wrong way round

This is one fix, but it has two parts and both matter.

**The multiplicand and the divisor are swapped.** Retail multiplies 3600 by
`*beat` and divides by the tempo. You have it the other way round:

    target:  fsubs f1, f0, f1        ; f1 = (f32)*beat
             lfs   f0, "@58098"      ; 3600.0f
             fmuls f0, f0, f1        ; 3600.0f * *beat
             fsubs f1, f2, f3        ; (f32)(tempo & 0xffff)
             fdivs f1, f0, f1        ; divide by the tempo

**The two conversions have different signedness, and the magic constant proves
which is which.** Retail biases `*beat` with `xoris r0, r0, 0x8000` and decodes
it against `@58101` — the signed idiom, which is what you get for free from an
`s16`. It does *not* bias the tempo, and decodes it against `@58103` — the
**unsigned** idiom, which needs an explicit `(u32)`. Your version applies the
signed form to both.

    - return 3600.0f * (f32)(tempo & 0xffff) / (f32)*beat;
    + return 3600.0f * (f32)*beat / (f32)(u32)(tempo & 0xffff);

Do not hand-roll the conversions — write the casts and let MWCC emit the
`0x43300000` idiom. It already does this correctly for you; the only thing wrong
was which cast went where.

---

## The two misses do NOT share a cause

`execute`'s three fixes are all about control-flow shape and declared types.
`fn_80102F10`'s is an arithmetic expression bug. Fixing one will not move the
other, so check both counts separately after rebuilding.

## Acceptance

- Rebuild and re-score with `--all`.
- **The true count at `DIFFS 0` out of 5 as your headline.** Count
  `__arraydtor` as matched — it already is, and if `fndiff` still does not
  mention it, say so rather than leaving it out.
- For any remaining miss: the `-v` output, words / frame / GPR / FPR both sides,
  and one sentence on what you think is left.
- The final header, and exactly what you changed in it.
- A landing readiness statement.

If one of my fixes does not do what I said, **say so plainly.** I was wrong
about the nested `if` this round and your version was correct; I would rather be
told than have you work around it silently.

You do not need addresses for `dAudio::*`, `__register_global_object`,
`__destroy_arr` or the `ForceInCourseList_t` destructor — I have them and will
add the `syms.txt` entries when I land. Do not touch `syms.txt`.

Do not run `ninja`, `configure.py`, `progress.py` or `land.py`. Work only in
`scratch/qwen_bgm/`. Do not modify `include/**` or `source/**`.
