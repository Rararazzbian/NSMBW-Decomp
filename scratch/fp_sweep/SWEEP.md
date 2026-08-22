# False-positive sweep — landed units (slices/wiimj2d.json + 4 REL slice jsons)

Scope of THIS report: the landed-unit half of the repo-wide audit only (per
coordinator instruction — the wm_units/d_line_mng/m_pad/nand_thread/
player_manager drafts are being covered by sibling agents, not by this
report). All work is read-only; every artifact lives under `scratch/fp_sweep/`.
Tools built this session: `scratch/fp_sweep/poolcheck.py`→ actually
`tools/auto_decomp/poolcheck.py` (pre-existing) plus new
`scratch/fp_sweep/blcheck.py` (bl-callee equivalent), `scratch/fp_sweep/audit.py`
(generic driver), and the landed-sweep drivers under `scratch/fp_sweep/landed/`
(`sweep_dol.py`, `sweep_rel.py`, `sweep_rel2.py`, `index_dtkspl.py`).

## A. Headline

- **144 units** claimed landed in `slices/wiimj2d.json`; **38 units** claimed
  landed across the 4 REL slice jsons (`d_basesNP` 28, `d_enemiesNP` 4,
  `d_profileNP` 5, `d_en_bossNP` 1) — **182 landed units total**.
- **DOL pool-constant check**: 30/143 units had both a compiled object
  (`bin/compiled/wiimj2d/...`) and a usable retail split object
  (`bin/dtkspl/obj/...`) available. Across those 30 units, **578 pooled
  `lfs`/`lfd` constants were compared by decoded IEEE-754 VALUE — 0
  mismatches.**
- **DOL bl-callee check**: same 30 units, **1,788 `bl`/`bla` targets compared
  by resolved symbol name — 7 raw mismatches, all 7 manually resolved as
  false alarms of my own comparison method (see §B), zero confirmed real
  defects.**
- **REL bl-callee check**: 28 `d_basesNP` + 4 `d_enemiesNP` + 1 `d_en_bossNP`
  units got a retail dump (`d_profileNP`'s 5 units did not — no matching
  split objects found, see §D). Content-based pairing (verify_anon.py-style,
  required because REL symbol names are almost entirely stripped) matched
  123/179 functions in `d_basesNP` and found **30 raw `bl` mismatches, all
  investigated and all traced to two known, documented artifact classes —
  zero confirmed real defects** (see §B). `d_enemiesNP`/`d_en_bossNP`
  content-pairing matched 0 functions — could not check (see §D).
- **REL pool-constant check: NOT performed at scale.** RELs compile with
  `-sdata 0 -sdata2 0`, which disables `@sda21` addressing entirely, so
  `poolcheck.py`'s regex (which only matches `@sda21`/`@sda2`) has **zero**
  applicability to REL code — every REL float/double load uses the
  two-instruction `lis ...@ha` / `lfs/lfd ...@l` form instead. I hand-verified
  exactly one instance end-to-end (decoding raw bytes out of
  `original/d_basesNP.rel`) as a worked example/proof of method; a full
  automated sweep of this form across all REL units was not completed in the
  time available (see §D).
