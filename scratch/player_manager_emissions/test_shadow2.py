"""Test declare-without-body for daPlBase_c::executeLastPlayer/executeLastAll
and dAcPy_c::isItemKinopio, without touching real include/ headers.
"""
import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import harness as H

SHADOW = os.path.join(ROOT, 'scratch', 'player_manager_emissions', 'shadow_include2')
H.INCLUDES = [SHADOW] + H.INCLUDES

import unit_verify as UV
UV.LO = 0x8005E9A0
UV.HI = 0x800613B0
UV.UNIT = 'dol_bases_d_a_player_manager'
UV.TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', UV.UNIT, 'target.txt')

sys.argv = ['unit_verify.py', os.path.join(ROOT, 'wip', 'player_manager', 'assembled.cpp')]
sys.exit(UV.main())
