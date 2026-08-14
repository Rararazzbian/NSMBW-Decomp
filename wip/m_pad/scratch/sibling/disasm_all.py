import os, subprocess, sys

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")
SRC_DIR = os.path.join(ROOT, "bin", "compiled", "wiimj2d")
OUT_DIR = os.path.join(ROOT, "wip", "m_pad", "scratch", "sibling", "disasm")
os.makedirs(OUT_DIR, exist_ok=True)

count = 0
fails = []
for dirpath, dirnames, filenames in os.walk(SRC_DIR):
    for fn in filenames:
        if not fn.endswith(".o"):
            continue
        full = os.path.join(dirpath, fn)
        rel = os.path.relpath(full, SRC_DIR)
        outname = rel.replace(os.sep, "__") + ".txt"
        outpath = os.path.join(OUT_DIR, outname)
        p = subprocess.run([DTK, "elf", "disasm", full, outpath],
                            cwd=ROOT, capture_output=True, text=True)
        if p.returncode != 0:
            fails.append((rel, (p.stdout or "") + (p.stderr or "")))
        else:
            count += 1

print("disassembled:", count)
print("failed:", len(fails))
for rel, err in fails[:10]:
    print("  FAIL", rel, err[:200])
