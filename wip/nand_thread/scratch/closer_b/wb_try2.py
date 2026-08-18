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

COMMON_TOP = """
s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

"""

COMMON_BOT = """
    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
"""

VARIANTS = {
"volatile_frame": '    volatile int frame = 0;\n    a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << (frame * 2))) | (2 << (frame * 2));\n',
"volatile_mask_val": '    volatile int mask = 3, val = 2;\n    a_banner.iconSpeed = (a_banner.iconSpeed & ~mask) | val;\n',
"static_nonconst_frame": '    static int s_frame = 0;\n    a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << (s_frame * 2))) | (2 << (s_frame * 2));\n',
"extern_frame_ptr": '    extern int g_iconFrame;\n    a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << (g_iconFrame * 2))) | (2 << (g_iconFrame * 2));\n',
"deref_param_frame": '    int frame = fileInfo->uniqueNo;\n    frame = 0;\n    a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << (frame * 2))) | (2 << (frame * 2));\n',
}

for label, body in VARIANTS.items():
    src = HEADER + COMMON_TOP + body + COMMON_BOT
    srcpath = os.path.join(HERE, 'wb_try.cpp')
    with open(srcpath, 'w') as f:
        f.write(src)
    ok, log = harness.compile_draft(srcpath, OBJ)
    if not ok:
        print(label, "COMPILE FAIL")
        print(log[:600])
        continue
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print(label, "DISASM FAIL", dlog[:300])
        continue
    matched, report = harness.diff_fn(TARGET, TXT, NAME)
    if matched:
        print(label, "-> MATCH!!!")
    else:
        lines = report.splitlines()
        print(label, "->", lines[0])
