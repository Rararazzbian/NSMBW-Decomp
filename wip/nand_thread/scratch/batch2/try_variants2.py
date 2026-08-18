import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

WORK = os.path.join(ROOT, "wip", "nand_thread", "scratch", "batch2")
OBJ = os.path.join(WORK, "try.o")
TXT = os.path.join(WORK, "try.txt")
SHADOW = os.path.join(WORK, "shadow")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")
NAME = "spaceCheck__13dNandThread_cFv"

HEADER = '#include <game/bases/d_nand_thread.hpp>\n\n'

BODIES = {
"F_early_return_style_no_nest": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError != 0) {
    } else if (err != 0) {
    } else if (answer & 5) {
        mError = 7;
    } else if (answer & 0xa) {
        mError = 8;
    }
}
""",
"G_s32_answer_cast": """
bool dNandThread_c::spaceCheck() {
    s32 answer = -1;
    s32 err = NANDCheck(3, 2, (u32*)&answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"H_notequal_explicit": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if ((answer & 5) != 0) {
                mError = 7;
            } else if ((answer & 0xa) != 0) {
                mError = 8;
            }
        }
    }
}
""",
"I_swap_arg_order": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (0 == mError) {
        if (0 == err) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"J_bool_flag_intermediate": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            bool over = (answer & 5) != 0;
            bool under = (answer & 0xa) != 0;
            if (over) {
                mError = 7;
            } else if (under) {
                mError = 8;
            }
        }
    }
}
""",
}

for label, body in BODIES.items():
    src = HEADER + body
    srcpath = os.path.join(WORK, "tryfile.cpp")
    with open(srcpath, "w") as f:
        f.write(src)
    ok, log = harness.compile_draft(srcpath, OBJ, extra_inc=[SHADOW])
    if not ok:
        print(label, "COMPILE FAIL")
        print(log)
        continue
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print(label, "DISASM FAIL")
        continue
    want = harness.extract(TARGET, NAME)
    got = harness.extract(TXT, NAME)
    if want == got:
        print(label, "-> MATCH")
    else:
        n = sum(1 for i in range(max(len(want), len(got))) if (want[i] if i < len(want) else None) != (got[i] if i < len(got) else None))
        print(label, "-> DIFF (%d lines)" % n)
