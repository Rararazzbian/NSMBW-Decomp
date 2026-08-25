"""Emit operate() register-search variants into sw_perm/. Bodies differ only in
how out/pl/en(ev) are bound to locals; structure and offsets stay fixed."""
import os

HERE = os.path.dirname(os.path.abspath(__file__))

PRE = '''#include <game/bases/d_actor.hpp>
#include <game/bases/d_en_fumi_check.hpp>
#include <types.h>

/// @unofficial Read-only view of the fields the fumi checks touch on the
/// player actor. The leading data block places the class vptr at +0x60,
/// matching the real player layout.
class FumiPlayerView {
public:
    u8 mLead[0x60];

    virtual ~FumiPlayerView();
    virtual void vf08();
    virtual void vf0C();
    virtual void vf10();
    virtual void vf14();
    virtual void vf18();
    virtual void vf1C();
    virtual void vf20();
    virtual void vf24();
    virtual void vf28();
    virtual void vf2C();
    virtual void vf30();
    virtual void vf34();
    virtual void vf38();
    virtual void vf3C();
    virtual void vf40();
    virtual void vf44();
    virtual void vf48();
    virtual void vf4C();
    virtual void vf50();
    virtual void vf54();
    virtual void vf58();
    virtual void vf5C();
    virtual void vf60();
    virtual void vf64();
    virtual const s8 *getDamageTable(); ///< vtable slot at +0x6C.

public:
    u8 mPad1[0xB0 - 0x64];
    float mB0; ///< @unofficial
    u8 mPad2[0xEC - 0xB4];
    float mEC; ///< @unofficial
    u8 mPad3[0x1074 - 0xF0];
    u32 mFlagA; ///< @unofficial
    u32 mFlagB; ///< @unofficial
    u8 mPad4[0x1090 - 0x107C];
    s32 mMode;  ///< @unofficial
};

/// @unofficial Read-only view of the enemy-side fields written by the check.
class FumiEnemyView {
public:
    u8 mPad[0xB0];
    float mB0; ///< @unofficial
    u8 mPad2[0xEC - 0xB4];
    float mEC; ///< @unofficial
    u8 mPad3[0x504 - 0xF0];
    u16 mFumiVals[0x80]; ///< @unofficial damage values, indexed by table entry.
};

KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c() {}

'''

# Shared tail: everything after the local-binding header. %s = enemy access expr,
# %e = name of enemy view expression prefix.
TAIL_TMPL = '''
    if (((pl->mFlagA | pl->mFlagB) != 0) && pl->mEC > 0.0f) {{
        out = 0;
        return true;
    }}

    if (!((dBc_c *) ((u8 *) pl + 0x1EC))->isFoot()) {{
        if ({ev}->mEC > 0.0f) {{
            if (pl->mMode == 3) {{
                if (pl->mB0 >= {ev}->mB0 + 4.0f) {{
                    const s8 *tbl = pl->getDamageTable();
                    s8 idx = tbl[0];
                    {ev}->mFumiVals[idx] = 0x18;
                    out = 1;
                    return true;
                }}
            }} else {{
                if (pl->mB0 >= {ev}->mB0 + 10.0f) {{
                    const s8 *tbl = pl->getDamageTable();
                    s8 idx = tbl[0];
                    {ev}->mFumiVals[idx] = 0x18;
                    out = 1;
                    return true;
                }}
            }}
        }}
    }}
    return false;
}}
'''

SIG = 'bool KokoopaSpFumiCheck_c::operate(int &out, dEn_c *en, FumiCcInfo_c &info) {'


def tail(ev='ev'):
    return TAIL_TMPL.format(ev=ev)


VARIANTS = {}

# vb: ev declared before pl
VARIANTS['vb'] = SIG + '''
    out = 0;
    FumiEnemyView *ev = (FumiEnemyView *) en;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
''' + tail()

