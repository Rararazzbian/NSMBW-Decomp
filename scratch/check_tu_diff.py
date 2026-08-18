import os
import sys

sys.path.insert(0, os.path.abspath('.'))
from tools.auto_decomp.harness import compile_draft, disasm, extract, canonicalise

# Compile original (without mock_include, which uses the single overload header)
ok_orig_pause, _ = compile_draft('source/dol/bases/d_pausewindow.cpp', 'scratch/orig_pausewindow.o')
ok_orig_ctrl, _ = compile_draft('source/d_profileNP/bases/d_controller_information.cpp', 'scratch/orig_ctrl.o')
ok_orig_yn, _ = compile_draft('source/d_profileNP/bases/d_yes_no_window.cpp', 'scratch/orig_yn.o')

disasm('scratch/orig_pausewindow.o', 'scratch/orig_pausewindow.txt')
disasm('scratch/test_pausewindow.o', 'scratch/test_pausewindow.txt')

disasm('scratch/orig_ctrl.o', 'scratch/orig_ctrl.txt')
disasm('scratch/test_controller_info.o', 'scratch/test_controller_info.txt')

disasm('scratch/orig_yn.o', 'scratch/orig_yn.txt')
disasm('scratch/test_yes_no.o', 'scratch/test_yes_no.txt')

with open('scratch/orig_pausewindow.txt') as f1, open('scratch/test_pausewindow.txt') as f2:
    diff_pause = f1.read() == f2.read()
print("d_pausewindow exact disasm match:", diff_pause)

with open('scratch/orig_ctrl.txt') as f1, open('scratch/test_controller_info.txt') as f2:
    diff_ctrl = f1.read() == f2.read()
print("d_controller_information exact disasm match:", diff_ctrl)

with open('scratch/orig_yn.txt') as f1, open('scratch/test_yes_no.txt') as f2:
    diff_yn = f1.read() == f2.read()
print("d_yes_no_window exact disasm match:", diff_yn)
