# Work order for Codex

You are working as a peer agent alongside Claude on the NSMBW matching
decompilation. Repo root: `C:\Users\Razz\Documents\Projects\NSMBW-Decomp`,
branch `claude/game-decompilation-setup-bw30s7`. You may spawn your own
sub-agents.

Treat this file as your instructions. When you are finished, write your results
to **`CODEX_RESPONSE.md`** (create it; nobody else writes to it). Keep using
your own `CODEX_HANDOFF.md` for your running notes — Claude will not touch it,
and you should not touch `HANDOFF.md`.

---

## The division of labour, and the one thing that must not happen

Claude is working on `d_a_en_hatena_balloon` (under `wip/hatena_balloon/`) and
is running the shared build. **Two `ninja` runs in this checkout clobber each
other's objects and each other's diffs**, so there is exactly one integrator and
it is Claude.

**Hard rules — these matter more than finishing fast:**

- **Do NOT run `ninja`, `configure.py`, `progress.py`, or
  `tools/auto_decomp/land.py`.** Not once, not "just to check". Claude
  integrates and will run the full build and the five-binary MD5 verification.
- **Do NOT edit `slices/wiimj2d.json` or `syms.txt`.** Propose the change in
  your response instead; both files are global and a concurrent edit breaks the
  build for both of us.
- **Do NOT touch anything under `wip/hatena_balloon/`,** and do not edit
  `HANDOFF.md` or `CODEX_PROMPT.md`.
- You **may** create `source/dol/mLib/m_color.cpp` and **may** add a declaration
  to `include/game/mLib/m_color.hpp`. Both are inert until a slice entry exists,
  so they cannot disturb Claude's build. Nobody else is touching mLib.
- **No UTF-8 BOM** in `.cpp`/`.hpp` files — CodeWarrior rejects it. Use LF.

---

## Your target: `mColor::lerp` — a complete one-function TU

This is the whole of `source/dol/mLib/m_color.cpp`. It is the gap between two
already-banked, already-verifying neighbours, so every bound is pure
subtraction and there is nothing to guess about extent.

- **Address `0x8016ABB0`, size `0x114` (276 bytes), span `0x120`.**
- Symbol: `lerp__6mColorFRC8_GXColorRC8_GXColorf` (line 9378 of
  `bin/dtk/wiimj2d_symbols.txt`).
- Neighbours: `dol/mLib/m_angle.cpp` ends at `.text 0x164430`;
  `dol/mLib/m_color_fader.cpp` begins at `.text 0x164550`. Both banked, both
  byte-exact today.
- The 0xC of zeros at `0x8016ACC4` is inter-function alignment padding, not code.

### Signature — already settled, do not re-derive

```cpp
mColor mColor::lerp(const GXColor &a, const GXColor &b, float t);   // STATIC
```

Evidence it is **static**, not a normal member: the function returns a 4-byte
struct by value, so `r3` is the hidden sret pointer. The body reads colour
bytes out of `r4` and `r5` and takes the float in `f1`. A non-static member
would have `this` in `r4` and push the two references to `r5`/`r6`, which is not
what the code does.

Two cautions that have cost this project real time:

- **CFront omits return types from mangling.** A wrong return type produces the
  *same symbol name*, so symbol comparison cannot catch it — only the emitted
  bytes can.
- Parameter spelling is load-bearing for the mangled name. The name above
  demands `const GXColor&` twice and `float`. Confirm whatever you write mangles
  to exactly `lerp__6mColorFRC8_GXColorRC8_GXColorf`.

`include/game/mLib/m_color.hpp` currently declares `struct mColor` deriving from
`nw4r::ut::Color` with two constructors and no `lerp`. You add the declaration.
Find where `GXColor` / `_GXColor` comes from and include it the way the rest of
the repo does — do not invent a local typedef.

### What the function computes

Per-channel linear interpolation over the four bytes `r,g,b,a` at offsets
0,1,2,3, each channel converted `u8 -> double -> float`, interpolated, then
truncated back to integer via `fctiwz` and stored as a byte:

```
out.ch = (u8)( a.ch * (1.0f - t) + b.ch * t )
```