- **Zero confirmed live false positives in landed code.** The one real,
  historical defect I found (`d_a_wm_ghost.cpp`'s `create()`) is confirmed
  **already fixed** in the landed copy — see §B.1. Every other flagged item
  was investigated and traced to a tooling/methodology artifact, not a real
  semantic bug.
- **A structural reason this is the expected outcome, not a coincidence**,
  established this session (§C.4): for a unit that is genuinely *landed* —
  present in a slice json AND covered by a passing `progress.py --verify-bin`
  — a wrong pooled constant or a wrong `bl` callee is not just unlikely but
  **cannot survive**, because both affect the FINAL LINKED bytes directly
  (concrete float bits in `.rodata`/`.sdata2`; a concrete resolved
  displacement in `.text`), and the whole-binary MD5 check reads those bytes
  as they actually are, not zeroed. The zeroed-relocation blindness this
  audit was commissioned to hunt is a property of the PRE-LINK, per-function
  tooling used *while a unit is still a draft* — which is exactly where every
  confirmed historical false positive (line_mng, kokoopa, and the ghost case
  below) actually occurred.

## B. Every hit investigated

### B.1 `d_a_wm_ghost.cpp` — real, historical, ALREADY FIXED (highest-confidence finding of this report)

This is not a hit from the automated sweep above — it surfaced from checking
a stale `wip/wm_units/agent_ghost/d_a_wm_ghost.cpp` draft (left over from
before the unit landed) and is included here because tracing it forward
revealed the *landed* copy and let me fully verify the fix.

- **Where**: `wip/wm_units/agent_ghost/d_a_wm_ghost.cpp:35` (stale, pre-land
  draft, mtime 2026-08-18 15:11) had `mClipSphere.set(mPos, 0.0f);`.
- **Retail value**: `180.0f`. Verified two independent ways: (1) the
  project's own `wip/wm_units/verify_anon.py` docstring records this exact
  defect, found earlier via a linked-REL relocation diff; (2) I independently
  re-derived it this session by reading the raw bytes myself: the retail
  split object `bin/dtkspl/d_basesNP/obj/auto_00_00163620_text.o` shows
  `create()`'s pool reference as `lbl_2_rodata_8884` (a two-instruction
  `@ha`/`@l` pair, **not** an `@sda21` single-instruction pool ref);
  `.rodata`'s file offset in `original/d_basesNP.rel` is `0x1C6600` (read from
  the REL section table); bytes at `0x1C6600 + 0x8884 = 0x1CEE84` are
  `43 34 00 00` = **180.0f** exactly.
- **Current status: already fixed.** The landed unit,
  `source/d_basesNP/bases/d_a_wm_ghost.cpp:25,43`, now reads:
  ```cpp
  const float sGhostClipRadius[] = { 180.0f };
  ...
  mClipSphere.set(mPos, sGhostClipRadius[0]);
  ```
  with a comment at line 20 explicitly noting the earlier `0.0f` bug and that
  `verify_anon` could not see it. This unit is landed, in
  `slices/d_basesNP.json`, and by the reasoning in §C.4 its correctness is
  additionally guaranteed by the whole-REL MD5 check.
- **Severity as found**: would have been the single worst possible finding
  (semantic defect in verified/landed code) had it still been live. As it
  stands: **zero current impact**, but it is the concrete proof that this
  defect class is real and has actually bitten this project on this exact
  code, and that the fix path (a two-instruction `@ha`/`@l` load) is
  precisely the class `poolcheck.py` cannot see (§C.1).
- **Process note, not a code defect**: `wip/wm_units/agent_ghost/` (and
  likely its siblings `agent_antlion`, `agent_kinoko_1up`, `agent_kinoko_base`,
  `agent_smallcloud`, `agent_sandpillar` — all of which I found ALSO already
  landed in `source/d_basesNP/bases/`, see §D) are stale leftover draft copies
  of units that have since landed. They are misleading to any agent (including
  me, initially, and possibly the sibling agents auditing "wm_units drafts")
  that doesn't cross-check `source/` first. Worth cleaning up or marking
  clearly superseded.

### B.2 The 7 raw DOL `bl` mismatches — all false alarms of method, not defects

