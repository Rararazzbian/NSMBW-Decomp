import os
import re
import sys

# Let's inspect non-text sections for the candidate ranges
with open('bin/dtk/dtk_splits_wiimj2d.txt', 'r', encoding='utf-8') as f:
    splits_text = f.read()

# For d_nand_thread:
# Find what sections are between d_multi_manager and d_next
def get_sec_between(tu1, tu2, sec_name):
    # Search in splits_text for tu1 and tu2 section ranges
    # dtk_splits has them in order
    pass

# Let's print the full split entries for the top candidates' neighbours
targets = [
    ('dol/bases/d_multi_manager.cpp', 'dol/bases/d_next.cpp', 'dol/bases/d_nand_thread.cpp'),
    ('dol/bases/d_last_actor.cpp', 'dol/bases/d_a_player.cpp', 'dol/bases/d_a_mask.cpp'),
    ('dol/mLib/m_mtx.cpp', 'dol/mLib/m_vec.cpp', 'dol/mLib/m_pad.cpp'),
    ('dol/bases/d_wm_actor.cpp', 'dol/bases/d_wm_csvdata.cpp', 'dol/bases/d_wm_connect.cpp'),
    ('dol/bases/d_s_boot.cpp', 'dol/bases/d_scene.cpp', 'dol/bases/d_WarningManager.cpp'),
]

for left_tu, right_tu, candidate in targets:
    print("=" * 60)
    print(f"Candidate: {candidate}")
    print(f"Left TU:   {left_tu}")
    print(f"Right TU:  {right_tu}")
    
    # Extract left_tu sections and right_tu sections
    def extract_tu_sec(tu):
        m = re.search(rf'{re.escape(tu)}:\n((?:\t[^\n]+\n)+)', splits_text)
        return m.group(1) if m else "NOT FOUND"
    
    print("Left TU sections:")
    print(extract_tu_sec(left_tu))
    print("Right TU sections:")
    print(extract_tu_sec(right_tu))