# vc: no ev local at all -- cast at every use site
VARIANTS['vc'] = SIG + '''
    out = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
''' + tail('((FumiEnemyView *) en)')

# vd: ev defined lazily inside the isFoot guard (its first use)
VARIANTS['vd'] = SIG + '''
    out = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();

    if (((pl->mFlagA | pl->mFlagB) != 0) && pl->mEC > 0.0f) {
        out = 0;
        return true;
    }

    if (!((dBc_c *) ((u8 *) pl + 0x1EC))->isFoot()) {
        FumiEnemyView *ev = (FumiEnemyView *) en;
''' + TAIL_TMPL.format(ev='ev').replace('\n    if', '\n        if', 1)

# ve: intermediate local for the collision struct
VARIANTS['ve'] = SIG + '''
    out = 0;
    dCc_c *cc2 = info.mCc2;
    FumiPlayerView *pl = (FumiPlayerView *) cc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail()

# vf: bind int &o = out as the very first statement, use o everywhere
body_vf = '''
    int &o = out;
    o = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;

    if (((pl->mFlagA | pl->mFlagB) != 0) && pl->mEC > 0.0f) {
        o = 0;
        return true;
    }

    if (!((dBc_c *) ((u8 *) pl + 0x1EC))->isFoot()) {
        if (ev->mEC > 0.0f) {
            if (pl->mMode == 3) {
                if (pl->mB0 >= ev->mB0 + 4.0f) {
                    const s8 *tbl = pl->getDamageTable();
                    s8 idx = tbl[0];
                    ev->mFumiVals[idx] = 0x18;
                    o = 1;
                    return true;
                }
            } else {
                if (pl->mB0 >= ev->mB0 + 10.0f) {
                    const s8 *tbl = pl->getDamageTable();
                    s8 idx = tbl[0];
                    ev->mFumiVals[idx] = 0x18;
                    o = 1;
                    return true;
                }
            }
        }
    }
    return false;
}
'''
VARIANTS['vf'] = SIG + body_vf

# vi: const-qualified pointers
VARIANTS['vi'] = SIG + '''
    out = 0;
    FumiPlayerView *const pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *const ev = (FumiEnemyView *) en;
''' + tail()

# vj: pl split into decl + assign
VARIANTS['vj'] = SIG + '''
    out = 0;
    FumiPlayerView *pl;
    pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail()

# vk: both locals split, declared first, assigned after out=0
VARIANTS['vk'] = SIG + '''
    FumiPlayerView *pl;
    FumiEnemyView *ev;
    out = 0;
    pl = (FumiPlayerView *) info.mCc2->getOwner();
    ev = (FumiEnemyView *) en;
''' + tail()

# vl: locals initialised BEFORE out = 0
VARIANTS['vl'] = SIG + '''
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
    out = 0;
''' + tail()

# vm: ev declared early (after pl), assigned late inside the guard
VARIANTS['vm'] = SIG + '''
    out = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev;

    if (((pl->mFlagA | pl->mFlagB) != 0) && pl->mEC > 0.0f) {
        out = 0;
        return true;
    }

    if (!((dBc_c *) ((u8 *) pl + 0x1EC))->isFoot()) {
        ev = (FumiEnemyView *) en;
''' + TAIL_TMPL.format(ev='ev').replace('\n    if', '\n        if', 1)

# vn: vc shape but pl also split decl/assign (no ev anywhere)
VARIANTS['vn'] = SIG + '''
    out = 0;
    FumiPlayerView *pl;
    pl = (FumiPlayerView *) info.mCc2->getOwner();
''' + tail('((FumiEnemyView *) en)')

# vo: vc shape, pl declared before out = 0
VARIANTS['vo'] = SIG + '''
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    out = 0;
''' + tail('((FumiEnemyView *) en)')

for name, body in VARIANTS.items():
    with open(os.path.join(HERE, name + '.cpp'), 'w') as fh:
        fh.write(PRE + body)
print('wrote:', ', '.join(sorted(VARIANTS)))
