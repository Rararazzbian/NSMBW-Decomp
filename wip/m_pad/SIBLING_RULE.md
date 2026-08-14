Sibling-register investigation for mPad::clearWPADInfo (0x8016F640)
====================================================================

Scope: not a 22nd source shape. This is a corpus search over the 145
landed, verifying wiimj2d TUs for the idiom

    <indexed load/store using rBase, rIndex>
    add   rD, rBase, rIndex        ; fold base+index for reuse
    <more accesses through rD at fixed displacements>

and a rule for which of {rBase, rIndex} the compiler picks as rD.

Method
------
- Disassembled every `.o` under `bin/compiled/wiimj2d/` (145 objects,
  100% success) with `bin\dtk-windows-x86_64.exe elf disasm`, into
  `wip/m_pad/scratch/sibling/disasm/`. These are the compiled outputs of
  the units listed in `slices/wiimj2d.json` -- landed and byte-exact by
  construction, so any instruction shape found here is a fact about the
  real compiler, not a guess.
- Scanned every function for an X-form load/store (`l**x`/`st**x`) with
  a non-zero base register, followed within a short window by an `add`
  that recombines the exact same register pair, and recorded which
  register the `add` writes to. Scripts:
  `wip/m_pad/scratch/sibling/find_pattern2.py` (load-first, 20 hits) and
  `wip/m_pad/scratch/sibling/find_store_pattern.py` (store-first, 0
  hits -- see "Negative result" below).

Group A -- clean match for clearWPADInfo's shape (BASE reused)
----------------------------------------------------------------
No intervening branch, no intervening call, and the fold's result feeds
more same-pointer accesses at growing displacements. Three examples,
all reuse the register that held the ADDRESS, never the one that held
the freshly-computed scaled INDEX.

### daEnDpakkunBase_c::checkQuakeDeath (source/dol/bases/d_a_en_dpakkun_base.cpp:395-422)
```cpp
static const mVec2_c cs_check_ofs[4] = { ... };
...
check[0].x = center.x + cs_check_ofs[mPakkunDir].x;
check[0].y = center.y + cs_check_ofs[mPakkunDir].y;
```
```
addi r3, r3, "@LOCAL@...cs_check_ofs"@l   ; r3 = &cs_check_ofs[0]  (BASE)
slwi r0, r0, 3                            ; r0 = mPakkunDir * 8    (INDEX)
lfs  f1, 0x8(r1)
lfsx f0, r3, r0                           ; .x  -- first access, indexed
add  r3, r3, r0                           ; fold -> r3 (BASE's register)
lfs  f4, 0xc(r1)
fadds f5, f1, f0
lfs  f2, 0x4(r3)                          ; .y  -- second access, +4(r3)
```

### daEnDpakkunBase_c::setDeathInfo_Quake (source/dol/bases/d_a_en_dpakkun_base.cpp:458-462)
```cpp
pos.x = mStartPos.x + l_hole_offset[mPakkunDir].x;
pos.y = mStartPos.y + l_hole_offset[mPakkunDir].y;
```
```
lwz  r0, 0x6a8(r3)
lfs  f2, 0x6c8(r3)
slwi r0, r0, 3                 ; INDEX (r0 = mPakkunDir*8, r0 was mPakkunDir here)
addi r5, r5, l_hole_offset@l   ; BASE (r5 = &l_hole_offset[0])
lfsx f0, r5, r0                ; .x  -- first access, indexed
add  r5, r5, r0                ; fold -> r5 (BASE's register)
lfs  f1, 0x4(r5)               ; .y  -- second access, +4(r5)
```

### LytBase_c::AnimeEndSetup (source/dol/bases/d_lytbase.cpp:180-186) -- the example given in the prompt
```
lwz r5, 0x188(r3)      ; flags array pointer, unrelated to the fold below
lbzx r0, r5, r4
cmpwi r0, 0x0
beq .L_00000C3C
mulli r0, r4, 0x28              ; INDEX (r0 = animeIdx * 0x28)
lwz  r3, 0x184(r3)              ; BASE  (r3 = this->mAnmGroupArray, a member pointer)
li   r4, 0x0
add  r3, r3, r0                 ; fold -> r3 (BASE's register)
bl   setAnmEnable__Q23m2d14AnmGroupBase_cFb   ; r3 is also the call's "this"
```
Here the fold happens to land in r3 for two reinforcing reasons at once:
r3 already held the base pointer, AND r3 is the ABI register the
following `bl` needs its `this` argument in. This is consistent with
Group A but also foreshadows Group B below (an ABI/call requirement can
independently force r3).

