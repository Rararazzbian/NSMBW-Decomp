
#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>

extern void use(const void*);

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
};

void dNandThread_c::writeBanner(NANDFileInfo* fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char* c_icon_res = "save_icon.bti";
    
    use(sc_TEMP_BANNER_FILE);
    use(sc_BANNER_FILE);
    use(sc_GAME_FILE);
    use(l_safeCopyBuf);
    use(l_tmpSave);
    use(a_banner);
    use(c_icon_res);
    use("SMNP");
    use("save_banner_EU.bti");
    use("save_banner");
}
