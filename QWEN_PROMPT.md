# Work order — dWmBgmSync_c

**Read `AGENT_CONTEXT.md` first.**

Write results to **`QWEN_RESPONSE.md`** in the repository root (overwrite it).

---

## GXStateSave_c LANDED — 4/4

    ACCEPTED -- all five binaries are byte-identical to the original.
    Total: Decompiled 741112/6500368 code bytes (11.401%)

You went from 0/4 to 4/4 in two rounds. All three fixes I gave you worked
exactly as predicted, and your headline this time was the true number. That is
the report I can act on without re-deriving anything.

**Two things you got right that are worth naming.** You derived the symbol
`s_cacheGX__Q23EGG7StateGX` correctly from the disassembly — that global is real
and your name for it was exact. And your `EGG::StateGX` diagnosis held up under
the link. Landing took three attempts, but **none of them was your code** — the
source was byte-exact from the first try. All three were missing addresses for
library functions nobody has decompiled yet, which is my job, not yours.

---

## Your unit: `dol/bases/d_wm_bgm_sync.cpp` — five functions, 0x80102DB0-0x80103020

This is a music-synchronisation class for the world map. It is a good fit for
you: flat arithmetic, no vtable manipulation in the bodies, and **the class
header already exists and is 90% correct**.

    268 B  execute__12dWmBgmSync_cFv          0x80102DB0
     76 B  getAnmRate__12dWmBgmSync_cFf       0x80102EC0
    116 B  fn_80102F10                        0x80102F10   (file-static helper)
    112 B  __sinit_\d_wm_bgm_sync_cpp         0x80102F90   (see below -- free)
     28 B  __arraydtor$53530                  0x80103000   (also free)

Target listing: `tools/auto_decomp/work/dol_bases_d_wm_bgm_sync/target.txt`
**It contains neighbours** (`dWmEffectManager_c` and others) — ignore them.
Also ignore `__arraydtor$65837` at the very top; that one belongs to the
*previous* unit, not yours.

Work in **`scratch/qwen_bgm/`**. I have set up `target.txt`, a `build.py` with
the interface you know, and a `shadow/` directory.

    python -c "import sys; sys.path.insert(0,'scratch/qwen_bgm'); import build; print(build.build('NAME.cpp','NAME','execute__12dWmBgmSync_cFv'))"
    python tools/auto_decomp/fndiff.py scratch/qwen_bgm/target.txt scratch/qwen_bgm/NAME.txt --all

---

## The last two functions are FREE. Do not write them.

`__sinit` and `__arraydtor$53530` construct and destroy a file-scope static
called `dWmLib::sc_ForceList`. **That static is already defined in
`include/game/bases/d_wm_lib.hpp`**, like this:

    static ForceInCourseList_t sc_ForceList[] = {
        {WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0",
         mVec3_c(2160.0f, -30.0f, -478.0f)}
    };
    static int c_StartPointKinokoHouseID = dCsvData_c::c_START_ID;

Because it is declared `static` **inside a header**, every `.cpp` that includes
that header gets its own private copy — and the compiler emits a constructor
call, a `__register_global_object`, and a matching array destructor for it,
automatically. That is exactly what those last two functions are.

So: **`#include <game/bases/d_wm_lib.hpp>` and they appear by themselves.** Do
not hand-write them and do not try to reproduce the float constants; they are
the `mVec3_c(2160.0f, -30.0f, -478.0f)` above, and they are already right.

Verification that this reading is correct: both array destructors do
`li r5, 0x24` (element size — `ForceInCourseList_t` is 0x24 bytes) and
`li r6, 0x1` (exactly one element), matching the single-entry initialiser.

---

## The header needs one fix, and it is the only layout work in this unit

`include/game/bases/d_wm_bgm_sync.hpp` already exists and names almost every
member correctly. One field is wrong:

    u8 mPad2[0x8];          // declared as padding at 0x10-0x17

**Those eight bytes are two live floats.** The evidence:

    stfs f0, 0x10(r31)      execute, reset to 0.0f
    lfs  f1, 0x10(r31)      execute, read back
    stfs f1, 0x14(r31)      execute, store
    stfs f0, 0x14(r31)      execute, second branch

