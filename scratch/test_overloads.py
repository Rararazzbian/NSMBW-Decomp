import os
import sys

sys.path.insert(0, os.path.abspath('.'))
from tools.auto_decomp.harness import compile_draft, disasm

test_code = """
#include <types.h>

class SndAudioMgr {
public:
    void startSystemSe(unsigned int soundID, unsigned long);
    void startSystemSe(unsigned long soundID, unsigned long);

    static SndAudioMgr *sInstance;
};

void test_calls() {
    // Test 2: unsigned int
    SndAudioMgr::sInstance->startSystemSe((unsigned int)0x252, 1);
    // Test 3: u32
    u32 id_u32 = 0x78;
    SndAudioMgr::sInstance->startSystemSe(id_u32, 1);
    // Test 4: unsigned long / ulong
    ulong id_ulong = 0x100;
    SndAudioMgr::sInstance->startSystemSe(id_ulong, 1);
}
"""

with open('scratch/test_draft.cpp', 'w') as f:
    f.write(test_code)

ok, msg = compile_draft('scratch/test_draft.cpp', 'scratch/test_draft.o')
print("Compile OK:", ok)
print("Compiler output:", msg)

if ok:
    disasm('scratch/test_draft.o', 'scratch/test_draft.txt')
    with open('scratch/test_draft.txt') as f:
        print(f.read())
