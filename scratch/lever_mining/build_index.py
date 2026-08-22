import json, os, subprocess, sys

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK  = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")
OUT  = os.path.join(ROOT, "scratch", "lever_mining", "disasm")
os.makedirs(OUT, exist_ok=True)

MODULES = ["wiimj2d", "d_basesNP", "d_enemiesNP", "d_en_bossNP", "d_profileNP"]

index = []
missing_obj = []
missing_src = []

for mod in MODULES:
    sp = os.path.join(ROOT, "slices", mod + ".json")
    if not os.path.exists(sp):
        continue
    d = json.load(open(sp))
    for s in d["slices"]:
        rel = s["source"]                     # e.g. dol/bases/d_2d.cpp
        obj = os.path.join(ROOT, "bin", "compiled", mod, rel[:-4] + ".o")
        src = os.path.join(ROOT, "source", rel)
        if not os.path.exists(obj):
            missing_obj.append((mod, rel)); continue
        if not os.path.exists(src):
            missing_src.append((mod, rel)); continue
        key = mod + "__" + rel[:-4].replace("/", "_").replace("\\", "_")
        dis = os.path.join(OUT, key + ".txt")
        if not os.path.exists(dis):
            r = subprocess.run([DTK, "elf", "disasm", obj, dis],
                               capture_output=True, text=True)
            if r.returncode != 0:
                print("DISASM FAIL", obj, r.stderr[:200]); continue
        index.append({"module": mod, "rel": rel, "obj": obj, "src": src, "disasm": dis})

json.dump(index, open(os.path.join(ROOT, "scratch", "lever_mining", "index.json"), "w"), indent=1)
print("paired:", len(index))
print("missing obj:", len(missing_obj), missing_obj[:10])
print("missing src:", len(missing_src), missing_src[:10])
