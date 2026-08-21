import sys, os, re
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness
import pool

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
WIP = os.path.join(ROOT, 'wip', 'kokoopa_verify2')

TARGET_FILES = {
    'target_1_800A8710.txt': os.path.join(WIP, 'target_1_800A8710.txt'),
    'target_2_sinit.txt': os.path.join(WIP, 'target_2_sinit.txt'),
    'target_3_800B03D8.txt': os.path.join(WIP, 'target_3_800B03D8.txt'),
}

# rebuild name->srcfile map from true_target_list.txt
name_to_file = {}
with open(os.path.join(WIP, 'true_target_list.txt'), encoding='utf-8') as fh:
    for line in fh:
        addr, size, name, srcfile = line.rstrip('\n').split('\t')
        name_to_file[name] = srcfile

matched = []
with open(os.path.join(WIP, 'matched_list.txt'), encoding='utf-8') as fh:
    for line in fh:
        addr, size, name = line.rstrip('\n').split('\t')
        matched.append((int(addr,16), int(size), name))

POOLREF = re.compile(r'"?(@(\d+)_([0-9A-Fa-f]{8}))"?')

def raw_extract(path, name):
    """Extract raw (non-canonicalised) instruction lines for a function by exact name."""
    want = name
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = harness.FN_START.match(s)
            if m:
                body = [] if m.group(1).strip('"') == want else None
                continue
            if harness.FN_END.match(s):
                if body is not None:
                    return body
                continue
            if body is not None:
                mi = harness.INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())
    return body

# sort matched by size ascending (small first, per instructions) but put sound/voice/effect handlers first regardless
SOUND_VOICE_NAMES = set("""jumpSE landonSE shellinSE shellatkSE shelllandonSE shelloutSE blitzchargeSE getupSE awakeSE ikakuSE
notice1Vo notice2Vo wakeVo escJumpVo magicShotVo shellOutVo deadVo loseFirstVo loseSecondVo damageSVo damageLVo""".split())
EFFECT_NAMES = set("""jumpEffect landonEffect jumpRootEffect downFallEffect hitFireLoopEffect hitFireDamageEffect shellChangeEffect
fumidmgEffect fumideadEffect shellLandonEffect downLandOnEffect hitShellDamageEffect ikakuEffect""".split())

def base_name(mangled):
    return mangled.split('__', 1)[0]

results = []
for addr, size, name in matched:
    srcfile = name_to_file.get(name)
    if not srcfile:
        continue
    path = TARGET_FILES[srcfile]
    body = raw_extract(path, name)
    if body is None:
        print('WARNING: could not raw-extract', name)
        continue
    refs = []
    for line in body:
        for m in POOLREF.finditer(line):
            refs.append((m.group(1), int(m.group(3), 16)))
    if refs:
        results.append((addr, size, name, refs))

# priority: sound/voice + effect handlers first, then ascending size
def sortkey(item):
    addr, size, name, refs = item
    bn = base_name(name)
    pri = 0 if (bn in SOUND_VOICE_NAMES or bn in EFFECT_NAMES) else 1
    return (pri, size)

results.sort(key=sortkey)

print('Matched functions with pooled literal references: %d of %d matched\n' % (len(results), len(matched)))

for addr, size, name, refs in results:
    decoded = []
    for sym, va in refs:
        r = pool.read(va)
        if r:
            decoded.append('%s -> f32=%r' % (sym, r['f32']))
        else:
            decoded.append('%s -> ???' % sym)
    print('0x%08X %5d %-70s %s' % (addr, size, name, ' | '.join(decoded)))
