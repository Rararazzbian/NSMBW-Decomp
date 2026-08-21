import re

# --- retail side: parse the get_target plain disasm dump ---
retail_text = open('wip/kokoopa_verify/sinit_retail.txt', encoding='utf-8').read()
idx = retail_text.find('DISASSEMBLY')
body = retail_text[idx:]
retail_lines = []
LINE_RE = re.compile(r'^\s*([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s+(.*)$')
for line in body.splitlines():
    m = LINE_RE.match(line)
    if not m:
        continue
    addr = int(m.group(1), 16)
    if 0x800AED40 <= addr < 0x800B03D8:
        rest = m.group(3).strip()
        retail_lines.append((addr, rest))

print('retail __sinit instruction lines:', len(retail_lines))

# --- draft side: parse draft.txt's own __sinit function body (raw, dtk format) ---
draft_text = open('wip/kokoopa_verify/draft.txt', encoding='utf-8').read()
lines = draft_text.splitlines()
in_fn = False
draft_lines = []
FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')
for line in lines:
    s = line.strip()
    m = FN_START.match(s)
    if m:
        in_fn = ('sinit' in m.group(1).lower())
        continue
    if s.startswith('.endfn'):
        in_fn = False
        continue
    if in_fn:
        mi = INSN.match(s)
        if mi:
            draft_lines.append(mi.group(1).strip())

print('draft __sinit instruction lines:', len(draft_lines))

# --- normalize both sides to (mnemonic, operands-without-hex-immediate-in-lis/addi, trailing comment) ---
def norm(s):
    # split off a trailing "# comment" or "-> symbol" style annotation
    s = s.strip()
    comment = ''
    if '#' in s:
        s, comment = s.split('#', 1)
        comment = comment.strip()
    s = s.strip()
    return s, comment

retail_norm = [norm(r[1]) for r in retail_lines]
draft_norm = [norm(d) for d in draft_lines]

print('\n--- length compare ---')
print('retail:', len(retail_norm), ' draft:', len(draft_norm))

mismatches = 0
max_show = 400
for i in range(max(len(retail_norm), len(draft_norm))):
    r = retail_norm[i] if i < len(retail_norm) else None
    d = draft_norm[i] if i < len(draft_norm) else None
    if r is None or d is None:
        print(i, 'MISSING', 'retail=', r, 'draft=', d)
        mismatches += 1
        continue
    rmnem = r[0].split(',')[0].split()[0] if r[0] else ''
    dmnem = d[0].split(',')[0].split()[0] if d[0] else ''
    # compare mnemonic always; compare comment (symbol ref) when either side has one
    if rmnem != dmnem:
        if mismatches < max_show:
            print(i, 'MNEM DIFF', 'retail=', r, 'draft=', d)
        mismatches += 1
        continue
    if (r[1] or d[1]) and r[1] != d[1]:
        # allow addr-suffix differences but flag genuine symbol differences
        if mismatches < max_show:
            print(i, 'COMMENT DIFF', 'retail=', r, 'draft=', d)
        mismatches += 1

print('\ntotal mismatches:', mismatches, 'of', max(len(retail_norm), len(draft_norm)))
