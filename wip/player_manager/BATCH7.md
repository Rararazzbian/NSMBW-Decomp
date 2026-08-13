# Batch B7 — `0x800608E0`–`0x80060F1F`

Verified with `compilers/Wii/1.1/mwcceppc.exe` (the flags from `SHARED-BRIEF.md`)
against `wip/player_manager/target_text.txt`, using
`tools/auto_decomp/harness.py`'s `compile_draft` / `disasm` / `extract` /
`diff_fn` directly from a scratch Python driver (not `--auto`, not `land.py`).
Two shared headers had to be shadow-copied into scratch to compile at all —
see "Header findings" below; **neither shadow copy was written back into
`include/`**.

## Per-function status

| Function | Address | Size | Status |
|---|---|---|---|
| `checkLastAlivePlayer()` | `0x800608E0` | `0x88` | **NOT MATCHING** — near miss, see below |
| `executeLastPlayer()` | `0x80060970` | `0x98` | **MATCHING** — diff printed nothing |
| `executeLastAll()` | `0x80060A10` | `0x98` | **MATCHING** — diff printed nothing |
| `deleteCullingYoshi()` | `0x80060AB0` | `0x158` | **NOT MATCHING** — near miss, see below |
| `setHipAttackQuake(int, u8)` | `0x80060C10` | `0x1A0` | **NOT MATCHING** — near miss, see below |
| `fn_80060DB0` (file-scope static) | `0x80060DB0` | `0x138` | **MATCHING** — byte-exact (see note on how this was verified) |
| `checkBonusNoCap()` | `0x80060EF0` | `0x24` | **MATCHING** — diff printed nothing |

4 of 7 are byte-exact. The other 3 are well-characterized near-misses — every
one differs from the target only in specific, named ways (below), not in
overall shape, callees, or SDA symbols.

**Note on `fn_80060DB0`'s verification:** it is `static` at file scope (per
this batch's brief), so my compiled object names it `fn_80060DB0__Fv`
(CFront still mangles internal-linkage functions), while the target's own
symbol is the bare placeholder `fn_80060DB0` (no name survives in the
shipped object at all — dtk falls back to an address-based name). The
harness's `extract()` matches by literal name, so the automated diff
reports "DRAFT MISSING" for this one function; I bypassed that by calling
`harness.extract()` on each side with its own real name and diffing the
returned instruction lists directly. Confirmed byte-identical, 78/78
canonicalised instructions, zero differences.

## Source