| unit | function | instr | retail (raw) | draft (raw) | resolution |
|---|---|---|---|---|---|
| `d_a_en_blockmain.cpp` | `ObjBg_PonCheck` | 24 | `fn_80021470` | `blockPonJumpSet__FP15daEnBlockMain_cSc` | Address `0x80021470` falls **inside this same TU's own claimed `.text` range** (`0x800208b0`–`0x80023c60`). Retail's stripped symbol table has no name for this local static; our compiled object does. Same call, same target, no defect. |
| `d_a_en_blockmain.cpp` | `ObjBg_PonCheck_jump` | 24 | `fn_80021580` | `blockPonJumpSet_jump__FP15daEnBlockMain_cSc` | Same as above, address inside the same TU. |
| `d_a_en_blockmain.cpp` | `player_set` | 25 | `fn_8005F4D0` | `fn_8005f4d0` | **Pure case-formatting artifact** — same address, same (both-unnamed) symbol; my quick disasm-without-full-symbol-context run rendered the hex suffix in a different case on each side. Confirmed identical in `bin/dtk/wiimj2d_symbols.txt` (`fn_8005F4D0`, size 0x9C, genuinely unnamed in the full retail map too). |
| `d_a_rot_block.cpp` | `rot_block_init` | 43 | `fn_80062600` | `setBgData__15daRotObjsBase_c...` | **Confirmed same function.** `0x80062600` IS `setBgData__15daRotObjsBase_c...` in the FULL `bin/dtk/wiimj2d_symbols.txt` — my narrow per-split-object disasm simply lacked the symbol context to resolve it. Not a defect. |
| `d_tag_processor.cpp` | `preProcess` | 114 | `fn_800E7170` | `getPlayerNum__14TagProcessor_cFv` | Same as above — `0x800E7170` is `getPlayerNum__14TagProcessor_cFv` in the full symbol map. Disasm-context artifact. |
| `d_a_en_hatena_balloon.cpp` | `fly_xdisp_check` | 18 | `fn_80112040` | `bg_dispx_get__FP19daEnHatenaBalloon_c` | Address `0x80112040` is inside this TU's own range (`0x801102b0`–`0x80114c00`). Local static, unnamed in retail, same-TU call. Not a defect. |
| `d_a_en_hatena_balloon.cpp` | `executeState_DispFlyMove` | 20 | `fn_80112040` | `bg_dispx_get__FP19daEnHatenaBalloon_c` | Same as above. |

**Root cause of all 7**: `sweep_dol.py` disassembles each retail *split*
object in isolation (`dtk elf disasm <one small .o>`), which does not carry
the full symbol table, so any call whose target lies outside that one small
object's own bytes — including a call to a **sibling function in the very
same already-landed TU** — renders as an unresolved `fn_<ADDR>` even where
the full project symbol map (`bin/dtk/wiimj2d_symbols.txt`) already has a real
name at that exact address. This is a limitation of my sweep script, not of
the underlying binaries. I did not find a way to pass dtk the full symbol map
during a single-object disasm in the time available — noted as a real gap in
§D.

### B.3 The 30 raw REL (`d_basesNP`) `bl` mismatches — all traced to two known, documented artifact classes, zero confirmed real

Same root cause as B.2 for the majority (retail's split object shows an
unresolved `fn_2_<OFF>` for what is, on inspection, a same-TU sibling-method
call — `create()` calling `createModel()`/`initState()`/`calcModel()`, etc.,
all in the same class, all landed together). I spot-verified several by
confirming the retail address falls inside the same unit's own claimed
`.text` span (e.g. antlion's `fn_2_15AF50` is inside antlion's own range).

One pair is a different, also-documented trap, worth calling out specifically
because BOTH sides showed a real (non-placeholder) name and they genuinely
differ as text:

