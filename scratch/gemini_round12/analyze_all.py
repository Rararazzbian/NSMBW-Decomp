import os
import sys
import json
import re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

def main():
    print("=== Analyzing d_basesNP and d_en_bossNP ===")
    
    # 1. Load slices/d_basesNP.json
    with open(os.path.join(ROOT, 'slices', 'd_basesNP.json')) as f:
        bases_slices = json.load(f)
    
    print(f"Landed slices in d_basesNP.json: {len(bases_slices['slices'])}")
    for s in bases_slices['slices']:
        print(f"  {s['source']}: {s['memoryRanges']}")

    # 2. Load alias_db.txt
    alias_db = {}
    with open(os.path.join(ROOT, 'alias_db.txt')) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' in line:
                k, v = line.split('=', 1)
                alias_db[k.strip()] = v.strip()
                
    print(f"Total aliases in alias_db.txt: {len(alias_db)}")

    # 3. Load syms.txt
    syms_txt = {}
    with open(os.path.join(ROOT, 'syms.txt')) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' in line:
                k, v = line.split('=', 1)
                syms_txt[k.strip()] = int(v.strip(), 16)
    print(f"Total symbols in syms.txt: {len(syms_txt)}")

if __name__ == '__main__':
    main()
