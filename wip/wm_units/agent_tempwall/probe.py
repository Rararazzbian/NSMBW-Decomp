"""Minimal synthetic-probe harness for the by-value-temporary slot-assignment
wall (see AGENT prompt). Compiles wip/wm_units/agent_tempwall/draft.cpp with
the SAME flags as a real d_basesNP unit (-O4,p -sdata 0 -sdata2 0 -char signed),
disassembles it, and prints the raw instruction listing for a chosen function
so the stack-slot immediates can be read off by eye.

Usage: python wip/wm_units/agent_tempwall/probe.py [fn_name]
"""
import sys, os, re
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H

HERE = 'wip/wm_units/agent_tempwall'
SRC = os.path.join(HERE, 'draft.cpp')
OBJ = os.path.join(HERE, 'draft.o')
TXT = os.path.join(HERE, 'draft.txt')

def run(fn=None):
    ok, log = H.compile_draft(SRC, OBJ, module='d_basesNP')
    if not ok:
        print('COMPILE FAILED')
        print(log[-6000:])
        sys.exit(2)
    H.disasm(OBJ, TXT)
    with open(TXT, encoding='utf-8', errors='replace') as f:
        text = f.read()
    if fn is None:
        print(text)
        return
    m = re.search(r'^\.fn\s+"?' + re.escape(fn) + r'"?\s*,.*?$(.*?)^\.endfn',
                  text, re.M | re.S)
    if not m:
        print('FUNCTION NOT FOUND:', fn)
        print('Functions present:')
        for fm in re.finditer(r'^\.fn\s+"?(.+?)"?\s*,', text, re.M):
            print('  ', fm.group(1))
        sys.exit(3)
    print(m.group(0))

if __name__ == '__main__':
    run(sys.argv[1] if len(sys.argv) > 1 else None)
