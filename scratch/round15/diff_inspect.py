import re
from pathlib import Path


def functions(path):
    text = Path(path).read_text(errors="replace")
    out = {}
    for match in re.finditer(r"^\.fn (\S+?), \w+\n(.*?)^\.endfn", text, re.M | re.S):
        body = match.group(2)
        out[match.group(1)] = [x.group(1) for x in re.finditer(r"\*/\s*(.+?)\s*$", body, re.M)]
    return out


def norm(line):
    line = re.sub(r'"?[.A-Za-z_@$][^\s,]*"?@(ha|l|sda21|sda2)\b', r"SYM@\1", line)
    line = re.sub(r"^(bl|b) \S+$", r"\1 SYM", line)
    return re.sub(r"\.L_[0-9A-Fa-f]+", "LBL", line)


target = {}
for path in [
    "scratch/round15/target_163620.txt",
    "scratch/round15/auto_fn_2_164180_text.o.dis.txt",
    "scratch/round15/auto_00_00164204_text.o.dis.txt",
]:
    target.update(functions(path))
draft = functions("scratch/round15/d_a_wm_ghost.txt")

for target_name, draft_name in [
    ("fn_2_163940", "createModel__11daWmGhost_cFv"),
    ("fn_2_163B30", "initState__11daWmGhost_cFv"),
    ("fn_2_163D60", "processCutsceneCommand__11daWmGhost_cFib"),
    ("fn_2_164180", '__sinit_\\d_a_wm_ghost_cpp'),
]:
    left = target[target_name]
    right = draft[draft_name]
    print("===", target_name, draft_name, len(left), len(right))
    for index, (a, b) in enumerate(zip(left, right)):
        if norm(a) != norm(b):
            print(index, "T:", a, "D:", b)
    if len(left) != len(right):
        print("length diff")
