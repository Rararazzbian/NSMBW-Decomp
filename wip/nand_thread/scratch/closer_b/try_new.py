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
"P_enum_flags": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & (NAND_CHECK_TOO_MANY_APP_BLOCKS | NAND_CHECK_TOO_MANY_USER_BLOCKS)) {
                mError = 7;
            } else if (answer & (NAND_CHECK_TOO_MANY_APP_FILES | NAND_CHECK_TOO_MANY_USER_FILES)) {
                mError = 8;
            }
        }
    }
}
""",
"Q_err_reused_as_scratch": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    err = 0 != err;
    setNandError(err);
    if (mError == 0) {
        if (!err) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"R_answer_addr_reused_ptr": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    u32 *pAnswer = &answer;
    s32 err = NANDCheck(3, 2, pAnswer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (*pAnswer & 5) {
                mError = 7;
            } else if (*pAnswer & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"S_mError_local_first": """
bool dNandThread_c::spaceCheck() {
    s32 noErr;
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    noErr = mError;
    if (noErr == 0) {
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
"T_answer_after_err_check_decl": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        u32 a2 = answer;
        if (err == 0) {
            if (a2 & 5) {
                mError = 7;
            } else if (a2 & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"U_this_call_explicit": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    this->setNandError(err);
    if (this->mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                this->mError = 7;
            } else if (answer & 0xa) {
                this->mError = 8;
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
        for l in lines[1:]:
            print("   ", l)
