
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
