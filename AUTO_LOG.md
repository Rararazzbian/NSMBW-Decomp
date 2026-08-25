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

## dol/bases/d_random.cpp (dRandom_c::calcMachineRandom) — 0/1 byte-exact — PARKED
Target fully decoded: CRC32 over {u64 time, nick[0x18], 4x RandRec{3 floats via
two mVec3_c copies, raw pos, dist}} from mPad::g_core[i], do-while ++i<=3,
frame 0xd0 with _savegpr_27. Best drafts: 61w/58 diffs (cursor form) and
63w/54 diffs (sw27 k1). Blocker: retail lowers both array walks as pinned base
+ running byte IV + per-iteration frame-base rebuild; every source spelling in
~50 variants across sweeps 12-27 collapses to cursor form or mulli instead.
Full state + ruled-out list in READY_TO_LAND.md PARKED section. Claiming
dFunsuiAct_c next per stall rule.

## dol/bases/d_funsui_act.cpp (dFunsuiAct_c::posMove) — 1/1 — LANDED 11.405% -> 11.410%
Resumed prior session's claim (best was swB_B6 83w/81w/72). Two levers closed
it: (1) naming the getCenterPos receiver through a dBaseActor_c* alias moves
`mr r31,r3` from the switch preamble into case 3 where retail has it; (2) an
mVec2_c declared FIRST in case 3 and read back through for both member writes
leaves retail's dead float-pair stores at r1+0x8/0xc and shifts every later
temp slot up 8 bytes to match. cs_rev_speed is a function-local static whose
mangled name reproduces retail's exactly; its .sdata2 block (0x8042C840, 16 B
incl. two anonymous pool floats) is claimed by this TU. Five binaries verified.

## dol/bases/d_lang_loc_string.cpp (LangLocString::LangLocString) — 1/1 — LANDED 11.410% -> 11.416%
Byte-exact on the FIRST compile: 45 char* members assigned inline locale-code
string literals ("JPJpn".."CM") in offset order reproduce retail's schedule
exactly (28 literal addresses hoisted into r4-r31 under _savegpr_14, then
sequential stores). MWCC emits each literal with alignment = size rounded to
pow2 (6-7 byte strings at 8-byte stride, 3-4 byte at 4-byte stride), which
tiles .sdata 0x804293C0-0x804294E4 exactly. No external symbols.

## dol/bases/d_rom_font_manager.cpp (dRomFontMgr_c) — 2/2 — LANDED 11.416% -> 11.420%
load_resource matched first try. createInstance needed two shape fixes:
(1) containment, not inheritance — deriving from RomFont made MWCC emit a
vtable store retail does not have; a plain first member calls the same base
ctor with none. (2) zeroing mBuffer/mLoaded belongs in an in-class ctor so
the new-expression's own null-check covers it — an explicit `if (inst)`
in the caller duplicates the check (+2 words). heap->alloc compiles to
vtable slot 0x14 exactly as the frozen eggHeap.h lays it out.

## dol/bases/d_dvd_error_wide_msg.cpp (dDvdErrorWideMsg_c) — 2/2 — LANDED 11.420% -> 11.424%
Resumed prior session's claim (both functions already DIFFS 0; the working
tree had a broken edit: 15 initializer rows for a [14][3] table). Retail
truth: sMsgBase is exactly 14 rows — symbol map's 0x9C + anonymous
lbl_803224E4 (0xC) tile it; initialize's loop runs i<14 and fn_80107AF0
reads row 13. Prior final.o had one wrong word (row 3 mid pointer). Fixed
table, hex-verified both functions and all 42 pointers against retail DOL.
Two land.py rejections taught the real slice contract (see AGENT_CONTEXT):
claims must match the compiled object's per-section sizes EXACTLY; my first
manifest over-claimed .data by 8 bytes into the next TU's table and shifted
132k downstream bytes. Second rejection was the same bug on .text (claimed
through next symbol instead of object end); fixed by claiming .text
0x101220-0x101338 / .data 0x23da8-0x23e50.

## dol/bases/d_rot_shake.cpp (dRotShake_c) — 2/2 — LANDED 11.424% -> 11.429%
Resumed prior session's claim (attempt 1; init already DIFFS 0, move stuck at
30 diffs as a clean GPR permutation: retail keeps v=r5/x=r6 and folds the
m000+q sum into m000's register; every draft form `x = m000 + q` folded the
sum into the quotient register r0 instead). Fix found on variant sw17 of 21:
split the head into compound-add form -- `s16 x = m000; x += -v / m004;` --
which makes the add update x's register exactly like retail. Tail then fell
into place with no further edits. init 10/10, move 63/63 words, hex-diff
zero, land.py ACCEPTED first try. Slice .text 0xd91d0-0xd9300; no external
symbols (the unit performs zero calls). Lever written up in AGENT_CONTEXT.

## dol/bases/d_s_restart_crsin.cpp (dScRestartCrsin_c) — 2/2 — LANDED 11.429% -> 11.434%
Resumed prior session's claim: scratch was already byte-exact and the landing
was fully staged but never run (stale tree blocked land.py; source/ copy also
held a pre-final cpp). Re-verified everything before gating: fndiff MATCH both,
wdiff 0 differing words, object .text 0x168/.rodata 0x1E/.bss 0x10 vs claims
0x170/0x1E/0x10, all five externals resolvable (setFader pre-pinned), headers
shadow==include additive-only. The one open question — claiming .text through
8 bytes of trailing pad past object end (dRotShake precedent) vs exact-size
(dvderr lesson) — settled empirically: retail bytes at VA 0x80101A48-50 are
ZERO, and the next filler's natural alignment (16) forces the ld to pad our
object by exactly those 8 bytes, so claim-through-pad reproduces retail.
land.py ACCEPTED first try. Manifest mechanics recorded in AGENT_CONTEXT.