Replace `u8 mPad2[0x8]` with two `f32` members at 0x10 and 0x14. The size is
unchanged, so this is safe. Name them for what they do — 0x10 accumulates
elapsed frames since the last beat, 0x14 holds frames remaining until the next
one.

**Copy the header into `scratch/qwen_bgm/shadow/game/bases/d_wm_bgm_sync.hpp`
and edit it there.** Do not edit `include/` — I promote headers.

Every other offset the header names (0x4, 0x8, 0xc, 0xd, 0xe, 0x18) is already
correct. Do not change them.

---

## What the three functions do

### `execute()` — frame 0x10, saves r31 only, returns a value

Calls `dAudio::getBgmBeatTrg()`, `fn_80102F10` (twice), `dAudio::getBgmAccentSign()`.

    1. if (m_18 == nullptr) return 0;
    2. m_0c = false;
       if (m_0e) return 1;                    // early out
    3. if (dAudio::getBgmBeatTrg()) {
           if (m_08 > 0) { m_08--; if (m_08 == 0) m_04++; }
           if (m_04 == *m_18) {               // lha -- m_18[0] is s16
               m_0c = true; m_04 = 0; m_10 = 0.0f;
               m_14 = fn_80102F10();
           } else {
               m_10 += 1.0f;
               m_14 = fn_80102F10() - m_10;
           }
       }
    4. m_0d = dAudio::getBgmAccentSign();
    5. return 1;

**Note the declared return type.** The header currently says
`virtual void execute();` but the body returns 0 and 1 in r3. Check the retail
listing and fix the return type if it really is not `void` — a `void` function
will not emit those `li r3` values.

### `getAnmRate(f32)` — frame 0x20, saves r31 and f31

    return frameCount / fn_80102F10();

It also calls `dAudio::getBgmTempo()` and **discards the result**. That call is
real and must be there, even though nothing reads it. Do not optimise it away;
if your version drops it, find a form that keeps it.

`f31` is callee-saved because `frameCount` has to survive the call — the
prologue does `fmr f31, f1` immediately.

### `fn_80102F10` — frame 0x20, saves r31, no FPRs

Straight-line, no branches. Reads `m_18[0]` and calls `dAudio::getBgmTempo()`,
returns a `f32` "frames per beat".

    tempo    = getBgmTempo() & 0xFFFF        // clrlwi r3, r3, 16
    result   = 3600.0f * ... / ...           // see the listing for exact order

**Two int-to-float conversions in here cost you nothing to get right if you just
write the casts.** The `0x43300000` stores and the magic-constant subtractions
are MWCC's standard int-to-double idiom, not something to reproduce by hand —
write `(f32)` casts and let the compiler emit them. See AGENT_CONTEXT on this.

It is called from all three sites in this unit and nowhere else, so it is
private. Declare it as a private member function (it takes `this`).

---

## Order of work

1. **Copy and fix the header** (the two floats, and the `execute` return type).
2. **`fn_80102F10`** — everything else calls it, so close it first.
3. **`getAnmRate`** — 76 bytes, one division.
4. **`execute`** — the biggest, and the only one with branching.
5. Add the `d_wm_lib.hpp` include and confirm `__sinit` and `__arraydtor` appear
   and match.

**Function definition order is part of the object.** Define them in address
order: `execute`, `getAnmRate`, `fn_80102F10`.

**Compile or it did not happen.** Every function you report gets a real object
and a real `fndiff.py` line.

## Acceptance

- **The true count at `DIFFS 0` out of 5 as your headline.** Include `__sinit`
  and `__arraydtor$53530` in that count — if the include trick works they are
  free, and if it does not I need to know.
- For any miss: the `-v` output, words / frame / GPR / FPR both sides, and one
  sentence on what you think is wrong.
- The corrected header, and a note of exactly what you changed in it.
- A landing readiness statement.

You do **not** need to supply addresses for `dAudio::getBgmBeatTrg`,
`getBgmAccentSign`, `getBgmTempo`, `__register_global_object`, `__destroy_arr`
or the `ForceInCourseList_t` destructor. I have them and I will add the
`syms.txt` entries when I land. Do not touch `syms.txt`.

Do not run `ninja`, `configure.py`, `progress.py` or `land.py`. Work only in
`scratch/qwen_bgm/`. Do not modify `include/**` or `source/**`.
