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

VARIANTS = {}

# A: call the same inline helper TWICE with (0,2) and (1,0) instead of
#    hand-writing field 1's clear as a raw expression.
VARIANTS["A_two_calls_same_inline"] = (
"""namespace {
inline void setIconSpeed(NANDBanner *banner, int frame, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(3 << (frame * 2))) | (speed << (frame * 2));
}
} // namespace
""",
"""    setIconSpeed(&a_banner, 0, 2);
    setIconSpeed(&a_banner, 1, 0);
""")

# B: read mask/val/shift from const arrays indexed by frame (literal 0),
#    instead of writing "3 << (frame*2)" directly.
VARIANTS["B_const_array_lookup"] = (
"""namespace {
const u8 c_masks[2]  = {3, 3};
const u8 c_speeds[2] = {2, 0};
const u8 c_shifts[2] = {0, 2};
} // namespace
""",
"""    int frame = 0;
    a_banner.iconSpeed = (a_banner.iconSpeed & ~(c_masks[frame] << c_shifts[frame])) | (c_speeds[frame] << c_shifts[frame]);
    a_banner.iconSpeed &= ~(c_masks[1] << c_shifts[1]);
""")

# C: same as B but only field0 uses array lookup, field1 is a folded literal
VARIANTS["C_const_array_field0_only"] = (
"""namespace {
const u8 c_mask0 = 3;
const u8 c_speed0 = 2;
const u8 c_shift0 = 0;
} // namespace
""",
"""    a_banner.iconSpeed = (a_banner.iconSpeed & ~(c_mask0 << c_shift0)) | (c_speed0 << c_shift0);
    a_banner.iconSpeed &= ~(3 << 2);
""")

# D: mask/val/shift computed via a small loop over 2 iterations with array
#    reads, loop NOT unrolled by hand (let the compiler decide)
VARIANTS["D_for_loop_over_frames"] = (
"""namespace {
const u8 c_speeds[2] = {2, 0};
} // namespace
""",
"""    for (int frame = 0; frame < 2; frame++) {
        a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << (frame * 2))) | (c_speeds[frame] << (frame * 2));
    }
""")

# E: pass frame/speed via a function pointer call (defeats inlining
#    specialization while still requiring straight-line code -- likely will
#    add an extra bl, included as a negative control)
VARIANTS["E_two_calls_reordered"] = (
"""namespace {
inline void setIconSpeed(NANDBanner *banner, int frame, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(3 << (frame * 2))) | (speed << (frame * 2));
}
} // namespace
""",
"""    setIconSpeed(&a_banner, 1, 0);
    setIconSpeed(&a_banner, 0, 2);
""")

# F: field0/field1 both via the SAME inline helper, but sharing one call
#    that loops internally (helper takes an array)
VARIANTS["F_helper_shift_first_param"] = (
"""namespace {
inline void setIconSpeed(NANDBanner *banner, int shift, int mask, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(mask << shift)) | (speed << shift);
}
} // namespace
""",
"""    setIconSpeed(&a_banner, 0, 3, 2);
    a_banner.iconSpeed &= ~(3 << 2);
""")

def run_variant(label, decl, body):
    src = HEADER + decl + COMMON_TOP + body + COMMON_BOT
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

for label, (decl, body) in VARIANTS.items():
    run_variant(label, decl, body)
