import os, sys, re, subprocess
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'gemini_round24')
INC = os.path.join(BASE, 'include')
GAME = os.path.join(BASE, 'game')
TGT = os.path.join(BASE, 'target_all_text.txt')

with open(os.path.join(BASE, 'd_enemy_toride_kokoopa.cpp'), 'r', encoding='utf-8') as f:
    base_code = f.read()

pattern = r'void dEnTorideKokoopa_c::setQuakeDead\(\) \{.*?\n\}'
orig_fn = re.search(pattern, base_code, re.DOTALL).group(0)

tests = []

def add(name, code):
    tests.append((name, code))

# Test A: Variations around deathData / mDeathInfo at the end
add('Death_direct_assign', '''void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    mDeathInfo = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, dir, 0xFF };
}''')

add('Death_member_assign', '''void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    mDeathInfo = deathData;
    mDeathInfo.mDirection = dir;
}''')

# Test B: dir usage / type
add('Dir_u32', '''void dEnTorideKokoopa_c::setQuakeDead() {
    u32 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}''')

add('Dir_int', '''void dEnTorideKokoopa_c::setQuakeDead() {
    int dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}''')

add('Dir_late', '''void dEnTorideKokoopa_c::setQuakeDead() {
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    u8 dir = getPl_LRflag(mPos);
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}''')

# Test C: Volatile / local / dummy uses that block r29
add('Dummy_r29_live', '''void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}''')

# Test D: fManager_c::searchBaseByID without cast or with different cast
add('SearchBase_direct_int', '''void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    fBase_c *base = (mUnk770 == 0) ? nullptr : fManager_c::searchBaseByID(mUnk770);
    if (base != nullptr) {
        base->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}''')

add('SearchBase_actor_ternary', '''void dEnTorideKokoopa_c::setQuakeDead() {
    u8 dir = getPl_LRflag(mPos);
    if (mAnmMatClr.mpChildren[1].getObj() != nullptr) {
        mAnmMatClr.setFrame(0.0f, 1);
    }
    removeCc();
    mCc.release();
    mUnk792 = 0;
    mUnk790 = 0;
    dScoreMng_c::m_instance->UnKnownScoreSet(this, 6, 0.0f, 24.0f);
    dActor_c *act = (mUnk770 == 0) ? nullptr : (dActor_c*)fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (act != nullptr) {
        act->deleteRequest();
    }
    mActorProperties &= ~8;
    sDeathInfoData deathData = (sDeathInfoData){ 0.0f, 3.0f, -4.0f, -0.1875f, &dEnBoss_c::StateID_DieStar, -1, -1, 0, 0xFF };
    deathData.mDirection = dir;
    mDeathInfo = deathData;
}''')

for name, body in tests:
    new_code = base_code.replace(orig_fn, body)
    src = os.path.join(BASE, 'test_quake_opt.cpp')
    obj = os.path.join(BASE, 'test_quake_opt.o')
    txt = os.path.join(BASE, 'test_quake_opt.txt')
    with open(src, 'w', encoding='utf-8') as f:
        f.write(new_code)
    ok, log = harness.compile_draft(src, obj, extra_inc=(INC, GAME, BASE), module='wiimj2d')
    if not ok:
        print(f'{name:25s} | COMPILE FAIL')
        continue
    harness.disasm(obj, txt)
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'auto_decomp', 'fndiff.py'), TGT, txt, 'setQuakeDead__18dEnTorideKokoopa_cFv'], capture_output=True, text=True)
    with open(txt, 'r', encoding='utf-8') as f:
        content = f.read()
    m = re.search(r'\.fn setQuakeDead__18dEnTorideKokoopa_cFv,.*?\n(.*?\n)\.endfn', content, re.DOTALL)
    fn_body = m.group(1) if m else ''
    lines = [l.strip() for l in fn_body.splitlines() if '*/' in l]
    
    store_reg = None
    for l in lines:
        if 'sth' in l and ('0x792' in l or '0x790' in l):
            m_sth = re.search(r'sth\s+(r\d+)', l)
            if m_sth: store_reg = m_sth.group(1)
            break
            
    has_li_r3_0 = False
    for i, l in enumerate(lines):
        if 'searchBaseByID' in l:
            for prev in lines[max(0, i-5):i]:
                if 'li r3, 0x0' in prev or 'li r3, 0' in prev:
                    has_li_r3_0 = True
                    break
                    
    merge_reg = None
    for i, l in enumerate(lines):
        if 'deleteRequest' in l:
            for prev in reversed(lines[:i]):
                if 'cmpwi' in prev:
                    m_c = re.search(r'cmpwi\s+(r\d+)', prev)
                    if m_c: merge_reg = m_c.group(1)
                    break
            break
            
    frame = None
    gprs = []
    for l in lines[:10]:
        if 'stwu r1,' in l:
            frame = l.split(',')[-1].split('(')[0].strip()
        if 'stw r' in l:
            m_gpr = re.search(r'stw\s+(r\d+)', l)
            if m_gpr and m_gpr.group(1) != 'r0': gprs.append(m_gpr.group(1))
            
    diff_line = ''
    for l in r.stdout.splitlines():
        if 'DIFFS' in l:
            diff_line = l.strip()
            
    print(f'{name:25s} | words: {len(lines):2d} | frame: {frame:5s} | GPR: {str(gprs):15s} | store: {str(store_reg):4s} | merge: {str(merge_reg):4s} | li r3,0: {str(has_li_r3_0):5s} | {diff_line}')
