import os
import subprocess
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'scratch', 'gemini_round13'))
from test_grid import test_grid_cpp

# Test A: extern const float DUMMY_ORDERING = 0.0f;
codeA = """
extern const float DUMMY_ORDERING = 0.0f;
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

# Test B: struct with static const float
codeB = """
struct WmGridData_t {
    static const float s_dummy;
};
const float WmGridData_t::s_dummy = 0.0f;

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

print("=== Test A ===")
test_grid_cpp(codeA)

print("=== Test B ===")
test_grid_cpp(codeB)
