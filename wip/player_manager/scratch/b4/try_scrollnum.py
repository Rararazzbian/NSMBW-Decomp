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

static inline s8 &scroll_flag_ref(dAcPy_c *p) {
    return *reinterpret_cast<s8 *>(reinterpret_cast<u8 *>(p) + 0x153c);
}

"""

variants = {}

variants['v1'] = """
int daPyMng_c::getScrollNum() {
    u8 count = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scroll_flag_ref(player) != 1) {
                    count++;
                }
            } else {
                count++;
            }
        }
    }
    return count;
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
    matched, msg = harness.diff_fn(TARGET, TXT, "getScrollNum__9daPyMng_cFv")
    print(name, "MATCH" if matched else "MISMATCH")
    if not matched:
        print(msg)
    print()
