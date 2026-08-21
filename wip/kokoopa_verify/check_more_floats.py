import sys, re
sys.path.insert(0, 'tools/auto_decomp')
import harness

target_path = 'wip/kokoopa_verify/target_combined.txt'
FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')

def raw_extract(path, name):
    want = harness.norm_name(name)
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                body = [] if harness.norm_name(m.group(1)) == want else None
                continue
            if s.startswith('.endfn'):
                if body is not None:
                    return body
                continue
            if body is not None:
                mi = INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())
    return body

for name in ['getShellChangeEffectOffsetY__18dEnTorideKokoopa_cCFv',
             'getMagicStickEffectOffset__18dEnTorideKokoopa_cCFv']:
    t = raw_extract(target_path, name)
    print('===', name, '===')
    for l in t:
        print(' ', l)
    print()
