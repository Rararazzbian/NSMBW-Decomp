#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>

namespace {
    u8 l_safeCopyBuf[0x4000];
    u8 l_tmpSave[0x3fa0];
}

dNandThread_c *dNandThread_c::m_instance;

dNandThread_c::dNandThread_c(int msgCount, EGG::Heap *heap)
    : EGG::Thread(0x4000, 0, msgCount, heap) {
    mState = 0;
    m_instance = this;
}

dNandThread_c::~dNandThread_c() {
    m_instance = nullptr;
}

void dNandThread_c::create(EGG::Heap *heap) {
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(heap);
    dNandThread_c *thread = new dNandThread_c(OSGetThreadPriority(OSGetCurrentThread()) - 1, nullptr);
    mHeap::setCurrentHeap(prevHeap);
    OSResumeThread(*(OSThread **)((u8 *)thread + 0x8));
}

void *dNandThread_c::run() {
    return 0;
}
