import os
import subprocess
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))

scratch_dir = os.path.join(ROOT, 'scratch', 'codex_round14')

# Write header for daWmGrid_c
grid_hpp = """#pragma once
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_actor.hpp>

class daWmGrid_c : public dWmActor_c {
public:
    daWmGrid_c();
    virtual ~daWmGrid_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual bool processCutsceneCommand(int, bool);

    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
};
"""

with open(os.path.join(scratch_dir, 'd_a_wm_grid.hpp'), 'w') as f:
    f.write(grid_hpp)

# Write cpp for daWmGrid_c
grid_cpp = """#include <game/bases/d_wm_lib.hpp>
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

with open(os.path.join(scratch_dir, 'd_a_wm_grid.cpp'), 'w') as f:
    f.write(grid_cpp)

mwcc = os.path.join(ROOT, 'compilers', 'Wii', '1.1', 'mwcceppc.exe')
dtk = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')

cmd = [
    mwcc, '-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto',
    '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file',
    '-enc', 'SJIS', '-DREVOLUTION', '-I-',
    '-i', scratch_dir,
    '-i', 'include',
    '-i', 'include/lib',
    '-i', 'include/lib/MSL',
    '-i', 'include/lib/MSL/internal',
    '-i', 'include/lib/revolution/BTE/include',
    '-i', 'include/lib/revolution/BTE/stack/include',
    '-i', 'include/lib/revolution/BTE/stack/btm',
    '-i', 'include/lib/revolution/BTE/bta/include',
    '-i', 'include/lib/revolution/BTE/bta/sys',
    '-i', 'include/lib/revolution/BTE/gki/common',
    '-i', 'include/lib/revolution/BTE/gki/platform',
    os.path.join(scratch_dir, 'd_a_wm_grid.cpp'),
    '-o', os.path.join(scratch_dir, 'd_a_wm_grid.o')
]

res = subprocess.run(cmd, capture_output=True, text=True)
if res.returncode == 0:
    out_txt = os.path.join(scratch_dir, 'd_a_wm_grid_compiled.txt')
    subprocess.run([dtk, 'elf', 'disasm', os.path.join(scratch_dir, 'd_a_wm_grid.o'), out_txt], check=True)
    print("Compiled and disassembled successfully!")
else:
    print("Compilation error:\n", res.stderr)
