# Batch 3 report — `fn_800CF170` (cmdSave), `save`, `createBanner`, `writeBanner`

Work done in `wip/nand_thread/scratch/batch3/`. Compiled/diffed with
`tools/auto_decomp/harness.py`'s `compile_draft`/`extract`/`diff_fn`, never a
hand-rolled compiler line or differ. Extraction was by ADDRESS via the target's
own function-size comments, cross-checked against
`bin/dtk/wiimj2d_symbols.txt` for every function and data object below.

## Result summary

| Function | Address | Size (map) | Status |
|---|---|---|---|
| `fn_800CF170` → **`cmdSave`** | `0x800CF170` | `0x8C` (35 instr) | **MATCH** — 35/35 instructions, byte-identical |
| `createBanner` | `0x800CF380` | `0x178` (94 instr) | **MATCH** — 94/94 instructions, byte-identical |
| `save` | `0x800CF200` | `0x17C` (95 instr) | **NOT CLOSED** — 86/95 instructions emitted, structurally right, register allocation and one codegen idiom differ |
| `writeBanner` | `0x800CF500` | `0x108` (66 instr) | **NOT CLOSED** — 63/66 instructions emitted, all calls/operands correct, missing 3 instructions from register pressure + one bitfield-store idiom differs |

## `fn_800CF170` = `cmdSave(const void*)` — confirmed, not just hypothesized

Batch 2's `CMD_SHAPE.md` hypothesized this from a neighbouring function; I
confirm it independently and it is now proven, not inferred:

- `mState = 4` is stored on the locked path — `4` is `save`'s command id per
  `run()`'s own switch (`0x800CFB08 cmpwi r0,0x4 / beq -> bl save__...`),
  which I read directly out of `target_raw.txt` myself before seeing
  `CMD_SHAPE.md`.
- The extra `memcpy(l_tmpSave, arg, 0x3fa0)` Batch 2 predicted is exactly
  what's there, in exactly that position (after `mError`/`mState`, before
  unlock).
- Applying Batch 2's proven lever — store `OSTryLockMutex`'s result into a
  real `bool` local, branch on that local, success-block-first — closed it
  completely on the first try after the lever was applied. Two independent
  derivations (Batch 2's from a neighbour, mine from direct compilation)
  agree, and mine is now a byte-exact proof, not a pattern match.

Final source, `@unofficial` (no name in the symbol map):

```cpp
/// @unofficial Name not recovered (unnamed in the symbol map, 0x800CF170).
/// Confirmed (not just inferred) to be the cmdSave counterpart to
/// cmdExistCheck/cmdSpaceCheck/cmdLoad/cmdDeleteFile: stores 4 into mState
/// (matching run()'s command-id switch, where 4 dispatches to save()) and
/// copies the caller's save data into l_tmpSave before signalling.
bool dNandThread_c::cmdSave(const void *saveData) {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 4;
        memcpy(l_tmpSave, saveData, sizeof(l_tmpSave));
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}
```

## `createBanner` — MATCH, final source

```cpp
bool dNandThread_c::createBanner() {
    setNandError(NANDCreate(sc_TEMP_BANNER_FILE, 0x3c, 0));
    if (mError != 0)
        return true;

    NANDFileInfo info;
    setNandError(NANDOpen(sc_TEMP_BANNER_FILE, &info, 2));
    if (mError != 0)
        return true;

    u32 written = writeBanner(&info);
    if (written != 0x72a0) {
        setNandError(written);
        return true;
    }

    setNandError(NANDClose(&info));
    if (mError != 0)
        return true;

    char homeDir[0x40] = {0};
    setNandError(NANDGetHomeDir(homeDir));
    if (mError != 0)
        return true;

    setNandError(NANDMove(sc_TEMP_BANNER_FILE, homeDir));
    return mError != 0;
}
```

Correction to my own first draft, found only by compiling: I assumed the
final `NANDMove` moved the temp banner file onto `sc_BANNER_FILE`
("banner.bin"). **`sc_BANNER_FILE` is never referenced anywhere in
`createBanner`.** The real second argument is the buffer `NANDGetHomeDir`
just filled (zero-initialised first, 16 individual `stw`s — matches an
aggregate `= {0}` init on a `char[0x40]`, not a `memset` call). `sc_BANNER_FILE`
is written by Batch 2's `existCheck` only; it does not appear anywhere in my
four functions.

## `save` — not closed

95 instructions in target, 86 emitted. The **control flow, every callee, every
argument, and every branch condition are correct** (confirmed by reading the
diff line-by-line — see full diff in
`wip/nand_thread/scratch/batch3/full_diff_save_writebanner.txt`). Two
consistent kinds of divergence, both present from the first `mError` check
onward, so every subsequent instruction offset walks:

1. **Register allocation is swapped.** Target keeps `this` in `r31` and
   `sc_GAME_FILE`'s address in `r30` (`mr r31,r3` immediately after prologue,
   `lis r30,sc_GAME_FILE@ha` after). My draft allocates the opposite:
   `sc_GAME_FILE` in `r31`, `this` in `r30`. Both values genuinely span two
   calls (correctly identified — my draft *does* preserve both, in nonvolatile
   registers, across the right calls), just numbered the other way round.
