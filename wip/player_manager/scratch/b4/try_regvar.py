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

typedef s8 &(*GetPlrNoFn)(dActor_c *);

"""

variants = {}
variants['v_register_kw'] = """
daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            register void *p asm("r12");
            p = *(void **)((u8 *)base + 0x60);
            p = *(void **)((u8 *)p + 0x6c);
            if (((GetPlrNoFn)p)((dActor_c *)base) == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}
"""

for name, body in variants.items():
    with open(SRC, 'w') as f:
        f.write(HEADER + body)
    ok, out = harness.compile_draft(SRC, OBJ)
    if not ok:
        print(name, "COMPILE FAILED")
        print(out[:2000])
        continue
    ok, out = harness.disasm(OBJ, TXT)
    if not ok:
        print(name, "DISASM FAILED", out)
        continue
    matched, msg = harness.diff_fn(TARGET, TXT, "getYoshi__9daPyMng_cFi")
    print(name, "MATCH" if matched else "MISMATCH")
    if not matched:
        print(msg)
    print()
