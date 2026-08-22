"""Build a scratch mirror of bin/dtk + slices in which DOL symbol addresses are
SECTION-RELATIVE OFFSETS, so the unmodified wip/wm_units/check_bounds.py can be
run against the DOL.  check_bounds.py derives ROOT from its own path
(dirname^3), so a copy at scratch/next_targets/shim/wm_units/check_bounds.py
sees ROOT = scratch/next_targets.  Nothing under the project source is touched.
"""
import os, re, json, shutil, sys

PROJ = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
SHIMROOT = os.path.join(PROJ, "scratch", "next_targets")

sl = json.load(open(os.path.join(PROJ, "slices", "wiimj2d.json")))
BASE = {k: int(v["addr"], 16) for k, v in sl["meta"]["sections"].items()}
# .ctors/.dtors carry an extra `offset` for the $10 sub-sections; slice ranges
# are relative to the SECTION base, which is what BASE already holds.

os.makedirs(os.path.join(SHIMROOT, "bin", "dtk"), exist_ok=True)
os.makedirs(os.path.join(SHIMROOT, "slices"), exist_ok=True)
os.makedirs(os.path.join(SHIMROOT, "shim", "wm_units"), exist_ok=True)
os.makedirs(os.path.join(SHIMROOT, "original"), exist_ok=True)

pat = re.compile(r"^(\S+)\s*=\s*(\.\w+):0x([0-9A-Fa-f]+);(.*)$")
out = []
src = os.path.join(PROJ, "bin", "dtk", "wiimj2d_symbols.txt")
for line in open(src, encoding="utf-8", errors="replace"):
    m = pat.match(line.strip())
    if not m:
        continue
    name, secn, addr, rest = m.group(1), m.group(2), int(m.group(3), 16), m.group(4)
    if secn not in BASE:
        continue
    out.append("%s = %s:0x%X;%s" % (name, secn, addr - BASE[secn], rest))
open(os.path.join(SHIMROOT, "bin", "dtk", "wiimj2ddol_symbols.txt"), "w",
     encoding="utf-8").write("\n".join(out) + "\n")

shutil.copy(os.path.join(PROJ, "slices", "wiimj2d.json"),
            os.path.join(SHIMROOT, "slices", "wiimj2ddol.json"))
shutil.copy(os.path.join(PROJ, "wip", "wm_units", "check_bounds.py"),
            os.path.join(SHIMROOT, "shim", "wm_units", "check_bounds.py"))
print("shim built: %d symbols" % len(out))
