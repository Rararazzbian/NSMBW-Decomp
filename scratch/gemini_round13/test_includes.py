import os
import subprocess
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'scratch', 'gemini_round13'))
from test_grid import test_grid_cpp

headers_to_try = [
    '#include <game/bases/d_res_mng.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_cs_seq_manager.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_a_wm_map.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_w_camera.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_info.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_a_wm_player.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_wm_se_manager.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
    '#include <game/bases/d_save_mng.hpp>\n#include <game/bases/d_wm_lib.hpp>\n#include "d_a_wm_grid.hpp"',
]

base_body = """
ACTOR_PROFILE(WM_GRID, daWmGrid_c, 0);

daWmGrid_c::daWmGrid_c() {}
daWmGrid_c::~daWmGrid_c() {}

int daWmGrid_c::create() {
    return SUCCEEDED;
}

int daWmGrid_c::doDelete() {
    return SUCCEEDED;
}

int daWmGrid_c::execute() {
    return SUCCEEDED;
}

int daWmGrid_c::draw() {
    return SUCCEEDED;
}

bool daWmGrid_c::processCutsceneCommand(int, bool) {
    return false;
}
"""

for i, h in enumerate(headers_to_try):
    first_line = h.splitlines()[0]
    print(f"\n[{i+1}/{len(headers_to_try)}] Testing header: {first_line}")
    test_grid_cpp(h + base_body)