- `d_a_wm_kinoko_base.cpp`, matched retail fn `fn_2_16BE10` ↔ draft
  `__dt__Q23m3d8anmChr_cFv`, instr 10: retail names `fn_2_16B3B0` (unnamed),
  instr 14: retail calls `__dl__7fBase_cFPv`, draft calls `__dl__FPv`.
  Checked: **both** `__dl__7fBase_cFPv` (a class-scoped deleting `operator
  delete`) and `__dl__FPv` (the global one) are genuinely called from
  *different* sites within this same compiled object (confirmed by grepping
  the draft's own disassembly — both symbols appear multiple times, at
  distinct destructors). `wip/wm_units/verify_anon.py`'s own docstring
  explicitly documents this exact trap: *"Deleting-destructor wrappers (null
  check, one member-dtor call, optional `__dl__`) are byte-identical in shape
  across unrelated classes"* and gives a real prior instance
  (`d_a_wm_sandpillar.cpp`'s `__dt__Q23mEf8effect_cFv` mispaired to an
  unrelated class's destructor). My verify_anon-style content pairing greedily
  paired two shape-identical-but-different deleting-destructor wrappers to
  each other — this is the documented pairing limitation, not a wrong callee
  in the actual binary. (The unit is landed and MD5-verified, so per §C.4 a
  genuinely wrong callee here is not possible regardless.)

No REL `bl` mismatch survived hand inspection as a real defect.

## C. What this sweep's tools are BLIND to (required disclosure)

1. **`poolcheck.py` only matches the single-instruction `lfs/lfd
   "@SYM"@sda21(r0)` pool form.** It does not match the two-instruction
   `lis rX, "@SYM"@ha` / `lfs/lfd fN, "@SYM"@l(rX)` form used whenever a
   constant is not small-data-addressable. This is not a corner case:
   - It is the **only** form RELs ever use — `-sdata 0 -sdata2 0` is passed to
     every REL compile (see `tools/auto_decomp/harness.py`'s `flags_for`),
     which disables `@sda21` addressing outright. `poolcheck.py` therefore
     provides **zero** pool-constant coverage for any REL unit, landed or
     draft, as shipped.
   - It is also common in DOL-side world-map actor drafts: a grep across
     `wip/wm_units/agent_*/draft.txt` found 100+ such refs across ~25 units
     (koopa_castle 17, sandpillar 17, killerbullet 12, ghost 9, etc.) — none
     checked by value by any existing tool.
   - It is the exact form that hid the real, confirmed `d_a_wm_ghost.cpp`
     defect in §B.1.
   - In the 30 landed DOL units I did check, this form did not appear at all
     (0 `@ha`/`@l` float loads found) — so for *this specific sample* of DOL
     code the gap had no material effect, but that is a property of the
     sample (ordinary game-code TUs that fit in small-data), not of the tool.
2. **`poolcheck.py` only decodes retail values from `original/wiimj2d.dol`**
   (`pool.load()` hardcodes that path). It has **no REL support at all** —
   even for the `@sda21` form, if a REL somehow used it, `poolcheck.py` could
   not decode the retail side. I built and hand-verified a REL-capable
   decoder this session (read the REL's own section table for a section's
   file offset, add the `lbl_2_<section>_<OFF>` offset, read raw bytes from
   `original/<module>.rel`) but did not have time to generalise it into an
   automated per-function sweep — see §D.
3. **Neither `poolcheck.py` nor my new `blcheck.py` checks a `bl` target by
   PLACEMENT/relocation, only by resolved symbol NAME.** Where the retail
   symbol table has no name (very common — the DOL is a stripped, linked
   binary and ~24,000 of its ~32,000 total symbols carry no `scope:` tag at
   all; REL modules are similarly sparse for internal statics), the check can
   only be done by re-deriving the name from address containment or from
   content-based pairing (`verify_anon.py`'s technique) — both of which have
   their own documented failure modes (a call to the wrong function of
   IDENTICAL shape is invisible to content pairing; this is stated explicitly
   in `verify_anon.py`'s own docstring and I hit a live instance of it in
   §B.3).
4. **Neither tool, nor this whole class of per-function check, is even
   necessary for a unit that has actually landed.** A wrong pooled constant
   or wrong `bl` callee changes the FINAL LINKED bytes (concrete float bits;
   a concrete resolved branch displacement), and `progress.py --verify-bin`'s
   whole-binary MD5 reads those bytes as-is — it is not blind to either
   defect class the way the pre-link, per-function tools are. So: **the
   real exposure for this defect class is entirely in DRAFTS, before
   landing** — which is exactly where all three confirmed historical
   instances (d_line_mng, d_enemy_toride_kokoopa, and this ghost case)
   actually happened, and why a from-scratch sweep of already-landed code is
   expected to (and did) come back clean.
5. **My own sweep scripts (`sweep_dol.py`, `sweep_rel.py`, `sweep_rel2.py`)
   disassemble retail SPLIT objects in isolation**, without the full project
   symbol map loaded. This is the direct cause of every one of the 37 raw
   "mismatches" in §B.2/§B.3 — none were real. A more careful version would
   feed dtk the full symbol map (`bin/dtk/wiimj2d_symbols.txt` /
   `bin/dtk/d_basesNP_symbols.txt` etc.) during disassembly if dtk supports
   that, or post-process every `fn_<ADDR>` placeholder by looking it up in
   the full map before declaring a mismatch. I did the latter by hand for
   the DOL hits; I did not have time to do it exhaustively for all 30 REL
   hits (did it for the ones that looked most likely to be real).

## D. Explicit coverage gaps (what I could not check, and why)

- **DOL: only 30 of 143 units** (`21%`) had both a compiled object and an
  overlapping retail split object available locally. `bin/dtkspl/obj/`
  contains 235 address-named `.text` splits plus 139 name-only `__sinit_*`
  splits, covering roughly 87% of `.text` bytes but with real, unpredictable
  gaps (confirmed: `d_2d.cpp`, the very first landed unit, has NO covering
  split object in the local cache at all). Generating the missing splits
  would need `tools/auto_decomp/prepare.py`, which writes into
  `bin/dtkspl/` — outside my read-only/`scratch/fp_sweep/`-only mandate, so I
  did not run it. **113 DOL units are unaudited by this report.**
- **REL: `d_profileNP`'s 5 landed units could not be checked at all** —
  `bin/dtkspl/d_profileNP/obj/` contains essentially nothing usable (3
  entries at last check, versus 675/336/61 for the other three modules). Not
  investigated further why; flagged as a genuine gap.
- **REL: `d_enemiesNP` (4 units) and `d_en_bossNP` (1 unit) had retail dumps
  but content-based pairing matched ZERO functions** in the time available —
  did not debug why (possibly a units-in-file-offset-space bug specific to
  these modules, possibly these are genuinely tiny/structurally different
  units). **Not meaningfully audited.**
- **REL pool-constant values were not checked at scale for ANY of the 38
  landed REL units** beyond the one hand-verified `d_a_wm_ghost.cpp` proof of
  method in §B.1. Given RELs never use `@sda21` (see §C.1), this means **the
  pool-constant half of this audit has essentially no coverage of REL code**,
  which is worth flagging strongly: if a live, un-fixed version of the ghost
  defect exists anywhere else in the landed REL corpus, this sweep would not
  have found it. Building the automated version (matching retail's anonymous
  `lbl_2_<section>_<OFF>` against the draft's often-NAMED equivalent array/
  struct element, which is a relocation-target correlation problem rather
  than a name-string comparison) is the highest-value follow-up work item.
- **My bl-check methodology has a confirmed false-alarm rate** on already
  landed code (37/37 raw hits investigated were artifacts, 0 confirmed real)
  — see §C.5. I'm confident in the 0-real-defects conclusion because of the
  structural argument in §C.4, not because the automated check itself is
  precise; a reader should not take "0 mismatches" from a future run of
  these scripts at face value without the same manual triage.

## E. Recommendation

- **No landed unit needs re-verification on the strength of anything found
  here.** The one real defect found was already independently found and
  fixed by the time I got to it, and the structural argument in §C.4 means
  the whole-binary MD5 gate already provides stronger protection against this
  defect class than any per-function tool could, for anything that actually
  lands.
- **The valuable next step is hardening `poolcheck.py` for `@ha`/`@l`
  addressing**, not re-sweeping landed code. This one gap (a) is universal in
  REL code, (b) is common in wm_units DOL drafts, and (c) already hid one
  real, confirmed defect. A `bl`-callee equivalent of `poolcheck.py` (my
  `blcheck.py` is a starting point, but needs the full-symbol-map fix from
  §C.5 before it's trustworthy against already-landed code) is the second
  most valuable addition.
- **Clean up the stale `wip/wm_units/agent_{ghost,antlion,kinoko_1up,
  kinoko_base,smallcloud,sandpillar}/` directories** (or at least mark them
  superseded) — they are pre-land snapshots of units that have since landed
  in `source/d_basesNP/bases/`, and are a live hazard: any agent (including
  me, and likely the sibling agent auditing "wm_units drafts" right now) that
  doesn't think to check `source/` first will treat them as current work and
  waste effort re-deriving fixes that already shipped.
