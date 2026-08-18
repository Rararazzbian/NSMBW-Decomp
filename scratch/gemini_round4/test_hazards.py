import os
import subprocess
import re
import sys

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"
MWCC = os.path.join(ROOT, "compilers", "Wii", "1.1", "mwcceppc.exe")
DTK = os.path.join(ROOT, "bin", "dtk-windows-x86_64.exe")

CFLAGS = [
    "-c", "-proc", "gekko", "-fp", "hard", "-O4", "-inline", "noauto",
    "-Cpp_exceptions", "off", "-enum", "int", "-RTTI", "off", "-ipa", "file",
    "-enc", "SJIS", "-DREVOLUTION", "-I-"
]

INCLUDES = [
    os.path.join(ROOT, "include"),
    os.path.join(ROOT, "include", "lib"),
    os.path.join(ROOT, "include", "lib", "MSL"),
    os.path.join(ROOT, "include", "lib", "MSL", "internal"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "include"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "stack", "include"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "stack", "btm"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "bta", "include"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "bta", "sys"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "gki", "common"),
    os.path.join(ROOT, "include", "lib", "revolution", "BTE", "gki", "platform")
]

def compile_src(src_path, obj_path, extra_includes=()):
    args = [MWCC] + CFLAGS + [src_path, "-o", obj_path]
    for inc in list(extra_includes) + INCLUDES:
        args += ["-i", inc]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or "") + (p.stderr or "")

def disasm_elf(obj_path, out_path):
    p = subprocess.run([DTK, "elf", "disasm", obj_path, out_path], cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or "") + (p.stderr or "")

print("Test harness initialized.")
