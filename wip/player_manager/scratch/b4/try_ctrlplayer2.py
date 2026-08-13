import sys, os
sys.path.insert(0, r"tools/auto_decomp")
import harness

ROOT = os.getcwd()
SRC = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "try.cpp")
OBJ = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "try.o")
TXT = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "try.txt")
TARGET = os.path.join(ROOT, "wip", "player_manager", "target_text.txt")

HEADER = """#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/framework/f_manager.hpp>

"""

variants = {}

variants['v2'] = """
dAcPy_c *daPyMng_c::getCtrlPlayer(int plrNo) {
    dAcPy_c *player = getPlayer(plrNo);
    if (player == nullptr) {
        return nullptr;
    }
    daYoshi_c *yoshi = player->getRideYoshi();
    if (yoshi != nullptr) {
        return (dAcPy_c *)yoshi;
    }
    return player;
}
"""

for name, body in variants.items():
    with open(SRC, 'w') as f:
        f.write(HEADER + body)
    ok, out = harness.compile_draft(SRC, OBJ)
    if not ok:
        print(name, "COMPILE FAILED")
        print(out)
        continue
    ok, out = harness.disasm(OBJ, TXT)
    if not ok:
        print(name, "DISASM FAILED", out)
        continue
    matched, msg = harness.diff_fn(TARGET, TXT, "getCtrlPlayer__9daPyMng_cFi")
    print(name, "MATCH" if matched else "MISMATCH")
    if not matched:
        print(msg)
    print()
