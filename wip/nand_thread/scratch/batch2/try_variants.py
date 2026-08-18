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
"A_reordered_decl": """
bool dNandThread_c::spaceCheck() {
    s32 err;
    u32 answer = 0xFFFFFFFF;
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
"B_extra_local": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    int pad = 0;
    s32 err = NANDCheck(3, 2, &answer);
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
    (void)pad;
}
""",
"C_flags_alias": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            u32 flags = answer;
            if (flags & 5) {
                mError = 7;
            } else if (flags & 0xa) {
                mError = 8;
            }
        }
    }
}
""",
"D_combined_no_nest_inner": """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0 && err == 0) {
        if (answer & 5) {
            mError = 7;
        } else if (answer & 0xa) {
            mError = 8;
        }
    }
}
""",
"E_this_answer_swap_order": """
bool dNandThread_c::spaceCheck() {
    s32 err;
    u32 answer;
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
}

for label, body in BODIES.items():
    src = HEADER + body
    srcpath = os.path.join(WORK, "d_nand_thread.cpp")
    # write to a differently-named file to preserve correct anon-namespace mangling isn't
    # needed here (spaceCheck doesn't touch anon-namespace strings), so filename doesn't matter for this fn.
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
