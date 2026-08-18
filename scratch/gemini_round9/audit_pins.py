import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SLICES_JSON = ROOT / 'slices' / 'wiimj2d.json'
SYMS_TXT = ROOT / 'syms.txt'
SYMS_DTK = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'

# Load landed slices
slice_data = json.loads(SLICES_JSON.read_text())
landed_sources = set()
for s in slice_data['slices']:
    if s.get('source') and not s.get('nonMatching'):
        landed_sources.add(s['source'])

# Load syms.txt pins
syms_txt_pins = set()
for line in SYMS_TXT.read_text().splitlines():
    line = line.strip()
    if line and not line.startswith('#'):
        pin_name = line.split('=')[0].strip()
        syms_txt_pins.add(pin_name)

# Load dtk symbols (to find address and TU for all symbols)
dtk_syms = {}
for line in SYMS_DTK.read_text().splitlines():
    m = re.match(r'^(\S+)\s*=\s*(\.\w+):(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, sec, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        dtk_syms[name] = (sec, addr, meta)

# Callees and data refs from coin_main
callees = [
    'EnBgCheckFoot__5dEn_cFv',
    'EnBgCheckWall__5dEn_cFv',
    'GetResAnmChr__Q34nw4r3g3d7ResFileCFPCc',
    'GetResAnmTexSrt__Q34nw4r3g3d7ResFileCFPCc',
    'GetResMdl__Q34nw4r3g3d7ResFileCFPCc',
    'PSMTXTrans',
    'SetQuickSandEffect__15EffectManager_cFP7mVec3_c',
    'WaterCheck__5dEn_cFR7mVec3_cf',
    'XrotM__6mMtx_cF4mAng',
    'YrotM__6mMtx_cF4mAng',
    'ZrotM__6mMtx_cF4mAng',
    '__dl__7fBase_cFPv',
    '__dt__15dPanelObjList_cFv',
    '__dt__16dHeapAllocator_cFv',
    '__dt__5dEn_cFv',
    '__dt__9dBg_ctr_cFv',
    '__dt__Q23m3d11anmTexSrt_cFv',
    '__dt__Q23m3d5mdl_cFv',
    '__dt__Q23m3d6fanm_cFv',
    '_restgpr_27',
    '_savegpr_27',
    'adjustFrmHeap__16dHeapAllocator_cFv',
    'calc__Q23m3d9scnLeaf_cFb',
    'changePosAngle__8dActor_cFP7mVec3_cP7mAng3_ci',
    'check2__5dRc_cFUlUlUl',
    'checkFootEnm__5dBc_cFv',
    'checkHead__5dBc_cFUl',
    'coin_collisionCheck__18daEnObjCoinBlock_cFv',
    'createEffect__3mEfFPCcUlPC7mVec3_cPC7mAng3_cPC7mVec3_c',
    'createFrmHeap__16dHeapAllocator_cFUlPQ23EGG4HeapPCcUl',
    'create__Q23m3d11anmTexSrt_cFQ34nw4r3g3d6ResMdlQ34nw4r3g3d12ResAnmTexSrtP12mAllocator_cPUll',
    'create__Q23m3d5mdl_cFQ34nw4r3g3d6ResMdlP12mAllocator_cUliPUl',
    'create__Q23m3d8anmChr_cFQ34nw4r3g3d6ResMdlQ34nw4r3g3d9ResAnmChrP12mAllocator_cPUl',
    'cvtSndObjctPos__6dAudioFRC7mVec3_c',
    'deleteRequest__7fBase_cFv',
    'entry__5dCc_cFv',
    'getCenterPos__12dBaseActor_cCFv',
    'getRes__6dRes_cCFPCcPCc',
    'getUnitKind__5dBc_cFffUc',
    'getUnitType__5dBc_cFffUc',
    'setAnm__Q23m3d11anmTexSrt_cFRQ23m3d6bmdl_cQ34nw4r3g3d12ResAnmTexSrtlQ23m3d10playMode_e',
    'setAnm__Q23m3d5mdl_cFRQ23m3d6banm_cf',
    'setAnm__Q23m3d8anmChr_cFRQ23m3d6bmdl_cQ34nw4r3g3d9ResAnmChrQ23m3d10playMode_e',
    'setLocalMtx__Q23m3d9scnLeaf_cFPCQ34nw4r4math5MTX34',
    'setScale__Q23m3d9scnLeaf_cFRCQ34nw4r4math4VEC3',
    'setSoftLight_MapObj__8dActor_cFRQ23m3d6bmdl_c',
    'set__5dBc_cFP8dActor_cPC13sBcSensorIf_cPC13sBcSensorIf_cPC13sBcSensorIf_c',
    'set__5dCc_cFP8dActor_cP10sCcDatNewFUc',
    'set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c',
    'startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul'
]

data_symbols = [
    'g_gameHeaps__5mHeap',
    'g_pSndObjMap__6dAudio',
    'm_instance__9dResMng_c'
]

print("=== CALLEE & DATA PIN AUDIT ===")
needed_pins = []
already_pinned = []
in_landed_tu = []

# Let's check which symbols come from already landed files
# We can check by reading bin/dtk/dtk_splits_wiimj2d.txt
splits_lines = Path(ROOT / 'bin' / 'dtk' / 'dtk_splits_wiimj2d.txt').read_text().splitlines()
curr_split_src = None
split_ranges = []
for line in splits_lines:
    if line.endswith(':'):
        curr_split_src = line[:-1].strip()
    elif curr_split_src and 'start:' in line:
        parts = line.split()
        sec = parts[0]
        st = int(parts[1].split(':')[1], 16)
        en = int(parts[2].split(':')[1], 16)
        split_ranges.append((curr_split_src, sec, st, en))

def get_symbol_source(sym_name):
    if sym_name not in dtk_syms:
        return 'UNKNOWN'
    sec, addr, _ = dtk_syms[sym_name]
    for src, ssec, st, en in split_ranges:
        if ssec == sec and st <= addr < en:
            return src
    return 'UNSPLIT'

for sym in callees + data_symbols:
    sec, addr, meta = dtk_syms.get(sym, ('?', 0, '?'))
    src = get_symbol_source(sym)
    is_landed = (src in landed_sources)
    is_pinned = (sym in syms_txt_pins)
    
    status = "LANDED" if is_landed else ("PINNED" if is_pinned else "UNPINNED BLOCKER")
    if not is_landed and not is_pinned:
        needed_pins.append((sym, sec, hex(addr), src))
    elif is_pinned:
        already_pinned.append((sym, sec, hex(addr)))
    else:
        in_landed_tu.append((sym, sec, hex(addr), src))

print(f"Total external symbols checked: {len(callees) + len(data_symbols)}")
print(f"Defined in already landed TUs: {len(in_landed_tu)}")
print(f"Already pinned in syms.txt: {len(already_pinned)}")
print(f"Unpinned / Need Pinning in syms.txt: {len(needed_pins)}")

print("\n--- UNPINNED SYMBOLS NEEDING PINS IN syms.txt ---")
for sym, sec, addr_str, src in needed_pins:
    print(f"{sym:50s} = {addr_str}; // ({sec}, from {src})")

print("\n--- ALREADY PINNED IN syms.txt ---")
for sym, sec, addr_str in already_pinned:
    print(f"  {sym:50s} = {addr_str}")