2. **`if (mError == 0)` compiles to `cmpwi`+branch in my draft, `cntlzw`+
   `srwi.`+branch in target** (a 0/1-materialising idiom), every place this
   check occurs.

What I tried and ruled out for (2): the lever that closed `cmdSave` and (per
`CMD_SHAPE.md`) `cmdExistCheck`/`cmdSpaceCheck` — storing the tested value into
an explicit local first (`bool ok = !mError; if (ok)`, and separately
`s32 result = NANDCreate(...); setNandError(result);` to change evaluation
order) — produced byte-identical draft output before and after in both cases.
Unlike `OSTryLockMutex`'s `BOOL` (a real narrowing from an arbitrary-int to a
true `bool`), `mError` is already a plain `int`, and `!mError`/`mError == 0`
is the same comparison either way to the optimizer — it saw through both
rewrites. I do not know what source shape produces the `cntlzw` form here; I
did not find it by the two most obvious levers, and I am reporting that
negative result rather than guessing further.

Final (unmatched but structurally-correct) source:

```cpp
s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    if (mError == 0) {
        NANDFileInfo info;
        setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
        if (mError == 0) {
            s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
            if (written < 0) {
                setNandError(written);
                if (mError != 0) {
                    if (mError != 6) {
                        setNandError(NANDSimpleSafeCancel(&info));
                        if (mError == 0)
                            return 2;
                    }
                    return 1;
                }
            }
            setNandError(NANDSimpleSafeClose(&info));
            if (mError == 0)
                return createBanner();
            return 1;
        }
        if (mError != 6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }
    return 1;
}
```

**Finding, proven by direct compilation, not just read off the bytes: `save()`
does not return `bool`.** `run()` does
`bl save__13dNandThread_cFv / cmpwi r3,0x2 / beq <retry save>` — a normalized
bool can never be 2. `2` means "cancel succeeded, ask the caller to retry the
whole save"; `1` means "done" (with or without error). Declaring it `bool` in
my shadow header and returning `2` literally either gets rejected by the
compiler's implicit-conversion warning path or (worse) silently narrows to
`1`/`true`, which cannot reproduce the target bytes. I only got 86/95 matching
*after* fixing this to `s32` — with `bool` the tail of the function doesn't
even have a chance to match. PROPOSED for `include/game/bases/d_nand_thread.hpp`:
change `bool save();` to `s32 save();`.

## `writeBanner` — not closed

66 instructions in target, 63 emitted. Every call, every argument, and the
five data-object references are correct and in the right order (see below).
Two gaps, both isolated to the same two things:

1. **Missing `_savegpr_27`/`_restgpr_27`.** Target uses this compiler helper
   (visible in the constructor too — `wip/nand_thread/target_raw.txt` line
   18/72) because it needs **5** simultaneously-live nonvolatile registers
   (`r27`..`r31`): `r27` (message-group constant, then reused as `a_banner`'s
   base pointer), `r31` (the `fileInfo` parameter), `r28` (first `getMsg`
   result, then reused as `"save_banner"`'s address), plus two more (`r29`,
   `r30`) that only exist because of point 2. My draft only ever needs 4
   (`r28`..`r31`) and gets a plain `stw`/`lwz` prologue/epilogue instead.
