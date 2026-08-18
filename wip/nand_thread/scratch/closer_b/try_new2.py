import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

WORK = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_b")
OBJ = os.path.join(WORK, "try.o")
TXT = os.path.join(WORK, "try.txt")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")
NAME = "spaceCheck__13dNandThread_cFv"

HEADER = '#include <game/bases/d_nand_thread.hpp>\n\n'

BODIES = {
"V_inline_helper_first_param": """
namespace {
inline bool hasFlags(u32 v, u32 mask) { return (v & mask) != 0; }
}

bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (hasFlags(answer, 5)) {
                mError = 7;
            } else if (hasFlags(answer, 0xa)) {
                mError = 8;
            }
        }
    }
}
""",
"W_answer_volatile": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, (u32*)&answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            register u32 a = answer;
            if (a & 5) {
                mError = 7;
            } else if (a & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"X_answer_first_decl_before_err_no_init": """
bool dNandThread_c::spaceCheck() {
    u32 answer;
    s32 err;
    answer = 0xFFFFFFFF;
    err = NANDCheck(3, 2, &answer);
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
"Y_answer_via_deref_local_ptr_no_intermediate": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            switch (0) {
            default:
                if (answer & 5) {
                    mError = 7;
                    break;
                }
                if (answer & 0xa) {
                    mError = 8;
                }
            }
        }
    }
}
""",
"Z_pad_before_answer_used": """
bool dNandThread_c::spaceCheck() {
    s32 pad = 0;
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err + pad);
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
}

for label, body in BODIES.items():
    src = HEADER + body
    srcpath = os.path.join(WORK, "tryfile.cpp")
    with open(srcpath, "w") as f:
        f.write(src)
    ok, log = harness.compile_draft(srcpath, OBJ)
    if not ok:
        print(label, "COMPILE FAIL")
        print(log[:800])
        continue
    dok, dlog = harness.disasm(OBJ, TXT)
    if not dok:
        print(label, "DISASM FAIL")
        continue
    matched, report = harness.diff_fn(TARGET, TXT, NAME)
    if matched:
        print(label, "-> MATCH!!!")
    else:
        lines = report.splitlines()
        print(label, "->", lines[0])
        for l in lines[1:6]:
            print("   ", l)
