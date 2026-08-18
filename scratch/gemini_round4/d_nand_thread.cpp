
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
}

class dNandThread_c {
public:
    void writeBanner(NANDFileInfo* fileInfo);
    void save();
};

void dNandThread_c::writeBanner(NANDFileInfo* fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char* c_icon_res = "save_icon.bti";
    const char* tag = "SMNP";
    
    // Od-use all variables
    fileInfo->stageBuf = (void*)sc_TEMP_BANNER_FILE;
    fileInfo->stageBuf = (void*)sc_BANNER_FILE;
    fileInfo->stageBuf = (void*)sc_GAME_FILE;
    fileInfo->stageBuf = (void*)l_safeCopyBuf;
    fileInfo->stageBuf = (void*)l_tmpSave;
    fileInfo->stageBuf = (void*)a_banner;
    fileInfo->stageBuf = (void*)c_icon_res;
    fileInfo->stageBuf = (void*)tag;
}
