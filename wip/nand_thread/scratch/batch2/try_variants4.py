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
"K_ternary": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            mError = (answer & 5) ? 7 : (answer & 0xa) ? 8 : mError;
        }
    }
}
""",
"L_s16_barrier": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            s16 bits = (s16)answer;
            if (bits & 5) {
                mError = 7;
            } else if (bits & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"M_u8_barrier": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            u8 bits = (u8)answer;
            if (bits & 5) {
                mError = 7;
            } else if (bits & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"N_answer_s32_not_u32": """
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
"O_reversed_err_check_first_answer": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0 && err == 0 && (answer & 5)) {
        mError = 7;
    } else if (mError == 0 && err == 0 && (answer & 0xa)) {
        mError = 8;
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
        print(label, "-> DIFF (%d lines, target=%d got=%d)" % (n, len(want), len(got)))
