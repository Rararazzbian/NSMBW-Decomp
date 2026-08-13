import sys, os
sys.path.insert(0, r"tools/auto_decomp")
import harness
import subprocess

ROOT = os.getcwd()
SRC = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "try.cpp")
OBJ = os.path.join(ROOT, "wip", "player_manager", "scratch", "b4", "try.o")

HEADER = """#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/framework/f_manager.hpp>

typedef s8 &(*GetPlrNoFn)(dActor_c *);

static inline GetPlrNoFn get_vfunc_6c(fBase_c *base) {
    return (*(GetPlrNoFn **)((u8 *)base + 0x60))[0x6c / 4];
}

daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (get_vfunc_6c(base)((dActor_c *)base) == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}
"""

with open(SRC, 'w') as f:
    f.write(HEADER)
ok, out = harness.compile_draft(SRC, OBJ)
print("compile ok:", ok)
if not ok:
    print(out)
p = subprocess.run([os.path.join(ROOT,'bin','dtk-windows-x86_64.exe'),'elf','info',OBJ], cwd=ROOT, capture_output=True, text=True)
print(p.stdout)
