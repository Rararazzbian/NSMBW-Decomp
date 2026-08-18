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

TEMPLATE = """
bool dNandThread_c::spaceCheck() {{
{decl_before}
    u32 answer = 0xFFFFFFFF;
{decl_after_answer}
    s32 err = NANDCheck(3, 2, &answer);
{decl_after_call}
    setNandError(err);
{decl_after_set}
    if (mError == 0) {{
        if (err == 0) {{
{decl_in_inner}
            if (answer & 5) {{
                mError = 7;
            }} else if (answer & 0xa) {{
                mError = 8;
            }}
        }}
    }}
}}
"""

VARIANTS = {
    "before_int": dict(decl_before="    int dummy = 0;", decl_after_answer="", decl_after_call="", decl_after_set="", decl_in_inner=""),
    "after_answer_int": dict(decl_before="", decl_after_answer="    int dummy = 0;", decl_after_call="", decl_after_set="", decl_in_inner=""),
    "after_call_int": dict(decl_before="", decl_after_answer="", decl_after_call="    int dummy = 0;", decl_after_set="", decl_in_inner=""),
    "after_set_int": dict(decl_before="", decl_after_answer="", decl_after_call="", decl_after_set="    int dummy = 0;", decl_in_inner=""),
    "inner_int": dict(decl_before="", decl_after_answer="", decl_after_call="", decl_after_set="", decl_in_inner="            int dummy = 0;"),
    "before_u32": dict(decl_before="    u32 dummy = 0;", decl_after_answer="", decl_after_call="", decl_after_set="", decl_in_inner=""),
    "inner_u32": dict(decl_before="", decl_after_answer="", decl_after_call="", decl_after_set="", decl_in_inner="            u32 dummy = 0;"),
    "after_set_u32": dict(decl_before="", decl_after_answer="", decl_after_call="", decl_after_set="    u32 dummy = 0;", decl_in_inner=""),
}

for label, kw in VARIANTS.items():
    body = TEMPLATE.format(**kw)
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
