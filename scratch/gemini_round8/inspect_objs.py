import os, subprocess

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")

def disasm_obj(obj_rel_path, out_rel_path):
    obj = os.path.join(ROOT, obj_rel_path)
    out = os.path.join(ROOT, out_rel_path)
    p = subprocess.run([DTK, 'elf', 'disasm', obj, out], cwd=ROOT, capture_output=True, text=True)
    if p.returncode != 0:
        print(f"Error disassembling {obj_rel_path}: {p.stderr}")
    else:
        print(f"Successfully disassembled {obj_rel_path} -> {out_rel_path}")

disasm_obj("bin/dtkspl/obj/auto_07_80329A70_data.o", "scratch/gemini_round8/data_80329A70.txt")
disasm_obj("bin/dtkspl/obj/auto_07_80329F60_data.o", "scratch/gemini_round8/data_80329F60.txt")
disasm_obj("bin/dtkspl/obj/auto_07_80329FA0_data.o", "scratch/gemini_round8/data_80329FA0.txt")
disasm_obj("bin/dtkspl/obj/auto_03_8016F330_text.o", "scratch/gemini_round8/text_8016F330.txt")
disasm_obj("bin/dtkspl/obj/auto_03_8016F808_text.o", "scratch/gemini_round8/text_8016F808.txt")
disasm_obj("bin/dtkspl/obj/auto_03_800CE7F0_text.o", "scratch/gemini_round8/text_800CE7F0.txt")