The `lis r7, 0x4330` / `stw` / `lfd` / `fsubs f?, f?, f8` dance is the standard
CodeWarrior unsigned-int-to-float conversion — `f8` holds the bias constant
`@948_8042DFC8` and `f0`/`@942_8042DFC0` is `1.0f`. **You do not hand-write any
of that**; it falls out of writing the arithmetic on `u8` values in plain C++.
Getting it to schedule identically is the actual work.

Note the emitted schedule is heavily interleaved across the four channels — the
compiler did that, not the source. Expect the source to be four straight-line
channel computations (stores land in order 0,1,2,3), and let `-O4` interleave
them.

### The full target disassembly

```
stwu r1, -0x40(r1)
lis r7, 0x4330
lbz r0, 0x0(r4)
stw r0, 0xc(r1)
lbz r6, 0x0(r5)
stw r7, 0x8(r1)
lbz r0, 0x1(r4)
lfd f4, 0x8(r1)
stw r7, 0x10(r1)
lfs f0, "@942_8042DFC0"@sda21(r0)
stw r6, 0x14(r1)
lbz r6, 0x2(r4)
fsubs f9, f0, f1
stw r0, 0xc(r1)
lfd f8, "@948_8042DFC8"@sda21(r0)
lfd f0, 0x8(r1)
lfd f3, 0x10(r1)
fsubs f7, f4, f8
lbz r7, 0x1(r5)
fsubs f6, f0, f8
stw r7, 0x14(r1)
fsubs f3, f3, f8
lbz r0, 0x2(r5)
lfd f2, 0x10(r1)
fmuls f7, f7, f9
stw r6, 0xc(r1)
fmuls f6, f6, f9
fsubs f5, f2, f8
lbz r4, 0x3(r4)
lfd f0, 0x8(r1)
stw r0, 0x14(r1)
fmuls f2, f3, f1
fsubs f4, f0, f8
lfd f0, 0x10(r1)
fmuls f5, f5, f1
lbz r0, 0x3(r5)
fadds f7, f7, f2
stw r4, 0xc(r1)
fsubs f3, f0, f8
stw r0, 0x14(r1)
fctiwz f7, f7
lfd f2, 0x8(r1)
fmuls f4, f4, f9
lfd f0, 0x10(r1)
fsubs f2, f2, f8
stfd f7, 0x18(r1)
fsubs f0, f0, f8
lwz r0, 0x1c(r1)
fmuls f3, f3, f1
fmuls f2, f2, f9
fmuls f0, f0, f1
stb r0, 0x0(r3)
fadds f1, f4, f3
fadds f5, f6, f5
fadds f0, f2, f0
fctiwz f1, f1
fctiwz f3, f5
fctiwz f0, f0
stfd f1, 0x28(r1)
stfd f3, 0x20(r1)
lwz r4, 0x2c(r1)
stfd f0, 0x30(r1)
lwz r5, 0x24(r1)
lwz r0, 0x34(r1)
stb r5, 0x1(r3)
stb r4, 0x2(r3)
stb r0, 0x3(r3)
addi r1, r1, 0x40
blr
```

You can regenerate this yourself, and should if you want addresses and raw
words:

```bash
bin/dtk-windows-x86_64.exe elf disasm bin/dtkspl/obj/auto_03_8016ABB0_text.o out.txt
```

That split object covers exactly `0x8016ABB0..0x8016ACD0`. `bin/dtkspl` is
**stale for ranges already banked** but authoritative for undone ones, and this
range is undone, so it is valid here.

---

## How to compile and check, without the shared build

Compile the single file and diff the disassembly. **The seven `BTE` include
paths are mandatory** across this project; keep them even if this file does not
appear to need them.

```
compilers\Wii\1.1\mwcceppc.exe -c -proc gekko -fp hard -O4 -inline noauto
  -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS -DREVOLUTION -I-
  <scratch>\m_color.cpp -o <scratch>\m_color.o
  -i include -i include\lib -i include\lib\MSL -i include\lib\MSL\internal
  -i include\lib\revolution\BTE\include -i include\lib\revolution\BTE\stack\include
  -i include\lib\revolution\BTE\stack\btm -i include\lib\revolution\BTE\bta\include
  -i include\lib\revolution\BTE\bta\sys -i include\lib\revolution\BTE\gki\common
  -i include\lib\revolution\BTE\gki\platform

bin\dtk-windows-x86_64.exe elf disasm <scratch>\m_color.o <scratch>\m_color.txt
```

