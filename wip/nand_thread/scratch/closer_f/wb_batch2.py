import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
OBJ = os.path.join(HERE, 'wb_try.o')
TXT = os.path.join(HERE, 'wb_try.txt')
TARGET = os.path.join(ROOT, 'wip', 'nand_thread', 'target_raw.txt')
NAME = "writeBanner__13dNandThread_cFP12NANDFileInfo"

HEADER = """#include <types.h>
#include <string.h>
#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_message.hpp>
#include <game/bases/d_res_mng.hpp>

"""

def build_G():
    # u16-typed mask/val/shift locals, computed before the *first* memcpy,
    # kept live across both memcpy calls.
    return HEADER + """
s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    u16 mask = 3, val = 2, shift = 0;
    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    a_banner.iconSpeed = (a_banner.iconSpeed & ~(mask << shift)) | (val << shift);
    a_banner.iconSpeed &= ~(3 << 2);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
"""

def build_H():
    # mask/val/shift declared and computed right before the second getRes
    # call (matching the interleaving seen in the target disassembly),
    # rather than at the top of the function.
    return HEADER + """
s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    int shift = 0;
    int mask = 3;
    int val = 2;
    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    a_banner.iconSpeed = (a_banner.iconSpeed & ~(mask << shift)) | (val << shift);
    a_banner.iconSpeed &= ~(3 << 2);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
"""

def build_I():
    # address-taken frame -- forces it through memory, defeating
    # register-level constant propagation without volatile.
    return HEADER + """
s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    int shift = 0;
    int *pShift = &shift;
    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << *pShift)) | (2 << *pShift);
    a_banner.iconSpeed &= ~(3 << 2);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
"""

VARIANTS = {
    'G_u16_typed_locals': build_G(),
    'H_interleaved_decl': build_H(),
    'I_address_taken_shift': build_I(),
}

def run_variant(label, src):
    srcpath = os.path.join(HERE, 'wb_try.cpp')
    with open(srcpath, 'w') as f:
        f.write(src)
    ok, log = harness.compile_draft(srcpath, OBJ)
    if not ok:
        print(label, "COMPILE FAIL")
        print(log[:800])
        return
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print(label, "DISASM FAIL", dlog[:300])
        return
    matched, report = harness.diff_fn(TARGET, TXT, NAME)
    got = harness.extract(TXT, NAME)
    if matched:
        print(label, "-> MATCH!!! instr=%d" % len(got))
    else:
        lines = report.splitlines()
        print(label, "-> instr=%d  %s" % (len(got), lines[0] if lines else '?'))

if __name__ == '__main__':
    for label, src in VARIANTS.items():
        run_variant(label, src)
