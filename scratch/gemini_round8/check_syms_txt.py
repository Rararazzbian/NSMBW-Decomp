import os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"

with open(os.path.join(ROOT, 'syms.txt'), 'r', encoding='utf-8') as f:
    syms_txt = f.readlines()

symbols_to_check = [
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

print("=== Symbols defined by d_multi_manager.cpp in syms.txt ===")
for s in symbols_to_check:
    found = [l.strip() for l in syms_txt if s in l]
    if found:
        for f in found:
            print(f"  FOUND: {f}")
    else:
        print(f"  NOT in syms.txt: {s}")
