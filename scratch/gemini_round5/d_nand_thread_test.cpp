#include <game/bases/d_nand_thread.hpp>
#include <cstddef>
#include <cstring>

namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
}

dNandThread_c *dNandThread_c::m_instance = nullptr;

dNandThread_c::dNandThread_c(int msgCount, EGG::Heap *heap)
    : EGG::Thread(0x4000, 0, msgCount, heap) {
}

dNandThread_c::~dNandThread_c() {}

void *dNandThread_c::run() { return nullptr; }

void dNandThread_c::cmdExistCheck() {}
bool dNandThread_c::existCheck() { return false; }

void dNandThread_c::cmdSpaceCheck() {}
bool dNandThread_c::spaceCheck() { return false; }

bool dNandThread_c::cmdSave(const void *saveData) { return false; }
bool dNandThread_c::save() { return false; }

bool dNandThread_c::createBanner() { return false; }
bool dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char *c_icon_res = "save_icon.bti";
    const char *smnp = "SMNP";
    return false;
}

void dNandThread_c::cmdLoad() {}
bool dNandThread_c::load() { return false; }

bool dNandThread_c::checkCRC() { return false; }

void dNandThread_c::cmdDeleteFile() {}
bool dNandThread_c::deleteFile() { return false; }

void dNandThread_c::setNandError(s32 err) {}
void *dNandThread_c::getSaveData() { return nullptr; }

void dNandThread_c::create(EGG::Heap *heap) {}