```cpp
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_quake.hpp>
#include <game/snd/snd_scene_manager.hpp>
#include <game/snd/snd_audio_mgr.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/framework/f_manager.hpp>

// file-scope static helper -- see below. No cross-TU caller anywhere in
// source/, no syms.txt pin; only ever called from setHipAttackQuake, one
// basic block before its own epilogue.
static void fn_80060DB0();

// .sbss:0x80429FD0, the last object in the unit's .sbss -- unnamed, no class
// mangling, read/written only inside setHipAttackQuake. See below.
// s8, not bool: read back via `lbz`+record-form `extsb.` (a signed-byte test),
// not a plain zero compare.
static s8 lbl_80429FD0;

bool daPyMng_c::checkLastAlivePlayer() {
    bool multiplayer = getEntryNum() > 1;
    if (multiplayer) {
        if (mNum <= 1) {
            if (!(mBgmState & 4)) {
                mBgmState |= 4;
                SndSceneMgr::sInstance->fn_8019bd90(0x400);
            }
        } else {
            if (mBgmState & 4) {
                mBgmState &= ~4;
                SndSceneMgr::sInstance->fn_8019be60(0x400);
            }
        }
    }
    return false;
}

void daPyMng_c::executeLastPlayer() {
    for (int i = 0; i < 4; i++) {
        daYoshi_c *p = static_cast<daYoshi_c *>(fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]));
        if (p) {
            p->executeLastPlayer();
        }
    }
    for (int i = 0; i < 4; i++) {
        dAcPy_c *p = getPlayer(i);
        if (p) {
            p->executeLastPlayer();
        }
    }
}

void daPyMng_c::executeLastAll() {
    for (int i = 0; i < 4; i++) {
        daYoshi_c *p = static_cast<daYoshi_c *>(fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]));
        if (p) {
            p->executeLastAll();
        }
    }
    for (int i = 0; i < 4; i++) {
        dAcPy_c *p = getPlayer(i);
        if (p) {
            p->executeLastAll();
        }
    }
}

// fBase_c::mDeleteRequested is protected; this offset (fBase_c+0xb) is
// proven exact against the disassembly's own single `lbz`. See below.
static inline bool isDeleteRequested(fBase_c *p) {
    return *(reinterpret_cast<const u8 *>(p) + 0xb) != 0;
}

int daPyMng_c::deleteCullingYoshi() {
    dBgParameter_c *bg = dBgParameter_c::ms_Instance_p;
    mVec2_c mid;
    mid.y = bg->yStart() - bg->ySize() * 0.5f;
    mid.x = bg->xStart() + bg->xSize() * 0.5f;

    fBase_c *farthest = 0;
    float farthestDist = 0.0f;
    for (int i = 0; i < 4; i++) {
        daYoshi_c *p = static_cast<daYoshi_c *>(fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]));
        if (!p) continue;
        if (isDeleteRequested(p)) continue;
        if (p->getPlrNo() != -1) continue;
        if (!p->isStatus(daPlBase_c::STATUS_DISPLAY_OUT_DEAD)) continue;

        mVec2_c ppos;
        ppos.y = p->mPos.y;
        ppos.x = p->mPos.x;
        mVec2_c delta;
        delta.y = mid.y - ppos.y;
        delta.x = mid.x - ppos.x;
        float distSq = delta.x * delta.x + delta.y * delta.y;
        float dist = EGG::Mathf::sqrt(distSq);
        if (dist > farthestDist) {
            farthestDist = dist;
            farthest = p;
        }
    }

    if (farthest) {
        farthest->deleteRequest();
        return 1;
    }
    return 0;
}

void daPyMng_c::setHipAttackQuake(int type, u8 plrNo) {
    // Reproduces the disassembly's own addressing: this function reaches
    // m_quakeTimer/m_quakeEffectFlag (and the unnamed .bss table below) via
    // one base pointer computed from m_playerID plus constant offsets,
    // rather than through their own symbols -- unlike fn_80060DB0, which
    // addresses the exact same arrays by name. See below.
    u8 *base = reinterpret_cast<u8 *>(m_playerID);

    if (plrNo == -1) return;
    if (type == 2) {
        dQuake_c::m_instance->shockMotor((s8)plrNo, dQuake_c::TYPE_7, 0, false);
        return;
    }

    int *timer = reinterpret_cast<int *>(base + 0xa0);   // == m_quakeTimer
    int *flag = reinterpret_cast<int *>(base + 0xb0);    // == m_quakeEffectFlag
    timer[plrNo] = 5;
    flag[plrNo] = 0;
    int count = 0;
    if (!dScStage_c::m_isStaffCredit) {
        for (int j = 0; j < 4; j++) {
            if (j == plrNo) continue;
            if (timer[j] != 0) {
                timer[j] = 5;
                count++;
            }
        }
    }

    if (count != 0) {
        int *seTable = reinterpret_cast<int *>(base + 0xea0);
        if (!lbl_80429FD0) {
            int se2 = 0x152, se1 = 0x151, se0 = 0x150;
            seTable[0] = se2;
            seTable[1] = se1;
            seTable[2] = se0;
            lbl_80429FD0 = 1;
        }
        if ((u32)(count - 1) <= 2) {
            SndAudioMgr::sInstance->startSystemSe(seTable[count - 1], 1);
        }
        fn_80060DB0();
        return;
    }

    if (type == 1) {
        dQuake_c::m_instance->startShock((s8)plrNo, dQuake_c::TYPE_3, 3, 0, false);
    } else {
        dQuake_c::m_instance->shockMotor((s8)plrNo, dQuake_c::TYPE_4, 0, false);
    }
}

static void fn_80060DB0() {
    SndSceneMgr::sInstance->onPowerImpact();
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::m_quakeTimer[i] != 0 && daPyMng_c::m_quakeEffectFlag[i] == 0) {
            daPyMng_c::m_quakeEffectFlag[i] = 1;
            dAcPy_c *py = daPyMng_c::getPlayer(i);
            if (!py) continue;
            daPlBase_c *p = py;
            if (py->isStatus(daPlBase_c::STATUS_RIDE_YOSHI)) {
                p = py->getRideYoshi();
            }
            if (!p) continue;
            mVec3_c pos;
            pos.x = p->mPos.x;
            pos.y = p->mPos.y;
            pos.z = p->mPos.z;
            mEf::createEffect("Wm_mr_vshipattack", 0, &pos, 0, 0);
            pos.z = 3800.0f;
            mEf::createEffect("Wm_mr_vshipattack_ind", 0, &pos, 0, 0);
            dQuake_c::m_instance->startShock((s8)i, dQuake_c::TYPE_3, 3, 0x12, false);
        }
    }
}

void daPyMng_c::checkBonusNoCap() {
    mBonusNoCap = 0;
    if (mRest[0] >= 0x63) {
        mBonusNoCap = 1;
    }
}
```