2. **The `iconSpeed` bitfield store.** Target computes it at runtime:
   `slw r3,r28,r30 / slw r0,r29,r30 / andc / or / clrlwi / rlwinm / sth` — a
   general "mask out 2 bits at a variable shift, OR in a new 2-bit value"
   sequence, using registers loaded with the literal operands `3`, `2`, `0`.
   Every source shape I tried for `a_banner.iconSpeed = (a_banner.iconSpeed &
   ~(3 << (frame*2))) | (2 << (frame*2))` — inline with `frame` a compile-time
   `0`, a real `for (frame = 0; frame < 1; ++frame)` loop, and an `inline`
   free function taking `frame` as a parameter called with a literal `0` — all
   get fully constant-folded by MWCC at `-O4` down to
   `clrrwi r0,r0,2 / ori r0,r0,0x2` (3 instructions instead of target's 7).
   I could not find a source shape that keeps this a runtime shift. This is
   very likely why target needs the two extra registers (`r29`=2, `r30`=0)
   that account for the rest of the register-pressure gap in point 1. I do
   not know the real shift expression (I suspect a per-frame bitfield-setter
   shared by other Nintendo titles using `NANDBanner`, not something visible
   in this TU alone) and am reporting this as unresolved rather than guessing
   a value that happens to fold to the same constant.

Everything else about `writeBanner` — call order, `NANDBanner` field offsets
(`bannerTexture`@0xA0/0x6000 bytes, `iconTexture`@0x60A0/0x1200 bytes, both
already declared correctly in `include/lib/revolution/NAND/nand.h`), the
`0x1c`-byte-offset resource-payload read (see below), and the final
`return NANDWrite(fileInfo, &a_banner, 0x72a0);` — matched on the first
structurally-correct draft.

Final (unmatched but structurally-correct) source:

```cpp
s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    /* @unofficial: does not reproduce target's runtime shift -- see report */
    a_banner.iconSpeed = (a_banner.iconSpeed & ~3) | 2;

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
```

**Open question, not resolved:** `dResMng_c::m_instance->getRes(arcName,
resPath)` returns `nw4r::g3d::ResFile` (confirmed by the mangled name
`getRes__6dRes_cCFPCcPCc` matching the real, already-declared 2-arg overload
in `include/game/bases/d_res.hpp` exactly). The code then reads a `u32` at
byte offset `0x1c` from the raw pointer and adds it to the same pointer to
get the actual payload address. I worked out that absolute offset `0x1c`
inside `ResFileData` (`fileHeader` 0x10 + `ResTopLevelDictData.header` 0x8 +
0x4) lands on `ResDicData::numData` — a dictionary node count, which makes no
sense as a byte offset to add to a pointer. That means either (a) this
resource isn't really shaped like a G3D `ResFile` despite the exact-matching
mangled call, and `0x1c` is some other, simpler format's fixed header size, or
(b) my offset arithmetic through `ResFileData`/`ResTopLevelDictData`/
`ResDicData` is wrong somewhere. I did not invent a field name for this — I
left it as a raw pointer + comment rather than assert a `nw4r::g3d` field name
I'm not sure of. This did not block byte-matching the instructions that use it
(the `lwz ...,0x1c(r3)` / `add r4,r4,r0` pair matches target exactly, twice),
so it's a naming/documentation gap, not a correctness gap in the two `getRes`
call sites I proved.

## Data objects — measured, not asserted

Compiled my draft and inspected the actual object with
`bin/dtk-windows-x86_64.exe elf disasm`, not just read off the source:

| Object | Section (measured) | Size (measured) | Order (measured) | Matches brief? |
|---|---|---|---|---|
| `"save_icon.bti"` | `.data` | `0xE` | 1st `.data` object (offset `0x0`) | Yes |
| `"save_banner_EU.bti"` | `.data` | `0x13` | 2nd, immediately after (offset `0x10`, 8-aligned) | Yes |
| `"save_banner"` | `.data` | `0xC` | 3rd, immediately after (offset `0x24`) | Yes |
| `a_banner` | `.bss` | `0xF0A0` | Immediately after `l_tmpSave` in my draft's `.bss` (`l_safeCopyBuf` @0, `l_tmpSave` @0x4000, `a_banner` @0x7FA0) — the same contiguous layout the brief's real addresses show (`0x80359FC0`, `0x8035DFC0`, `0x80361F60`: `+0x4000`, `+0x3FA0`) | Yes |
| `c_icon_res` | `.sdata` | `0x4` | Only/first object in my draft's `.sdata`; initialiser is the address of `"save_icon.bti"` (`.4byte "@10535"` in the compiled dump, where `@10535` is that draft's pool id for `"save_icon.bti"`) | Yes |

