import json, os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"

with open(os.path.join(ROOT, 'slices', 'wiimj2d.json'), 'r') as f:
    slices_data = json.load(f)

# Find all banked slices and existing definitions
banked_units = []
for unit, sections in slices_data.items():
    if not any(k.startswith('.') for k in sections.keys()):
        # might be structured as { unit: { '.text': [start, end], ... } }
        pass
    banked_units.append(unit)

print(f"Total slices in wiimj2d.json: {len(slices_data)}")

# Let's inspect symbols in syms.txt
with open(os.path.join(ROOT, 'syms.txt'), 'r', encoding='utf-8') as f:
    syms_txt = f.readlines()

multi_syms = [
    '__ct__11dMultiMng_cFv',
    '__dt__11dMultiMng_cFv',
    'initStage__11dMultiMng_cFv',
    'setClapSE__11dMultiMng_cFv',
    'setRest__11dMultiMng_cFii',
    'addScore__11dMultiMng_cFii',
    'incCoin__11dMultiMng_cFi',
    'incEnemyDown__11dMultiMng_cFi',
    'setBattleCoin__11dMultiMng_cFii',
    'setCollectionCoin__11dMultiMng_cFv',
    '__vt__11dMultiMng_c',
    'mspInstance__11dMultiMng_c',
]

print("=== Check multi symbols in syms.txt ===")
for s in multi_syms:
    for line in syms_txt:
        if s in line:
            print(f"Found in syms.txt: {line.strip()}")

# Check external references made by d_multi_mng.cpp
ext_refs = [
    '__dl__FPv',
    'cvtSndObjctPos__6dAudioFRC7mVec2_c',
    'startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul',
    'changeItemKinopioPlrNo__9daPyMng_cFRi',
    'getCtrlPlayer__9daPyMng_cFi',
    'CreateSmallScore__8dGameComFRC7mVec3_ciib',
    'ms_Instance_p__14dBgParameter_c',
    'g_pSndObjMap__6dAudio',
    'mGameFlag__7dInfo_c',
    'mCollectionCoin__10dScStage_c'
]

print("\n=== Check external references in syms.txt / wiimj2d_symbols.txt ===")
with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'), 'r', encoding='utf-8') as f:
    all_syms = f.readlines()

for ref in ext_refs:
    found_wiimj2d = [l.strip() for l in all_syms if ref in l]
    found_symstxt = [l.strip() for l in syms_txt if ref in l]
    print(f"Ref: {ref}")
    if found_wiimj2d:
        print(f"  In wiimj2d_symbols: {found_wiimj2d[0]}")
    else:
        print(f"  NOT in wiimj2d_symbols!")
    if found_symstxt:
        print(f"  In syms.txt: {found_symstxt[0]}")
