import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, ROOT)
from tools.auto_decomp.harness import compile_draft, disasm
from tools.auto_decomp.fndiff import read_functions, canonical_name, compare

WORK_DIR = os.path.dirname(os.path.abspath(__file__))
TARGET_TXT = os.path.join(WORK_DIR, 'target.txt')
DRAFT_CPP = os.path.join(WORK_DIR, 'd_p_sw_manager.cpp')
DRAFT_OBJ = os.path.join(WORK_DIR, 'd_p_sw_manager.o')
DRAFT_TXT = os.path.join(WORK_DIR, 'd_p_sw_manager.txt')
EXTRA_INC = [os.path.join(WORK_DIR, 'include')]

base_template = """#include <game/bases/d_p_sw_manager.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/snd/snd_audio_mgr.hpp>
#include <game/snd/snd_scene_manager.hpp>

dPSwManager_c *dPSwManager_c::ms_instance;

dPSwManager_c::dPSwManager_c() {
    ms_instance = this;
}

dPSwManager_c::~dPSwManager_c() {
    ms_instance = NULL;
}

__INIT__

void dPSwManager_c::execute() {
    ProcMain();
}

void dPSwManager_c::ProcMain() {
    for (int i = 0; i < 3; i++) {
        if (checkSwitch((SwType_e)i)) {
            int timer = getTimer((SwType_e)i);
            if (timer != 0) {
                timer--;
            }
            if ((u32)timer % 60 == 0) {
                if ((u32)timer > 180) {
                    SndAudioMgr::sInstance->startSystemSe(0xAAu, 1ul);
                } else if (timer != 0) {
                    SndAudioMgr::sInstance->startSystemSe(0xABu, 1ul);
                }
            }
            if (timer == 0) {
                SndSceneMgr::sInstance->fn_8019be60(8);
                offSwitch((SwType_e)i);
            }
            setTimer((SwType_e)i, timer);
        }
    }
}

__FIN__

u32 dPSwManager_c::checkSwitch(SwType_e type) {
    return mSwitchFlags & (1 << type);
}

bool dPSwManager_c::checkMove() {
    return mSwitchFlags != 0;
}

int dPSwManager_c::getTimer(SwType_e type) {
    return mTimer[type];
}

void dPSwManager_c::onSwitch(SwType_e type, int timer) {
    mSwitchFlags |= (1 << type);
    setTimer(type, timer);
}

void dPSwManager_c::offSwitch(SwType_e type) {
    mSwitchFlags &= ~(1 << type);
}

void dPSwManager_c::setTimer(SwType_e type, int timer) {
    mTimer[type] = timer;
}
"""

tgt = read_functions(TARGET_TXT)
tgt_init = tgt[canonical_name('initialize__13dPSwManager_cFv')]
tgt_fin = tgt[canonical_name('finalize__13dPSwManager_cFv')]

init_variants = [
    # 1: direct
    ("v1_direct", """void dPSwManager_c::initialize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    mSwitchFlags = bgParam->mPSwitchFlags;
    mTimer[0] = bgParam->mPSwitchTimer[0];
    mTimer[1] = bgParam->mPSwitchTimer[1];
    mTimer[2] = bgParam->mPSwitchTimer[2];
}"""),
    # 2: getInstance
    ("v2_getInstance", """void dPSwManager_c::initialize() {
    dBgParameter_c *bgParam = dBgParameter_c::getInstance();
    mSwitchFlags = bgParam->mPSwitchFlags;
    mTimer[0] = bgParam->mPSwitchTimer[0];
    mTimer[1] = bgParam->mPSwitchTimer[1];
    mTimer[2] = bgParam->mPSwitchTimer[2];
}"""),
    # 3: struct copy / carrier
    ("v3_array_loop", """void dPSwManager_c::initialize() {
    dBgParameter_c *bg = dBgParameter_c::ms_Instance_p;
    mSwitchFlags = bg->mPSwitchFlags;
    for (int i = 0; i < 3; i++) mTimer[i] = bg->mPSwitchTimer[i];
}"""),
    # 4: 2 temps
    ("v4_two_temps", """void dPSwManager_c::initialize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    mSwitchFlags = bgParam->mPSwitchFlags;
    int t0 = bgParam->mPSwitchTimer[0];
    int t1 = bgParam->mPSwitchTimer[1];
    mTimer[1] = t1;
    mTimer[0] = t0;
    mTimer[2] = bgParam->mPSwitchTimer[2];
}"""),
    # 5: 3 temps
    ("v5_three_temps", """void dPSwManager_c::initialize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    mSwitchFlags = bgParam->mPSwitchFlags;
    int t0 = bgParam->mPSwitchTimer[0];
    int t1 = bgParam->mPSwitchTimer[1];
    int t2 = bgParam->mPSwitchTimer[2];
    mTimer[0] = t0;
    mTimer[1] = t1;
    mTimer[2] = t2;
}"""),
    # 6: pointers
    ("v6_pointers", """void dPSwManager_c::initialize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    mSwitchFlags = bgParam->mPSwitchFlags;
    const int *src = bgParam->mPSwitchTimer;
    int *dst = mTimer;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}"""),
]

