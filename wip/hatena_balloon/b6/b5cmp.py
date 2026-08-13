"""Exact-name function extractor + differ for batch B5.

harness.extract() CANNOT be used on this TU's unnamed functions: harness.norm_name
strips a trailing _<8 hex> suffix, so `fn_800221E0` -> `fn`, and every fn_* target
collapses to the same key. extract() then returns the FIRST such function in the
file for every query, and diff_fn reports all of them as identical.

This module reuses harness's canonicalise()/INSN/INSN_WORD/LOCAL_BRANCH (which are
correct) but matches the .fn name EXACTLY.
"""
import os
import sys
import difflib

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402


# Target symbol -> the name the draft emits. Applied to the RAW target line before
# harness.canonicalise() runs, because canonicalise turns `fn_80022810` into a bare
# `fn` (ADDR_SUFFIX_INLINE) and `lbl_802EE608` into a positional SYM (POOL_SYM),
# neither of which the draft's real names can ever equal.
RENAME = {
    'fn_800221E0': 'multi_item_mode_set__FP15daEnBlockMain_cUi',
    'fn_80022600': 'block_item_create__FP15daEnBlockMain_cP14sBlockItemInfo',
    'fn_800226D0': 'block_item_create_sub__FP15daEnBlockMain_cP14sBlockItemInfo',
    'fn_80022780': 'block_multi_item_create__FP15daEnBlockMain_cP14sBlockItemInfoiUi',
    'fn_80022810': 'block_item_set__FP15daEnBlockMain_cP14sBlockItemInfoi',
    'fn_80022B10': 'block_multi_item_set__FP15daEnBlockMain_cP14sBlockItemInfoiUi',
    'lbl_802EE608': 'l_item_create_tbl',
    'lbl_802EE620': 'l_item_no_tbl',
    'lbl_802EE668': 'l_item_no_tbl_ex',
}


def _rename(s, rename):
    for k, v in rename.items():
        s = s.replace(k, v)
    return s


def extract_exact(path, name, rename=None):
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = H.FN_START.match(s)
            if m:
                body = [] if m.group(1).strip().strip('"') == name else None
                continue
            if H.FN_END.match(s):
                if body is not None:
                    return H.canonicalise(body) if body else []
                continue
            if body is not None:
                # Rename only INSIDE a body: the `.fn` header must keep its real name.
                if rename:
                    s = _rename(s, rename)
                mw = H.INSN_WORD.match(s)
                if mw and H.LOCAL_BRANCH.search(mw.group(2)):
                    word = ''.join(mw.group(1).split())
                    body.append('%s |%s|' % (mw.group(2).strip(), word))
                    continue
                mi = H.INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())
    return body if body is None else (H.canonicalise(body) if body else [])


def names(path):
    out = []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = H.FN_START.match(line.strip())
            if m:
                out.append(m.group(1).strip().strip('"'))
    return out


def diff(tgt_path, tgt_name, drf_path, drf_name, ctx=3):
    a = extract_exact(tgt_path, tgt_name, RENAME)
    b = extract_exact(drf_path, drf_name)
    if a is None:
        return False, 'TARGET MISSING: %s' % tgt_name
    if b is None:
        return False, 'DRAFT MISSING: %s' % drf_name
    if a == b:
        return True, 'MATCH (%d insns)' % len(a)
    rep = ['MISMATCH target=%d draft=%d insns' % (len(a), len(b))]
    rep += list(difflib.unified_diff(a, b, 'target/' + tgt_name,
                                     'draft/' + drf_name, lineterm='', n=ctx))
    return False, '\n'.join(rep)
