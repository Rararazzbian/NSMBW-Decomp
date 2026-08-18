import os
import subprocess
import sys

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))

scratch_dir = os.path.join(ROOT, 'scratch', 'codex_round14')

# Header for daWmTower_c
tower_hpp = """#pragma once
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_demo_actor.hpp>

class daWmTower_c : public dWmDemoActor_c {
public:
    daWmTower_c();
    virtual ~daWmTower_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    void createModel();
    void calcModel();

    int mResNodeIdx;
    dHeapAllocator_c mAllocator;
    m3d::smdl_c mModel;
};
"""

with open(os.path.join(scratch_dir, 'd_a_wm_tower.hpp'), 'w') as f:
    f.write(tower_hpp)

# Source for daWmTower_c
tower_cpp = """#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>
#include "d_a_wm_tower.hpp"

ACTOR_PROFILE(WM_TOWER, daWmTower_c, 0);

daWmTower_c::daWmTower_c() : mResNodeIdx(-1) {}
daWmTower_c::~daWmTower_c() {}

int daWmTower_c::create() {
    createModel();
    calcModel();

    mClipSphere.set(mPos, 120.0f);

    return SUCCEEDED;
}

int daWmTower_c::execute() {
    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);

    daWmMap_c::m_instance->GetNodePos(mResNodeIdx, mPos);
    calcModel();

    return SUCCEEDED;
}

int daWmTower_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmTower_c::doDelete() {
    return SUCCEEDED;
}

void daWmTower_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobTower", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobTower");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);
    dWmActor_c::setSoftLight_MapObj(mModel);

    mAllocator.adjustFrmHeap();
}

void daWmTower_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}
"""

with open(os.path.join(scratch_dir, 'd_a_wm_tower.cpp'), 'w') as f:
    f.write(tower_cpp)

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
    os.path.join(scratch_dir, 'd_a_wm_tower.cpp'),
    '-o', os.path.join(scratch_dir, 'd_a_wm_tower.o')
]

res = subprocess.run(cmd, capture_output=True, text=True)
if res.returncode == 0:
    out_txt = os.path.join(scratch_dir, 'd_a_wm_tower_compiled.txt')
    subprocess.run([dtk, 'elf', 'disasm', os.path.join(scratch_dir, 'd_a_wm_tower.o'), out_txt], check=True)
    print("Compiled and disassembled daWmTower_c successfully!")
else:
    print("Compilation error:\n", res.stderr)
