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

variants['v1_no_actor_var'] = """
daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    for (int i = 0; i < 4; i++) {
        fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (((dActor_c *)base)->getPlrNo() == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}
"""

variants['v2_base_outside'] = """
daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (((dActor_c *)base)->getPlrNo() == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}
"""

variants['v3_i_outside'] = """
daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    int i = 0;
    while (i < 4) {
        fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (((dActor_c *)base)->getPlrNo() == plrNo) {
                return (daYoshi_c *)base;
            }
        }
        i++;
    }
    return nullptr;
}
"""

variants['v4_return_int_swap'] = """
daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    for (int i = 0; i < 4; i++) {
        fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base == nullptr) {
            continue;
        }
        if (((dActor_c *)base)->getPlrNo() == plrNo) {
            return (daYoshi_c *)base;
        }
    }
    return nullptr;
}
"""

variants['v5_plrno_var'] = """
daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    for (int i = 0; i < 4; i++) {
        fBase_c *base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            s8 pn = ((dActor_c *)base)->getPlrNo();
            if (pn == plrNo) {
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
        print(out)
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