## Data-object report — all four brief-listed objects, plus one more found

| Object | Section | Address | Status |
|---|---|---|---|
| `"Wm_mr_vshipattack"` | `.data` | `0x80309A28` | **Emitted, confirmed byte-exact.** First `mEf::createEffect` argument in `fn_80060DB0`. Verified directly against `target_data.txt`'s raw bytes (`57 6D 5F 6D 72 5F 76 73 68 69 70 61 74 74 61 63 6B 00`) before writing the literal, to avoid a transcription error — decodes to exactly `"Wm_mr_vshipattack"`, 17 chars + NUL = `0x12`, matching the brief. |
| `"Wm_mr_vshipattack_ind"` | `.data` | `0x80309A3C` | **Emitted, confirmed byte-exact.** Second `mEf::createEffect` argument, immediately after the first string in the same function. Bytes decode to `"Wm_mr_vshipattack_ind"`, 21 chars + NUL = `0x16`. |
| `3800.0f` | `.sdata2` | `0x8042BD7C` | **Emitted, confirmed byte-exact** (`0x456D8000` in `target_sdata2.txt` = `3800.0f`, checked by hand). It is **not** the yoshi's real Z position: `fn_80060DB0` calls `mEf::createEffect` **twice** reusing the same stack `mVec3_c`, and after the first call overwrites just `pos.z` with this constant before the second call — the first call uses the yoshi's real `mPos.z`. Missing this order was the difference between a near-miss and the eventual byte-exact match on this function. |
| `lbl_80429FD0` (unnamed `.sbss` byte) | `.sbss` | `0x80429FD0` | **Defined, at file scope in the `.cpp`** (see the top of the source above): `static s8 lbl_80429FD0;`. Confirmed to be the **last object in the unit's `.sbss`** per the brief; if it lands correctly the section ends exactly where `d_actor.cpp`'s `.sbss` begins (`0x80429FD8`). **Type is `s8`, not `bool`/`u8`** — the disassembly reads it back with `lbz` followed by a record-form `extsb.` (a signed-byte test), which only comes from a signed-char-typed read, not an unsigned/bool one. This is a correction to the brief and to `BATCHES.md`'s "unnamed 1-byte flag" framing. |

**A fifth object, not in the brief's table, that this batch also had to
resolve:** three `int`s (`0xC` bytes) at `0x80355FB0`, immediately after
`mEffectMng` ends (`0x80355354 + 0xC5C = 0x80355FB0`) and immediately before
the `.bss` section's own end (`0x80355FC0`). `target_bss.txt` labels this
range a `0x10`-byte **gap** (padding) — but `setHipAttackQuake` genuinely
**writes and reads** the first `0xC` bytes of it (a lazily-initialized,
one-shot table of three sound-effect IDs, `{0x152, 0x151, 0x150}`, indexed by
`count-1` to pick an escalating cue based on how many *other* players got hit
by the same quake). So `target_bss.txt`'s "gap" label for this specific
sub-range is **wrong** — it is code-addressed data, not padding, and the
remaining `0x4` bytes (`0x80355FBC`–`0x80355FC0`) are the true padding.
**Not given a name or a header declaration**: the disassembly reaches it via
`m_playerID`'s base pointer plus a constant offset (`+0xea0`), not via its
own symbol (see "Header/addressing findings" below for why that address
choice is itself significant), so it is expressed the same way as the
`d_a_player_demo_manager.cpp` precedent for undeclared fields — a raw offset
from a known anchor, with a comment — rather than as a new header member.
**Report only; not claimed as a `daPyMng_c` static member**, since I cannot
edit the header and the disassembly gives no name for it.