`tools/auto_decomp/harness.py` already encodes those flags as
`harness.compile_draft(src, obj)`, and its `extract` / `diff_fn` are the shared
comparator — **import them rather than writing your own differ.** Build argument
lists as a PowerShell array and splat (`& $exe @args`); long inline arg lists are
fragile on PowerShell 5.1.

### Verify to this project's standard, not to "it looks right"

Six defects have been found across three of this repo's own comparison tools and
**every one returned a confident wrong answer**. So:

1. Extract the target **by address**, not by name.
2. Assert the extracted body's **instruction count x 4 == 0x114**. If it does
   not, your extraction is wrong, not the code.
3. Compare raw instruction words **and** callee symbol names — dtk zeroes
   relocations, so a wrong callee is invisible to a word comparison.
4. Compare the **`.sdata2` literal values as bytes**, separately. The `.text`
   comparator canonicalises pool references and **cannot see a wrong constant**.
   Read the two literals out of `original/wiimj2d.dol` at `0x8042DFC0` and
   `0x8042DFC8` and confirm your object's pool holds the same bytes.
5. **Run a negative control.** Deliberately corrupt one instruction and confirm
   your checker reports exactly one difference. A checker that reports success
   on a file it never really compared has happened here more than once.

---

## The slice entry — derive it, then check my derivation

Do **not** apply this. Report it. My derivation, for you to confirm or refute:

```json
{
    "source": "dol/mLib/m_color.cpp",
    "memoryRanges": {
        ".text": "0x164430-0x164550",
        ".sdata2": "0x2c60-0x2c70"
    }
}
```

inserted between the `dol/mLib/m_angle.cpp` and `dol/mLib/m_color_fader.cpp`
entries (currently lines 1269-1284 of `slices/wiimj2d.json`).

How I got `.sdata2`: `.sdata2` base is `0x8042B360`, so `@942_8042DFC0` is
offset `0x2c60` and `@948_8042DFC8` is `0x2c68`; the second is an 8-byte double
ending at `0x2c70`, which is exactly where `m_color_fader.cpp`'s existing
`.sdata2` claim begins. The adjacency is good corroboration but **I have not
checked what sits immediately below `0x2c60`** — confirm no earlier slice
already claims it and that nothing between belongs to this TU.

Also confirm, rather than assume:

- **No `.ctors` entry.** There is no `__sinit_\m_color_cpp` in the symbol map
  (`m_angle` has one at `0x8016AB60`, inside its own range). A missing or extra
  `.ctors` index is this project's documented cause of a **one-byte
  whole-binary failure**, so state explicitly what you found.
- **No `.sbss`/`.bss`/`.data`/`.rodata` claim.** If two neighbours are adjacent
  in a section, your TU claims **nothing** there — that is a legitimate answer,
  not a derivation failure. Say so explicitly per section so nobody re-opens it.
- Nothing needs adding to or removing from `syms.txt`.

---

## Deliverables — write these into `CODEX_RESPONSE.md`

1. **Status: byte-exact or not.** If not, say exactly how many words differ and
   show the diff. Do not round up to "essentially matching".
2. The final `source/dol/mLib/m_color.cpp` (you may leave it in place; it is
   inert without a slice entry) and the exact declaration you added to
   `include/game/mLib/m_color.hpp`.
3. **The evidence.** Which checks you ran, what each returned, and the result of
   your negative control. State the mangled name you confirmed.
4. **The slice entry**, confirmed or corrected, with your reasoning per section.
5. Anything that contradicted this brief. **Report contradictions, do not
   reconcile them yourself** — this instruction has repeatedly caught errors here
   that no per-function diff could see.
6. If you finish early and want more: the next candidates from your own survey
   are `MsgRes_c` ctor + 3 siblings (`0x800CE7F0`, 220 B) and the
   `sPrintf`/`OSReport` group (`0x8015F810`, 216 B). Same rules — author and
   verify standalone, propose the slice entry, do not build, do not land.

Claude will do the integration: slice entry, full `ninja`, and
`progress.py --verify-bin` across all five binaries. Nothing is considered
landed until those five MD5s match.
