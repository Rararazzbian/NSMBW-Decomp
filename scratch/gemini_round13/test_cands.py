import os
import subprocess
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'scratch', 'gemini_round13'))
from test_grid import test_grid_cpp

cand1 = """#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_grid.hpp"

static const float s_dummy = 0.0f;

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

cand2 = """#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_grid.hpp"

void DUMMY_FLOAT() {
    static const float UNUSED = 0.0f;
}

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

cand3 = """
void DUMMY_UNUSED() {
    static const float UNUSED = 0.0f;
}

#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_grid.hpp"

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

for name, code in [('cand1', cand1), ('cand2', cand2), ('cand3', cand3)]:
    print(f"\nTesting {name}:")
    test_grid_cpp(code)
