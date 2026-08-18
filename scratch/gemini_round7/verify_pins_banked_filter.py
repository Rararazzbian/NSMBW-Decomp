import json, os

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

with open(os.path.join(ROOT, 'slices', 'wiimj2d.json')) as f:
    slices_data = json.load(f)

meta_sec = slices_data['meta']['sections']
sec_bases = {}
for sname, sinfo in meta_sec.items():
    addr = int(sinfo['addr'], 16)
    off = int(sinfo.get('offset', '0'), 16)
    sec_bases[sname] = addr + off

banked_intervals = []
for s in slices_data['slices']:
    src = s['source']
    mr = s.get('memoryRanges', {})
    for sec_name, rng in mr.items():
        base = sec_bases.get(sec_name, 0)
        a_str, b_str = rng.split('-')
        start_addr = base + int(a_str, 16)
        end_addr = base + int(b_str, 16)
        banked_intervals.append((start_addr, end_addr, src, sec_name))

candidates = [
    ("Print__Q34nw4r2ut17TextWriterBase<c>FPCci", 0x8022f010),
    ("__ct__Q34nw4r2ut17TextWriterBase<c>Fv", 0x8022df20),
    ("__dt__Q34nw4r2ut17TextWriterBase<c>Fv", 0x8022df80),
    ("EnableLinearFilter__Q34nw4r2ut10CharWriterFbb", 0x8022d700),
    ("MEMAllocFromExpHeapEx", 0x801d45a0),
    ("MEMCreateExpHeapEx", 0x801d44c0),
    ("MEMFreeToExpHeap", 0x801d4850),
    ("MEMGetAllocatableSizeForExpHeapEx", 0x801d49a0),
    ("MEMSetGroupIDForExpHeap", 0x801d4ae0),
    ("UpdateVertexColor__Q34nw4r2ut10CharWriterFv", 0x8022dae0),
    ("WPADGetInfoAsync", 0x801e1400),
    ("getNthController__Q23EGG17CoreControllerMgrFi", 0x802bd660),
    ("init__Q23EGG10CoreStatusFv", 0x802bc9d0),
    ("sInstance__Q23EGG17CoreControllerMgr", 0x8042b150),
    ("sceneReset__Q23EGG14CoreControllerFv", 0x802bcaf0),
    ("vsnprintf", 0x802e18cc),
    ("vswprintf", 0x802e4680),
]

print("=== BANKED-SLICE FILTER RESULTS ===")
collisions = 0
for name, addr in candidates:
    hit = False
    for start, end, src, sec in banked_intervals:
        if start <= addr < end:
            print(f"COLLISION: {name} (0x{addr:08X}) falls in {src} [{sec}] ({hex(start)}..{hex(end)})")
            hit = True
            collisions += 1
            break
    if not hit:
        print(f"CLEAN: {name:50} = 0x{addr:08X}")

print(f"\nSummary: {len(candidates)} checked, {len(candidates) - collisions} clean, {collisions} collisions.")
