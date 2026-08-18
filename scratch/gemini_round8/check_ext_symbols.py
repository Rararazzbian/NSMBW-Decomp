import os

ROOT = r"c:\Users\Razz\Documents\Projects\NSMBW-Decomp"

with open(os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt'), 'r', encoding='utf-8') as f:
    symbols = f.readlines()

with open(os.path.join(ROOT, 'syms.txt'), 'r', encoding='utf-8') as f:
    syms_txt = f.readlines()

ext_symbols = [
    'changeItemKinopioPlrNo__9daPyMng_cFRi',
    'getCtrlPlayer__9daPyMng_cFi',
    'CreateSmallScore__8dGameComFRC7mVec3_ciib',
    'cvtSndObjctPos__6dAudioFRC7mVec2_c',
    'startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul',
    'ms_Instance_p__14dBgParameter_c',
    'g_pSndObjMap__6dAudio',
    'mGameFlag__7dInfo_c',
    'mCollectionCoin__10dScStage_c',
    '__dl__FPv',
]

print("=== External Symbol Check ===")
for s in ext_symbols:
    wiimj2d_match = [l.strip() for l in symbols if s in l]
    symstxt_match = [l.strip() for l in syms_txt if s in l]
    w_info = wiimj2d_match[0] if wiimj2d_match else "MISSING in wiimj2d_symbols"
    s_info = symstxt_match[0] if symstxt_match else "Not in syms.txt"
    print(f"Symbol: {s}")
    print(f"  wiimj2d: {w_info}")
    print(f"  syms.txt: {s_info}")