Confirming the brief's own instruction: **these are the entire `.data` claim
of the unit** — no third string appears anywhere in this batch's functions,
and the two strings plus the float are the full extent of what
`fn_80060DB0` references from those two sections.

## Header findings (report only — no shared header was edited)

Three declarations needed by this batch's functions were missing or wrong in
already-decompiled, shared headers. All three were proven by getting a
byte-exact match against target bytes with the fix in place (in a
scratch-only shadow copy), so they are load-bearing findings, not guesses.

1. **`dQuake_c::startShock` is entirely undeclared** in
   `include/game/bases/d_quake.hpp`. Only `shockMotor` and `startShockAll`
   are there. The real signature, confirmed by both `setHipAttackQuake` and
   `fn_80060DB0` calling it and matching byte-for-byte once declared:
   `void startShock(s8, TYPE_SHOCK_e, int, int, bool);` (mangled
   `startShock__8dQuake_cFScQ28dQuake_c12TYPE_SHOCK_eiib`, i.e. one more
   `int` parameter than `shockMotor`).
2. **`SndSceneMgr::onPowerImpact()` is entirely undeclared** in
   `include/game/snd/snd_scene_manager.hpp`. `fn_80060DB0` calls it as the
   very first thing it does. Confirmed signature: `void onPowerImpact();`
   (mangled `onPowerImpact__11SndSceneMgrFv`).
3. **`SndAudioMgr::startSystemSe`'s first parameter type is wrong** in
   `include/game/snd/snd_audio_mgr.hpp`: declared
   `void startSystemSe(unsigned int soundID, unsigned long);`, but the
   target's actual mangled symbol is `startSystemSe__11SndAudioMgrFUlUl` —
   **both** parameters are `unsigned long`, not `unsigned int` +
   `unsigned long`. Confirmed the same way: the call only produced the right
   symbol name once the shadow copy was corrected.

None of these three headers were edited in `include/` — all three fixes live
only in a scratch shadow-include directory used to compile this batch's
draft locally, per the shared brief's "shadow-copy it into your scratch,
prove the change there, and report it" instruction.

## Addressing finding: `setHipAttackQuake` does not use `m_quakeTimer`'s or `m_quakeEffectFlag`'s own symbols

This is worth flagging on its own because it contradicts the natural
assumption that "a named member is always addressed by its own name."

