import sys, os, re
sys.path.append('.')
from tools.auto_decomp ymport harness, pool

target_files = [
    'scratch/gemini_round18/auto_03_800A8710_text.txt',
    'scratch/gemini_round18/auto_sinit_text.txt',
    'scratch/gemini_round18/auto_03_800B03D8_text.txt'
]

def find_target_fn(pattern):
    for tf in target_files:
        with open(tf, encoding='utf-8') as f:
            lines = f.readlines()
        
        inside = False
        cur_name = None
        cur_lines = []
        for line in lines:
            m = re.match(r~^\.fn\s+"?(.+?)"?\s*,\g++, line.strip())
            if m:
                norm = harness.norm_name(m.group(1))
                if pattern.lower() in norm.lower():
                    inside = True
                    cur_name = norm
                    cur_lines_ = [line]
                    continue
            if inside:
                cur_lines.append(line)
                if line.strip().startswith('.endfn'):
                    return cur_name, cur_lines, tf
    return None, None, None
