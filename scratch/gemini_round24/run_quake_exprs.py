import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.run_quake_sweep import test_quake_var, make_body

exprs = [
    ("E1: (fBase_c*)(mUnk770 - mUnk770)",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 - mUnk770) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("E2: (fBase_c*)(mUnk770 ^ mUnk770)",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 ^ mUnk770) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("E3: (fBase_c*)(u32)mUnk790",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(u32)mUnk790 : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("E4: (fBase_c*)(mUnk770 & 1)",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 & 1) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("E5: (fBase_c*)(mUnk770 >> 31)",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 >> 31) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),

    ("E6: (fBase_c*)(mUnk770 * 0)",
     make_body("""    fBase_c *base = (mUnk770 == 0) ? (fBase_c*)(mUnk770 * 0) : fManager_c::searchBaseByID((fBaseID_e)mUnk770);
    if (base != 0) base->deleteRequest();""")),
]

for name, body in exprs:
    test_quake_var(name, body)
