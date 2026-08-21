import sys, os, re
sys.path.insert(0, os.path.join('tools','auto_decomp'))
import harness

SRC = os.path.join('scratch','round17','d_bg_actor_mng.cpp')
SHADOW = os.path.join('scratch','round17','shadow')

def run(name, override_dir):
    obj = os.path.join('wip','vec3pod','out', name + '.o')
    disasm_out = os.path.join('wip','vec3pod','out', name + '_disasm.txt')
    extra_inc = [SHADOW]
    if override_dir:
        extra_inc = [override_dir, SHADOW]
    ok, log = harness.compile_draft(SRC, obj, extra_inc=extra_inc, module='wiimj2d')
    if not ok:
        print(f"=== {name}: COMPILE FAILED ===")
        print(log[-3000:])
        return
    ok2, log2 = harness.disasm(obj, disasm_out)
    if not ok2:
        print(f"=== {name}: DISASM FAILED ===")
        print(log2[-2000:])
        return
    with open(disasm_out, encoding='utf-8', errors='replace') as fh:
        text = fh.read()
    body = harness.extract(disasm_out, 'ProcMain__17dBgActorManager_cFv')
    if body is None:
        print(f"=== {name}: function not found in disasm ===")
        return
    n = len(body)
    has_lwz_copy = any('lwz' in l or 'stw' in l for l in body)
    has_lfs_copy = any('lfs' in l or 'stfs' in l for l in body)
    print(f"=== {name}: {n} words | has lwz/stw: {has_lwz_copy} | has lfs/stfs: {has_lfs_copy} ===")

if __name__ == '__main__':
    name = sys.argv[1]
    override = sys.argv[2] if len(sys.argv) > 2 else None
    run(name, override)