## dol/bases/d_pos_shake.cpp (dPosShake_c) — 3/3 — LANDED 11.434% -> 11.439%
Resumed prior session's claim (attempt 1 stalled at a clean 20-word f1/f2
register mirror on move(); ~24 swept variants all converged on the same swap).
Fix was OUTSIDE the swept axis: move() returns f32 -- CFront mangling omits
return types so `move__11dPosShake_cFv` proves nothing, and retail keeps x in
f1 = the float return register for the whole function. `return x;` costs zero
extra instructions (epilogue stores leave it in place); the prior session's
s14/s15 had tried f32 with `return vel;` and paid +1 word for an fmr. init
7/7, startShake 4/4, move 61/61, hex-diff zero, land.py ACCEPTED. Slice .text
0xd1a20-0xd1b50, .sdata2 0x1a88-0x1a8c; no external symbols; new header.
Lever written to AGENT_CONTEXT.

## dol/bases/d_message.cpp (MsgRes_c) — 4/4 — LANDED 11.439% -> 11.443%
Ctor/dtor/getScale/getFont byte-exact in three compiles. Layout solved by
reading the BASE ctor: EGG::MsgRes zeroes 0x0-0x18 then stores &__vt__Q23EGG6MsgRes
at this+0x1C, and buildMsgRes allocates 0x20 -- so the class is [0x1C fields]
[vptr@0x1C]. MWCC appends a polymorphic class's implicit vptr AFTER declared
data members (not at offset 0): an @unofficial u8[0x1C] block places it exactly.
Declaring my own void* vptr member double-books the slot (vptr lands at 0x20).
Vtable {0,0,&__dt__8MsgRes_c} reproduced automatically once ~MsgRes_c() is the
class's only virtual and is defined out-of-line. getScale/getFont pass r3-r5
straight through to getMsgEntry(this,g,id) and lhz/lbz offsets 0x4/0x6.
eggMsgRes.h expanded ADDITIVE + committed before landing; gate ACCEPTED first
try. Slice .text 0xc8070-0xc8170, .data 0x19618-0x19624. Lever written to
AGENT_CONTEXT.

## dol/bases/d_player_effect_manager.cpp (dPyEffectMng_c) — 4/4 — LANDED 11.443% -> 11.449%
Byte-exact on the FIRST compile. The prior analysis session's header was so
complete that only bodies were needed. Two findings worth keeping:
fn_800d2de0 returns bool (mangling hides it again -- third unit running;
landed d_a_player.cpp discards the result at all four call sites so the
void->bool header fix is codegen-neutral there, proven by the gate). And the
unnamed per-slot helper fn_800D2BB0 is callable via an extern "C"
declaration taking the slot as explicit this -- no invented member name
needed. Ctor/dtor are pure __construct_array/__destroy_arr machinery.
Slices .text/.data/.sbss all matched object sizes exactly. dPyEffect_c's own
TU portion (0x800D2AE0-0x800D2D10) is a separate future unit.

## dol/bases/d_a_zoom_pipe_base.cpp (daZoomPipeBase_c) — 1/2 — PARKED (Attempts 3)
execute__16daZoomPipeBase_cFv byte-exact (53/53, DIFFS 0). init__16daZoom-
PipeBase_cFUi stuck at 20 diffs after three sessions/~100 swept variants ->
parked per stall rule. Session-4 decode: source member/nibble/constant map is
CORRECT; the residual is purely WHICH int->double stack slot-pair each
conversion gets. Retail: mSpeed[1]'s conv reuses pair A {0x8,0xC}, mSpeed[0]'s
takes fresh pair B {0x10,0x14} (their live ranges overlap); draft inverts the
pairing because its schedule has no overlap. Untried lever recorded in
READY_TO_LAND.md: force the lfd of the second conversion to schedule after
the stfw of the third. State committed for instant resume.

## dol/bases/d_kokoopa_sp_fumi_check.cpp (KokoopaSpFumiCheck_c) — 2/2 — LANDED 11.449% -> 11.456%
Resumed own IN PROGRESS claim (attempt 6) and found the "one GPR permutation
left" from attempts 4-5 was already solved by variant s8 sitting in sw_perm
since session 3 — the fix was swapping the fake FumiPlayerView/FumiEnemyView
structs for the REAL daPlBase_c/dEn_c headers; only the 4 remaining diffs were
canonicalisation artifacts of its extern pool pins. Replacing them with plain
float literals made MWCC emit the TU's own anonymous .sdata2 {0.0f,4.0f,10.0f}
at 8042C768-74. First gate run REJECTED: definition order is link order --
s9's dtor-first object linked dtor at the claim start; retail has operate
first. s10 reordered, ACCEPTED second run. Vtable {0,0,&dt,&op} proves operate
is virtual; header change purely additive (appended standalone class to the
existing shared d_en_fumi_check.hpp). Linker mechanics (weak-copy drop +
balign pad survival) recorded in AGENT_CONTEXT.
