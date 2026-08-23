import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.test_jump_sweep import test_variant

# Let us test different declaration orders and expressions
candidates = [
    ("H1: sx before muki",
     """    float rate = calcJumpRate();
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * sx;"""),

    ("H2: sx before rate",
     """    float sx = speed.x;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    mSpeed.x = (muki * rate) * sx;"""),

    ("H3: sy, sx before rate",
     """    float sy = speed.y;
    float sx = speed.x;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;"""),

    ("H4: sy before rate, sx after rate",
     """    float sy = speed.y;
    float rate = calcJumpRate();
    float sx = speed.x;
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;"""),

    ("H5: sy before rate, sx after muki",
     """    float sy = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sx = speed.x;
    mSpeed.y = sy;
    mSpeed.x = (muki * rate) * sx;"""),

    ("H6: mSpeed.y before rate",
     """    mSpeed.y = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;"""),

    ("H7: mSpeed.y before rate, mSpeed.x direct",
     """    mSpeed.y = speed.y;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.x = (muki * rate) * speed.x;"""),
]

for name, code in candidates:
    test_variant(name, code)