In all three, the pattern that decides `rD`:
- BASE's defining instruction is an address computation: `addi rX,
  rX, LABEL@l`, `lis/addi`, or `lwz rX, OFS(rY)` of a pointer member.
- INDEX's defining instruction is a magnitude computation: `mulli`,
  `slwi`.
- After the fold, the INDEX's specific scaled value is never read
  again anywhere in the function (confirmed for all three by reading
  the full disassembly, not just the excerpt) -- it was a one-shot
  scalar consumed entirely by the `add`.
- The BASE's role (being "a pointer usable with a small displacement")
  continues immediately afterward.
- The `add` always targets the **BASE's register**, never the INDEX's.
  We found zero counterexamples to this among the 20 load-first hits
  when no branch/call intervenes.

Group B -- boundary cases where a THIRD register (or a call-ABI
register) wins instead, and why they do not apply to clearWPADInfo
-----------------------------------------------------------------------

### LytBase_c::ReverseAnimeStartSetup / AnimePlay (source/dol/bases/d_lytbase.cpp)
```
mulli r30, r31, 0x28      ; INDEX
lwz   r0, 0x184(r29)      ; BASE
lwzx  r3, r30, r0
add   r31, r0, r30        ; fold -> r31, NEITHER r30 nor r0
bl    setFrame__Q23m2d11FrameCtrl_cFf   ; call clobbers volatiles
mr    r3, r31                            ; recovered afterward
bl    updateFrame__Q23m2d14AnmGroupBase_cFv
```
Here the folded pointer must survive an intervening `bl` (which clobbers
r0/r3-r12), so it cannot live in either operand's volatile register; it
is placed in r31, a nonvolatile/saved register, and copied to r3 only
right before the next call needs it. **clearWPADInfo makes no calls at
all**, so this exception cannot fire there.

### m3d::anmMatClr_c::remove / play (and the identical anmTexPat_c /
### anmTexSrt_c siblings -- 6 hits total, same idiom copy-pasted 3x)
```
lwz  r0, 0x28(r28)       ; BASE (vtable-ish array pointer)
lwzx r12, r30, r0        ; INDEX = r30 (mulli'd loop offset)
add  r3, r0, r30         ; fold -> r3, NEITHER r0 nor r30
lwz  r12, 0x10(r12)
mtctr r12
bctrl                    ; the folded pointer is the call's "this"
```
Here the fold's very next use is as a **call argument**, so the
allocator places it directly in r3 regardless of which operand held the
base. **clearWPADInfo's fold result is never passed to any call** (it is
only used for more `stw`/`stb` at fixed displacements, then r4 is simply
reused again for the last `stb`), so this exception cannot fire either.

### dCsvData_c::ReadPointType / ReadFlagData / ReadRouteFlag (5 hits)
```
lwz  r4, 0x0(r26)
lbzx r5, r25, r4
extsb r0, r5
cmpwi r0, 0x22
bne .L_...                 ; <-- conditional branch BEFORE the fold
add  r3, r4, r25            ; fold -> r3, on the branch-not-taken path only
lbz  r0, 0x1(r3)
```
A conditional branch sits between the indexed access and the fold, so
the fold only executes on one control path and the liveness picture at
that point is different from a straight-line function; the destination
here is again a third register. **clearWPADInfo has no branches** (17
straight-line instructions, confirmed against the target disassembly),
so this exception cannot fire either.

### dPyModel_HIO_c::resetParam (source: not checked, single hit)
Two competing folds share one index register (r8, `slwi r8, r4, 4`):
```
add r9, r0, r8      ; first fold: BASE=r0, INDEX=r8 -> dest r9 (fresh, since
                     ;   r8 is needed AGAIN below and r0's later use is unclear)
...
add r4, r6, r8      ; second fold: BASE=r6, INDEX=r8 -> dest r4 (fresh; r4 was
                     ;   the ORIGINAL unscaled parameter register, already dead
                     ;   once r8 = r4<<4 was computed, so it gets recycled)
```
Both folds here land in a register that is neither operand -- but in
both cases it is a register that was independently proven dead just
before the fold (r0's fate is unclear from the excerpt; r4 is provably
dead since its only remaining live value, `r4<<4`, was already
committed to r8). This is consistent with "reuse whichever register is
dead at this exact program point," which is a *superset* of the Group A
rule (Group A's dead register always happens to be the INDEX's,
because in Group A the index truly has no other consumer). It shows the
allocator is not literally choosing "base slot" as a rule engraved in
stone -- it is choosing whatever is dead, and in every branch-free,
call-free case we found, that happens to be the index.

Negative result: no landed STORE-first sibling exists
-------------------------------------------------------
clearWPADInfo's actual shape is store-first: `stwx r0, r4, r5` (write
the first field via indexed store) THEN `add r4, r4, r5` THEN more
`stw`/`stb` at fixed displacements. Every one of the 20 hits above is
LOAD-first (`lfsx`/`lwzx`/`lhax`/`lbzx`). A dedicated search
(`find_store_pattern.py`) for the store-first version of the same
idiom -- an indexed store (`st**x`) followed by an `add` recombining the
same register pair -- returned **zero hits** anywhere in the 145 landed
TUs, despite `st**x` instructions with a non-zero base existing in 16
different files (`stwx`, `stbux`, `stwux` etc., 60+ occurrences
grepped). So this exact fold-after-store idiom is unprecedented in the
landed corpus; everything above is a load-side analog, not a direct
sibling.

The rule, stated plainly
-------------------------
In a straight-line stretch of code (no branch, no call) that computes
`&array[index]` once via `add rD, rBase, rIndex` and then continues to
use `rD` for more field accesses at fixed displacements, `rD` is the
register that held the **address computation** (the array/struct base,
defined by `lwz`, `addi ...@l`, or `lis`+`addi`), not the register that
held the **freshly scaled magnitude** (the index, defined by `mulli` or
`slwi`) -- because after the fold, the index's specific scaled value is
never read again in every clean example we found, while the base's role
as "a pointer good for +0/+4/+8/..." continues. When a call intervenes,
or the fold's result is itself about to be a call argument, or a branch
sits between the indexed access and the fold, a different (ABI- or
liveness-driven) register can win instead -- but none of those
conditions hold in clearWPADInfo.

Does it close clearWPADInfo?
------------------------------
No, and it was not expected to (per the brief). But it is directly
diagnostic:

- clearWPADInfo is call-free, branch-free, 17 straight-line
  instructions, and its fold's result (r4) is used for 6 more stores
  after the fold while the index's scaled value (old r5, `ch*0x18`) is
  never read again -- the SAME shape as Group A in every dimension that
  matters. The rule therefore **predicts base-register reuse**, i.e.
  `add r4, r4, r5` -- which is exactly what the target does.
- The rule gives **no reason** for the 21 failed drafts' `add r5, r4,
  r5` (index reused instead). None of the Group B exceptions apply:
  there is no call for r5 to be an argument to, no call for anything to
  survive across, and no branch. The current draft
  (`wip/m_pad/scratch/merge_lead/m_pad.cpp:114-125`) already has the
  right field order and the right later use of the raw, unscaled `ch`
  (via `s_WPADInfoAvailable[ch] = false;`), which is the only source-
  level feature this investigation identified as relevant to which
  register survives -- and it is already present.
- Conclusion: the corpus supports the target's register choice as the
  *normal* outcome for this shape, which reinforces (rather than
  contradicts) AGENT_CONTEXT.md's existing finding that a pure
  register-permutation residual is "not source-addressable" through
  statement reordering -- here even a source draft with the
  structurally correct field order and correct downstream use of `ch`
  still lands on the wrong side of a choice that, in every landed
  precedent, goes the other way. Whatever tips MWCC from "reuse r4"
  to "reuse r5" in the current draft is not visible as a difference in
  which statements are present or what they reference; it is more
  likely internal IR/live-range numbering, consistent with the
  project's `beginPad` finding that declaration order does not drive
  saved-register assignment.
- What would raise confidence further: a landed, byte-exact **store-
  first** sibling (the Negative Result section) would settle whether
  the load-vs-store direction of the first access itself matters to the
  allocator's choice. None exists yet in this corpus. If a future round
  lands any TU with that shape, it is worth rerunning
  `find_store_pattern.py` against it before spending more attempts on
  clearWPADInfo.

Artifacts
---------
- `wip/m_pad/scratch/sibling/disasm/` -- all 145 landed objects,
  disassembled (`disasm_all.py`).
- `wip/m_pad/scratch/sibling/find_pattern2.py`,
  `pattern2_results.txt` -- the 20 load-first hits.
- `wip/m_pad/scratch/sibling/find_store_pattern.py`,
  `store_pattern_results.txt` -- confirms 0 store-first hits.
- `wip/m_pad/scratch/sibling/find_pattern.py`,
  `pattern_results.txt` -- an earlier, looser scan (416 hits) kept only
  for reference; superseded by find_pattern2.py's tighter base/index
  classification.

Offset-perturbing: NO. This is read-only investigation; no `source/`,
`slices/`, `syms.txt`, or shared header was touched.
