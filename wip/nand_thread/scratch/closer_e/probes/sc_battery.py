import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

BASE = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e")
TARGET = os.path.join(ROOT, "wip", "nand_thread", "target_raw.txt")
FN = "spaceCheck__13dNandThread_cFv"
SHADOW = os.path.join(BASE, "shadow_include")
NANDCHECK_H = os.path.join(SHADOW, "revolution", "NAND", "NANDCheck.h")
HPP = os.path.join(SHADOW, "game", "bases", "d_nand_thread.hpp")

ORIG_NANDCHECK = open(NANDCHECK_H, encoding="utf-8").read()
ORIG_HPP = open(HPP, encoding="utf-8").read()

NANDCHECK_TEMPLATE = """#ifndef RVL_SDK_NAND_CHECK_H
#define RVL_SDK_NAND_CHECK_H
#include <revolution/NAND/nand.h>
#include <types.h>
#ifdef __cplusplus
extern "C" {{
#endif

typedef enum {{
    NAND_CHECK_TOO_MANY_APP_BLOCKS = (1 << 0),
    NAND_CHECK_TOO_MANY_APP_FILES = (1 << 1),
    NAND_CHECK_TOO_MANY_USER_BLOCKS = (1 << 2),
    NAND_CHECK_TOO_MANY_USER_FILES = (1 << 3),
}} NANDCheckFlags;

{decl}

#ifdef __cplusplus
}}
#endif
#endif
"""

VARIANTS = []

# name, nandcheck_decl, source_body, hpp_text(None=unchanged)
VARIANTS.append((
    "sig_int_params",
    "s32 NANDCheck(int neededBlocks, int neededFiles, u32* answer);",
    None))

VARIANTS.append((
    "sig_long_return",
    "long NANDCheck(u32 neededBlocks, u32 neededFiles, u32* answer);",
    None))

VARIANTS.append((
    "sig_void_return_adjusted",
    "void NANDCheck(u32 neededBlocks, u32 neededFiles, u32* answer);",
    """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    NANDCheck(3, 2, &answer);
    setNandError(0);
    if (mError == 0) {
        if (answer & 5) {
            mError = 7;
        } else if (answer & 0xa) {
            mError = 8;
        }
    }
}
"""))

VARIANTS.append((
    "answer_array1",
    "s32 NANDCheck(u32 neededBlocks, u32 neededFiles, u32* answer);",
    """
bool dNandThread_c::spaceCheck() {
    u32 answer[1] = {0xFFFFFFFF};
    s32 err = NANDCheck(3, 2, answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer[0] & 5) {
                mError = 7;
            } else if (answer[0] & 0xa) {
                mError = 8;
            }
        }
    }
}
"""))

VARIANTS.append((
    "answer_struct_byaddr",
    """typedef struct { u32 v; } NANDCheckAnswer;
s32 NANDCheck(u32 neededBlocks, u32 neededFiles, NANDCheckAnswer* answer);""",
    """
bool dNandThread_c::spaceCheck() {
    NANDCheckAnswer answer = {0xFFFFFFFF};
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer.v & 5) {
                mError = 7;
            } else if (answer.v & 0xa) {
                mError = 8;
            }
        }
    }
}
"""))

VARIANTS.append((
    "extra_unused_param",
    "s32 NANDCheck(u32 neededBlocks, u32 neededFiles, u32* answer, u32 reserved);",
    """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer, 0);
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
"""))

VARIANTS.append((
    "params_reordered",
    "s32 NANDCheck(u32* answer, u32 neededBlocks, u32 neededFiles);",
    """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(&answer, 3, 2);
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
"""))

DEFAULT_BODY = """
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
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
}
"""

# setNandError return type variants (needs hpp shadow change)
SETNANDERROR_BOOL_HPP = ORIG_HPP.replace(
    "void setNandError(long err);", "bool setNandError(long err);")
SETNANDERROR_BOOL_CPP_HEADER = """
#include <game/bases/d_nand_thread.hpp>
bool dNandThread_c::setNandError(long err) {
    switch (err) {
    default:
        mError = (err != 0);
        break;
    }
    return mError != 0;
}
"""


def write(path, text):
    open(path, "w", encoding="utf-8").write(text)


def run(name, src_text, extra_files=None):
    src_path = os.path.join(BASE, "probes", "sc_bat_%s.cpp" % name)
    obj = os.path.join(BASE, "probes", "sc_bat_%s.o" % name)
    txt = os.path.join(BASE, "probes", "sc_bat_%s.txt" % name)
    write(src_path, src_text)
    ok, log = harness.compile_draft(src_path, obj, extra_inc=[SHADOW])
    if not ok:
        print("[%s] COMPILE FAILED" % name)
        print(log[:600])
        return
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print("[%s] DISASM FAILED: %s" % (name, dlog[:300]))
        return
    matched, report = harness.diff_fn(TARGET, txt, FN)
    print("[%s] %s" % (name, "MATCH!!!" if matched else "diff"))
    if not matched:
        # print just the differing lines, compact
        for line in report.splitlines()[:6]:
            print("    " + line)
    print()


if __name__ == "__main__":
    for name, decl, body in VARIANTS:
        write(NANDCHECK_H, NANDCHECK_TEMPLATE.format(decl=decl))
        src = "#include <game/bases/d_nand_thread.hpp>\n" + (body or DEFAULT_BODY)
        run(name, src)
    # restore
    write(NANDCHECK_H, ORIG_NANDCHECK)

    # setNandError return-type variant
    write(HPP, SETNANDERROR_BOOL_HPP)
    run("setnanderror_bool_return", SETNANDERROR_BOOL_CPP_HEADER + "\n" + DEFAULT_BODY)
    write(HPP, ORIG_HPP)