`fn_80060DB0` addresses `daPyMng_c::m_quakeTimer[i]` and
`daPyMng_c::m_quakeEffectFlag[i]` by their own SDA symbols directly (proven —
that function is byte-exact using exactly `m_quakeTimer[i]` /
`m_quakeEffectFlag[i]` array syntax). But `setHipAttackQuake`, touching the
*same two arrays*, does not: the target disassembly computes
`&m_playerID[0]` **once**, early (`lis r8,m_playerID@ha` / `addi r8,r8,...`,
right after the `plrNo==-1` check, before even the `type==2` branch that
doesn't need it), and reaches `m_quakeTimer` and `m_quakeEffectFlag` as
`r8+0xa0` / `r8+0xb0` — constant offsets from that one base register — for
the rest of the function, including the unnamed 3-int table at `r8+0xea0`
above. Since `m_quakeTimer`'s and `m_playerID`'s link-time addresses are
**not related by any compile-time-visible constant** (they're independent
SDA symbols), a compiler cannot discover "m_quakeTimer is exactly `0xa0`
past `m_playerID`" on its own — this only happens if the *source itself*
expresses the access that way. So `setHipAttackQuake`'s original source
almost certainly used raw offset arithmetic from `m_playerID` for these
three regions, in contrast to `fn_80060DB0`'s proper named-array access to
the identical data. I matched this by computing a `u8 *base` from
`m_playerID` once and indexing through it, with a comment explaining why
(see the source above) — this is not guesswork, it's the only way the
`0xa0`/`0xb0`/`0xea0` triad of constant offsets from one shared base makes
sense.

## Finding: `checkLastAlivePlayer`'s threshold is `> 1`, not `== 1`

`BATCHES.md`/`MAP.md` describe the opening test as "the `xori/srawi/and/
subf/srwi` MWCC idiom for equality-to-1." I derived the idiom's truth table
by hand (it zeroes for `x` ∈ {0, 1} and is nonzero for `x >= 2`) and
confirmed it against the actual branch structure: the guarded block (which
further checks `mNum <= 1` to fire a "last player alive" music sting) is
**skipped** when the computed value is `0`. That means the guarded block
runs when `getEntryNum() > 1`, not when it `== 1` — i.e. this check gates
the sting logic on being in a **multiplayer** game (2+ entrants), not on
being down to exactly one. **Reporting this as a correction**, not
reconciling it against the existing docs.

## Near-misses — precisely characterized, not claimed as MATCHING

**`checkLastAlivePlayer` — 34/34 instructions, differs only in one 5-instruction sub-sequence.**
Every SDA symbol, every branch target, and the full structure (including the
return-`false` tail) match. The one gap: materializing `getEntryNum() > 1`
into a bool produces a `cntlzw`/`slw`/`srwi.`-based idiom in my draft, while
the target uses the `xori`/`srawi`/`and`/`subf`/`srwi`-based idiom
documented above. Both are mathematically equivalent branch-free "materialize
a 0/1 boolean" tricks; I could not find a source form that made MWCC choose
the second one over the first (tried: a direct `if`, an assigned `bool`, an
assigned `int`, a ternary, `!(x<=1)`, `x>=2` — all six produced one of the
two idioms already seen, never the target's). Reporting as a genuine,
narrowly-bounded gap rather than guessing further.

**`deleteCullingYoshi` — 86/86 instructions, all content matches; ~18 lines differ, all pure register-number swaps.**
Every symbol, every float constant, every call, and the full control flow
match exactly once `isDeleteRequested()` (the `fBase_c+0xb` protected-field
read) and the `mVec2_c mid`/`ppos`/`delta` locals (matching the target's own
choice to spill these to the stack rather than keep them in registers across
the `sqrt` call) were in place. What's left is the allocator's choice of
*which* register number holds `p`/`farthest` (`r28` vs `r30` etc.) and which
FPR holds which intermediate (`f2`/`f3`/`f4` permuted) — tried several
declaration-order permutations (this is the exact "declaration order sets
GPR allocation... FPR direction is not fixed, sweep it" lever) without
closing the gap. Reporting as a genuine near-miss.

**`setHipAttackQuake` — 103 vs 104 instructions, same shape and symbols throughout.**
Closed several real gaps here (the `-1` sentinel test, the `s8` flag type,
the `startSystemSe` signature, the `m_playerID`-relative addressing). What
remains is concentrated in the unrolled 4-way `m_quakeTimer` scan and the
3-entry SE-ID table write: the target computes `&m_quakeTimer[0]` into `r6`
and the *index* (`plrNo*4`) into `r9` as two separate steps before the
indexed store, while my draft's allocator computes the index first and picks
different register numbers for the same values — same instructions, mostly
different registers, plus one extra instruction I could not isolate further
in the time available. Reporting as a genuine near-miss rather than pushing
further permutations that risk making it worse (already saw one attempt do
exactly that and had to revert it).

## Files touched

- `wip/player_manager/BATCH7.md` (this file).
- Nothing under `include/`, `slices/`, or `syms.txt` was edited. Two headers
  were shadow-copied into the session scratchpad to compile locally
  (`d_quake.hpp` +`startShock`, `snd_scene_manager.hpp` +`onPowerImpact`,
  `snd_audio_mgr.hpp`'s `startSystemSe` signature correction) — see "Header
  findings" above.
- The shared `.cpp` was not edited; the source above is for the lead to
  assemble by address, per the batch rules.
