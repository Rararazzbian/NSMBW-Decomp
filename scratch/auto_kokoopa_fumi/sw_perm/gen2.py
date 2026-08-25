"""Emit menu variants pa..pm for the operate() register search.
Bodies differ only in how out / pl / en(ev) are bound; logic + offsets fixed."""
import os

HERE = os.path.dirname(os.path.abspath(__file__))

from gen import PRE  # reuse the exact class prelude


def tail(pl='pl', ev='ev', outw='out'):
    return f'''
    if ((({pl}->mFlagA | {pl}->mFlagB) != 0) && {pl}->mEC > 0.0f) {{
        {outw} = 0;
        return true;
    }}

    if (!((dBc_c *) ((u8 *) {pl} + 0x1EC))->isFoot()) {{
        if ({ev}->mEC > 0.0f) {{
            if ({pl}->mMode == 3) {{
                if ({pl}->mB0 >= {ev}->mB0 + 4.0f) {{
                    const s8 *tbl = {pl}->getDamageTable();
                    s8 idx = tbl[0];
                    {ev}->mFumiVals[idx] = 0x18;
                    {outw} = 1;
                    return true;
                }}
            }} else {{
                if ({pl}->mB0 >= {ev}->mB0 + 10.0f) {{
                    const s8 *tbl = {pl}->getDamageTable();
                    s8 idx = tbl[0];
                    {ev}->mFumiVals[idx] = 0x18;
                    {outw} = 1;
                    return true;
                }}
            }}
        }}
    }}
    return false;
}}
'''

SIG = 'bool KokoopaSpFumiCheck_c::operate(int &out, dEn_c *en, FumiCcInfo_c &info) {'
PLX = '((FumiPlayerView *) info.mCc2->getOwner())'

VARIANTS = {}

# pa: pointer-to-parameter local
VARIANTS['pa'] = SIG + '''
    int *const pout = &out;
    *pout = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail(outw='*pout')

# pb: no pl local -- re-derive owner at every use
VARIANTS['pb'] = SIG + '''
    out = 0;
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail(pl=PLX)

# pc: cache the collision struct instead (== earlier ve, rebuilt for the table)
VARIANTS['pc'] = SIG + '''
    out = 0;
    dCc_c *cc2 = info.mCc2;
    FumiPlayerView *pl = (FumiPlayerView *) cc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail()

# pd: base-class intermediate for BOTH actor pointers
VARIANTS['pd'] = SIG + '''
    out = 0;
    dActor_c *pbase = info.mCc2->getOwner();
    FumiPlayerView *pl = (FumiPlayerView *) pbase;
    dActor_c *ebase = (dActor_c *) en;
    FumiEnemyView *ev = (FumiEnemyView *) ebase;
''' + tail()

# pe: base-class intermediate for pl only
VARIANTS['pe'] = SIG + '''
    out = 0;
    dActor_c *pbase = info.mCc2->getOwner();
    FumiPlayerView *pl = (FumiPlayerView *) pbase;
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail()

# pf: pa + pb combined
VARIANTS['pf'] = SIG + '''
    int *const pout = &out;
    *pout = 0;
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail(pl=PLX, outw='*pout')

# pg: pl derived before out = 0 (== earlier vl, rebuilt for the table)
VARIANTS['pg'] = SIG + '''
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
    out = 0;
''' + tail()

# ph: reference to out bound LAST, writes through it
ph_body = '''
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
    int &o = out;
    o = 0;

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
VARIANTS['ph'] = SIG + ph_body

# pi: const cc2 + const pout combined
pi_body = '''
    dCc_c *const cc2 = info.mCc2;
    FumiPlayerView *pl = (FumiPlayerView *) cc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
    int *const pout = &out;
    *pout = 0;

    if (((pl->mFlagA | pl->mFlagB) != 0) && pl->mEC > 0.0f) {
        *pout = 0;
        return true;
    }

    if (!((dBc_c *) ((u8 *) pl + 0x1EC))->isFoot()) {
        if (ev->mEC > 0.0f) {
            if (pl->mMode == 3) {
                if (pl->mB0 >= ev->mB0 + 4.0f) {
                    const s8 *tbl = pl->getDamageTable();
                    s8 idx = tbl[0];
                    ev->mFumiVals[idx] = 0x18;
                    *pout = 1;
                    return true;
                }
            } else {
                if (pl->mB0 >= ev->mB0 + 10.0f) {
                    const s8 *tbl = pl->getDamageTable();
                    s8 idx = tbl[0];
                    ev->mFumiVals[idx] = 0x18;
                    *pout = 1;
                    return true;
                }
            }
        }
    }
    return false;
}
'''
VARIANTS['pi'] = SIG + pi_body

# pj: pl split decl/assign (== earlier vj, rebuilt for the table)
VARIANTS['pj'] = SIG + '''
    out = 0;
    FumiPlayerView *pl;
    pl = (FumiPlayerView *) info.mCc2->getOwner();
    FumiEnemyView *ev = (FumiEnemyView *) en;
''' + tail()

# pk: en rebound through a void* hop
VARIANTS['pk'] = SIG + '''
    out = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner();
    void *evp = en;
    FumiEnemyView *ev = (FumiEnemyView *) evp;
''' + tail()

# pm: both views bound in ONE declaration statement
VARIANTS['pm'] = SIG + '''
    out = 0;
    FumiPlayerView *pl = (FumiPlayerView *) info.mCc2->getOwner(), *ev = (FumiEnemyView *) en;
''' + tail()

for name, body in VARIANTS.items():
    with open(os.path.join(HERE, name + '.cpp'), 'w') as fh:
        fh.write(PRE + body)
print('wrote:', ', '.join(sorted(VARIANTS)))
