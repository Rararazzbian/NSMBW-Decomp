# Codex Handoff

Separate from HANDOFF.md (Claude's). Do not edit that file.

## Session start state
- Branch: claude/game-decompilation-setup-bw30s7
- Overall: 10.657% (692728 / 6500368)
- wiimj2d.dol: 20.968%
- Five binaries verified clean before work.
- 34 commits were already unpushed. Not pushed here.

## Work done this session

### 1. Landed cCounter_c::clear
- New file: source/dol/cLib/c_counter.cpp
- Function: void cCounter_c::clear(), 16 bytes at 0x8015FF80
- Body zeroes static members m_gameFrame and m_exeFrame.
- Header already existed: include/game/cLib/c_counter.hpp.
- Added definitions for the two static u32 members in the cpp (required by linker).
- Slice entry added to slices/wiimj2d.json between:
  - dol/sLib/s_StateMethodUsr_FI.cpp (.text 0x159650-0x159800)
  - dol/cLib/c_dylink.cpp (.text 0x159810-0x15a490)
  New ranges:
  - .text 0x159800-0x159810
  - .sbss 0x7a8-0x7b0
- Verified with python configure.py, ninja, python progress.py --verify-bin:
  all five binaries OK.

Gotcha learned: CodeWarrior rejects UTF-8 BOM. Write .cpp files without BOM.
(Claude handoff uses word counts and process notes, not this small target.)

## Next candidate targets (surveyed read-only)
Ranked small non-actor support functions, header exists where noted:
1. mColor::lerp, 0x8016ABB0, 276 B, offset 0x164430, gap between
   dol/mLib/m_angle.cpp and dol/mLib/m_color_fader.cpp.
   Header include/game/mLib/m_color.hpp exists.
2. MsgRes_c ctor + 3 siblings, 0x800CE7F0, total 220 B, offset 0xC8070,
   gap d_mj2d_data.cpp / d_multi_manager.cpp. Header d_message.hpp exists.
3. sPrintf / OSReport group, 0x8015F810, total 216 B, offset 0x159090,
   gap s_Phase.cpp / s_StateID.cpp. OSError.h exists, sPrintf decl missing.
4. dCurtainMng_c::CurtainInfoAllClear, 0x8008ECC0, 176 B, offset 0x88540,
   gap d_cd.cpp / d_cyuukan.cpp. Class header missing.
5. sCrc::calcCRC32, 0x8015F270, 4 B, offset 0x158AF0, header missing.
6. cLib utilities: memSet 4 B, targetAngleY 28 B, targetAngleX 104 B around
   offset 0x15A490-0x15A860; headers missing for these exact names.

## Process for next unit
- Keep lead/agent split: agents author/investigate, lead integrates only.
- Use small/fast models for read-only surveys and browsing-like investigation.
- Use highest-capability models for actual decompilation/code-authoring work.
- Do not run parallel ninja integrations in the same checkout.
- Add every newly banked non-actor unit to the right slice file, not just wiimj2d.
- Keep writing this file instead of HANDOFF.md.
