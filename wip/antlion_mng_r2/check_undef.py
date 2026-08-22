"""List every external symbol the draft references and check it exists in a target symbol map.

None of the four standard checks catches an unresolved symbol; this does.
"""
import re, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))

txt = open(os.path.join(HERE, 'draft.txt'), encoding='utf-8', errors='replace').read()
defined = set(re.findall(r'^\.fn (\S+?),', txt, re.M)) | set(re.findall(r'^\.obj (\S+?),', txt, re.M))

NAME = '([A-Za-z_$][A-Za-z0-9_$@' + chr(92) + chr(92) + ']*)'
refs = set()
for m in re.finditer(NAME + r'@(?:ha|l|sda21)\b', txt):
    refs.add(m.group(1))
for m in re.finditer(r'\bbl\s+' + NAME, txt):
    refs.add(m.group(1))
for m in re.finditer(r'^\s*b\s+' + NAME + r'\s*$', txt, re.M):
    refs.add(m.group(1))

ext = sorted(r for r in refs if r not in defined and not r.startswith('@')
             and not r.startswith('.') and not r.startswith('_savegpr')
             and not r.startswith('_restgpr'))

syms = open(os.path.join(ROOT, 'bin/dtk/d_basesNP_symbols.txt'), encoding='utf-8', errors='replace').read()
dol = open(os.path.join(ROOT, 'bin/dtk/wiimj2d_symbols.txt'), encoding='utf-8', errors='replace').read()

missing = []
for e in ext:
    pat = r'(?m)^' + re.escape(e) + r'\s*='
    if re.search(pat, syms) or re.search(pat, dol):
        continue
    missing.append(e)

print('externals referenced: %d' % len(ext))
if missing:
    print('NOT FOUND in d_basesNP or wiimj2d symbol maps (%d):' % len(missing))
    for m in missing:
        print('   ', m)
else:
    print('ALL EXTERNALS RESOLVE against the target symbol maps.')