fin_variants = [
    # 1: direct
    ("f1_direct", """void dPSwManager_c::finalize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    bgParam->mPSwitchFlags = mSwitchFlags;
    bgParam->mPSwitchTimer[0] = mTimer[0];
    bgParam->mPSwitchTimer[1] = mTimer[1];
    bgParam->mPSwitchTimer[2] = mTimer[2];
}"""),
    # 2: getInstance
    ("f2_getInstance", """void dPSwManager_c::finalize() {
    dBgParameter_c *bgParam = dBgParameter_c::getInstance();
    bgParam->mPSwitchFlags = mSwitchFlags;
    bgParam->mPSwitchTimer[0] = mTimer[0];
    bgParam->mPSwitchTimer[1] = mTimer[1];
    bgParam->mPSwitchTimer[2] = mTimer[2];
}"""),
    # 3: temps
    ("f3_temps", """void dPSwManager_c::finalize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    bgParam->mPSwitchFlags = mSwitchFlags;
    int t0 = mTimer[0];
    int t1 = mTimer[1];
    bgParam->mPSwitchTimer[0] = t0;
    bgParam->mPSwitchTimer[1] = t1;
    bgParam->mPSwitchTimer[2] = mTimer[2];
}"""),
    # 4: 3 temps
    ("f4_three_temps", """void dPSwManager_c::finalize() {
    dBgParameter_c *bgParam = dBgParameter_c::ms_Instance_p;
    bgParam->mPSwitchFlags = mSwitchFlags;
    int t0 = mTimer[0];
    int t1 = mTimer[1];
    int t2 = mTimer[2];
    bgParam->mPSwitchTimer[0] = t0;
    bgParam->mPSwitchTimer[1] = t1;
    bgParam->mPSwitchTimer[2] = t2;
}"""),
]

print("Testing init variants...")
for label, code in init_variants:
    src = base_template.replace('__INIT__', code).replace('__FIN__', fin_variants[0][1])
    with open(DRAFT_CPP, 'w') as f:
        f.write(src)
    ok, log = compile_draft(DRAFT_CPP, DRAFT_OBJ, extra_inc=EXTRA_INC, module='wiimj2d')
    if ok and disasm(DRAFT_OBJ, DRAFT_TXT)[0]:
        drf = read_functions(DRAFT_TXT)
        d_init = drf.get(canonical_name('initialize__13dPSwManager_cFv'), [])
        diff = compare(tgt_init, d_init, 'initialize', verbose=False)
        print(f"  {label:<20}: diffs={diff}")

print("\nTesting fin variants...")
for label, code in fin_variants:
    src = base_template.replace('__INIT__', init_variants[0][1]).replace('__FIN__', code)
    with open(DRAFT_CPP, 'w') as f:
        f.write(src)
    ok, log = compile_draft(DRAFT_CPP, DRAFT_OBJ, extra_inc=EXTRA_INC, module='wiimj2d')
    if ok and disasm(DRAFT_OBJ, DRAFT_TXT)[0]:
        drf = read_functions(DRAFT_TXT)
        d_fin = drf.get(canonical_name('finalize__13dPSwManager_cFv'), [])
        diff = compare(tgt_fin, d_fin, 'finalize', verbose=False)
        print(f"  {label:<20}: diffs={diff}")