**Contradiction found, not reconciled: `@67269` ("SMNP") is not Batch 3's.**
The brief's data inventory table assigns it to Batch 3. I searched all of
`target_raw.txt` for `67269`/`SMNP` and it appears exactly twice, both inside
`load()`'s address range (`0x800CF844` and `0x800CF860`, `load` spans
`0x800CF680`–`0x800CF904`) — a Batch 4 function. It does not appear anywhere
in `fn_800CF170`, `save`, `createBanner`, or `writeBanner`. Pooled string
literals are emitted where they're lexically written in the merged source, so
whoever writes `load()`'s body is who emits `"SMNP"`, not me. **I have not
defined `"SMNP"` anywhere in my draft.** Batch 4 should own it; the brief needs
correcting, and if Batch 4 also believes it's not theirs, that string is at
risk of going unemitted or duplicated.

## Header changes proposed (not applied to the real header — shadow-copied only)

Shadow copy at `wip/nand_thread/scratch/batch3/shadow_include/game/bases/d_nand_thread.hpp`,
based on the current landed header plus:

1. `bool cmdSave(const void *saveData);` — new declaration, proven by the
   byte-exact match above.
2. `bool save();` → **`s32 save();`** — proven necessary; see the `save()`
   section above (`run()` checks `save() == 2`, which a normalized `bool`
   cannot represent).
3. `bool writeBanner(NANDFileInfo *fileInfo);` → **`s32 writeBanner(NANDFileInfo *fileInfo);`**
   — proven necessary; `createBanner()` compares the raw return value against
   `0x72a0` (an exact byte count, not 0/1), and `writeBanner`'s own tail
   (`return NANDWrite(...);`) never narrows to bool in target's bytes either.

`createBanner()` itself is correctly `bool` already (its tail is the
canonical `neg/or/srwi` 0/1-normalisation idiom, and it byte-matches as `bool`).

Also shadow-copied `wip/nand_thread/scratch/batch3/shadow_include/revolution/NAND/NANDOpenClose.h`:
the real `include/lib/revolution/NAND/NANDOpenClose.h` is missing
`NANDSimpleSafeOpen`/`NANDSimpleSafeClose`/`NANDSimpleSafeCancel` entirely
(used by `save()`, and by `load()` in Batch 4). Signatures proven from the
call sites: `s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 access, void* buffer, u32 bufferSize);`,
`s32 NANDSimpleSafeClose(NANDFileInfo* info);`, `s32 NANDSimpleSafeCancel(NANDFileInfo* info);`.
PROPOSED for the real header; every batch that touches `save`/`load` needs
this.

## Files

- `wip/nand_thread/scratch/batch3/d_nand_thread.cpp` — current draft (all four functions)
- `wip/nand_thread/scratch/batch3/shadow_include/` — shadow headers (see above), not applied to the real tree
- `wip/nand_thread/scratch/batch3/run_diff.py` — the compile/diff driver I used, wraps `harness.compile_draft`/`extract`/`diff_fn`
- `wip/nand_thread/scratch/batch3/full_diff_save_writebanner.txt` — full (untruncated) instruction-by-instruction diff for `save`/`writeBanner`
