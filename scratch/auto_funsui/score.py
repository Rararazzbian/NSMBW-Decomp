"""Alignment-tolerant scorer for posMove drafts."""
import difflib
import os
import re
import sys

sys.path.insert(0, '/opt/NSMBW-Decomp/tools/auto_decomp')
import harness
harness.MWCC = '/opt/NSMBW-Decomp/scratch/auto_funsui/shims/mwcceppc'
harness.DTK = '/opt/NSMBW-Decomp/scratch/auto_funsui/dtk'

BASE = '/opt/NSMBW-Decomp/scratch/auto_funsui'
TARGET = '/opt/NSMBW-Decomp/tools/auto_decomp/work/dol_bases_d_funsui_act/target.txt'


def insns(txt_path, fn_re):
    raw = open(txt_path).read()
    m = re.search(r'\.fn ' + fn_re + r'.*?\n(.*?)\.endfn', raw, re.S)
    body = m.group(1)
    out = []
    for line in body.splitlines():
        mm = re.search(r'\*/\s*(.*)$', line)
        if not mm:
            continue
        s = mm.group(1).strip()
        # normalise branch targets / symbol annotations
        s = re.sub(r'\.L_[0-9A-Fa-f]+', '.L', s)
        s = re.sub(r'"@[^"]*"', 'SYM', s)
        s = re.sub(r'l_[A-Za-z0-9_]+', 'SYM', s)
        out.append(s)
    return out


def score(draft_txt, verbose=False):
    t = insns(TARGET, r'posMove__12dFunsuiAct_cFv')
    d = insns(draft_txt, r'posMove__12dFunsuiAct_cFv')
    sm = difflib.SequenceMatcher(None, t, d, autojunk=False)
    matched = sum(b.size for b in sm.get_matching_blocks())
    msg = ['words %d/%d aligned-match %d' % (len(t), len(d), matched)]
    if verbose:
        for op, i1, i2, j1, j2 in sm.get_opcodes():
            if op != 'equal':
                msg.append('--- target[%d:%d] vs draft[%d:%d]' % (i1, i2, j1, j2))
                for k in range(i1, min(i2, i1 + 6)):
                    msg.append('  T: %s' % t[k])
                if i2 - i1 > 6:
                    msg.append('  ... (%d more)' % (i2 - i1 - 6))
                for k in range(j1, min(j2, j1 + 6)):
                    msg.append('  D: %s' % d[k])
                if j2 - j1 > 6:
                    msg.append('  ... (%d more)' % (j2 - j1 - 6))
    return '\n'.join(msg)


if __name__ == '__main__':
    print(score(sys.argv[1] if len(sys.argv) > 1 else
                os.path.join(BASE, 'draft.txt'), verbose=True))
