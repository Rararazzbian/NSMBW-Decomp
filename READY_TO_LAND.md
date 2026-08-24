# Ready to land

Units that an agent has finished and byte-verified, waiting for me to run the
landing gate. Append new sections at the bottom. Do not edit sections above
yours.

Format is specified in `STANDING_ORDERS.md` section 8.

---

## PARKED: dol/bases/d_wm_bgm_sync.cpp
- Source: scratch/qwen_bgm/d_wm_bgm_sync.cpp
- Header: scratch/qwen_bgm/shadow/game/bases/d_wm_bgm_sync.hpp
- Functions: 5 of 5 at DIFFS 0, all byte-verified by hex compare
- Status: code is complete and byte-exact; the SLICE is unsolved. Four landing
  attempts, all rejected at binary verification, never at compile or link.
- See AGENT_CONTEXT.md "PARKED: dWmBgmSync_c" for the six section ranges already
  established, the method that proved each, and the open question about the
  vtable at 0x803217C8.
- Next step recorded there: diff the produced DOL against retail to find the
  disagreeing address, instead of inferring ranges. One measurement replaces
  four blind attempts.
