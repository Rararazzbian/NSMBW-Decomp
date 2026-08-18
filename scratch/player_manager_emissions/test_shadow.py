"""Test the shadow-header fix for the mVec2_c/EGG::Vector2f/Vector3f
destructor flush, without touching real include/ headers.

Prepends scratch/player_manager_emissions/shadow_include to the mwcc -i
search path so it wins over the real headers, then runs the exact same
comparison unit_verify.py does.
"""
import os
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import harness as H

SHADOW = os.path.join(ROOT, 'scratch', 'player_manager_emissions', 'shadow_include')
H.INCLUDES = [SHADOW] + H.INCLUDES

import unit_verify as UV
UV.LO = 0x8005E9A0
UV.HI = 0x800613B0
UV.UNIT = 'dol_bases_d_a_player_manager'
UV.TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work', UV.UNIT, 'target.txt')

sys.argv = ['unit_verify.py', os.path.join(ROOT, 'wip', 'player_manager', 'assembled.cpp')]
sys.exit(UV.main())
