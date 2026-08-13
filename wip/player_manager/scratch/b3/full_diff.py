import sys, os
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

TXT = os.path.join(ROOT, "wip", "player_manager", "scratch", "b3", "draft.txt")
TARGET = os.path.join(ROOT, "wip", "player_manager", "target_text.txt")

want = harness.extract(TARGET, "update__9daPyMng_cFv")
got = harness.extract(TXT, "update__9daPyMng_cFv")
print("target len", len(want), "draft len", len(got))
for i in range(max(len(want), len(got))):
    a = want[i] if i < len(want) else '<none>'
    b = got[i] if i < len(got) else '<none>'
    mark = "  " if a == b else "!!"
    print("%s %3d | want: %-46s got: %s" % (mark, i, a, b))
