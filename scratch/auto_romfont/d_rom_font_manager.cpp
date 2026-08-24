#include <game/bases/d_rom_font_manager.hpp>

dRomFontMgr_c *dRomFontMgr_c::m_pInstance;

void dRomFontMgr_c::createInstance(EGG::Heap *heap) {
    dRomFontMgr_c *inst = new (heap, 4) dRomFontMgr_c();
    m_pInstance = inst;
    inst->load_resource(heap);
}

void dRomFontMgr_c::load_resource(EGG::Heap *heap) {
    u32 size = mFont.GetRequireBufferSize();
    void *buf = heap->alloc(size, 0x20);
    mFont.Load(buf);
    mBuffer = buf;
    TextWriter::sDefaultFont = this;
    mLoaded = true;
}
