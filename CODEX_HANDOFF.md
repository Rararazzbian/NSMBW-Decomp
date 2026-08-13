# Codex Handoff

Separate from HANDOFF.md (Claude's). Do not edit that file.

## Session start state
- Branch: claude/game-decompilation-setup-bw30s7
- Overall: 10.657% (692728 / 6500368)
- Five binaries verified clean before work.
- 34 commits were already unpushed.

## Work done this session

### 1. Landed cCounter_c::clear
- New file: source/dol/cLib/c_counter.cpp
- Function: void cCounter_c::clear(), 16 bytes at 0x8015FF80
- Verified: all five binaries pass.

### 2. mColor::lerp ? implemented but NOT byte-exact (ABI contradiction)
- Source: source/dol/mLib/m_color.cpp (implemented, correct logic but wrong ABI)
- Header: include/game/mLib/m_color.hpp (static declaration added)
- The mwcceppc compiler returns 4-byte PODs in r3; target uses sret (r3=out ptr)
- Root cause: nw4r::ut::Color() does `*this = WHITE`, compiler optimizes through it differently than original
- ABI mismatch cannot be resolved without changing shared header (nw4r::ut::Color)
- Slice entry proposed but NOT applied: .text 0x164430-0x164550, .sdata2 0x2c60-0x2c70, no other sections
- Full analysis in CODEX_RESPONSE.md

## Next candidate targets
1. MsgRes_c ctor + 3 siblings, 0x800CE7F0, total 220 B, offset 0xC8070
   Gap: d_mj2d_data.cpp / d_multi_manager.cpp. Header d_message.hpp exists.
2. sPrintf/OSReport group, 0x8015F810, total 216 B, offset 0x159090
3. dCurtainMng_c::CurtainInfoAllClear, 0x8008ECC0, 176 B, offset 0x88540
4. cLib utilities around 0x15A490-0x15A860

## Gotchas learned this session
- CodeWarrior ABI for struct return: 4-byte structs with user-declared dtors should use sret per CW docs,
  but `-ipa file` inlines through trivial dtors and switches to register return.
  This is a header-level contradiction ? cannot fix from source alone.
- The WHITE-initializing nw4r::ut::Color() constructor emits `li r8, -0x1; stw r8, 0(r3)` that does
  NOT get optimized away even when all 4 bytes are subsequently overwritten with stb.
- Sub-agents work well for surveys but the compile-iterate-decompile loop benefits more from
  direct iteration than delegation.

## Rules
- Never run ninja/configure.py/progress.py/land.py
- Never edit slices/wiimj2d.json or syms.txt
- Write to CODEX_RESPONSE.md, not HANDOFF.md
