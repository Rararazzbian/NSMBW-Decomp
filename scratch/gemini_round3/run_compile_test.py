import sys
import os

sys.path.append(os.path.abspath('.'))
from tools.auto_decomp.harness import compile_draft

src = os.path.abspath('scratch/gemini_round3/test_scaffold.cpp')
obj = os.path.abspath('scratch/gemini_round3/test_scaffold.o')

ok, log = compile_draft(src, obj)
print("Compile success:", ok)
if log:
    print("Log:\n", log)
