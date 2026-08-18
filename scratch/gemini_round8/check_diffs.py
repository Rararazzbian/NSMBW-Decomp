import os, subprocess

# Let's verify diff for m_vec.hpp and eggVector.h
with open('include/game/mLib/m_vec.hpp', 'r') as f:
    orig_vec = f.read()

with open('scratch/gemini_round8/mock_include/game/mLib/m_vec.hpp', 'r') as f:
    mod_vec = f.read()

with open('include/lib/egg/math/eggVector.h', 'r') as f:
    orig_egg = f.read()

with open('scratch/gemini_round8/mock_include/lib/egg/math/eggVector.h', 'r') as f:
    mod_egg = f.read()

print("m_vec.hpp changed:", orig_vec != mod_vec)
print("eggVector.h changed:", orig_egg != mod_egg)
