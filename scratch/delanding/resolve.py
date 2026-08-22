import subprocess, sys, os, re

known = set()
for f in ('syms.txt', 'alias_db.txt'):
    if not os.path.exists(f):
        continue
    for line in open(f, encoding='utf-8', errors='replace'):
        for m in re.findall(r'[A-Za-z_$@][A-Za-z0-9_$@.]*', line):
            known.add(m)

for path in sys.argv[1:]:
    out = subprocess.run([sys.executable, 'scratch/delanding/undef.py', path],
                         capture_output=True, text=True).stdout
    undef = [l.split()[-1] for l in out.splitlines() if l.startswith('   ')]
    miss = [u for u in undef if u not in known]
    print('%-32s undefined=%-4d NOT in syms.txt/alias_db=%d' %
          (os.path.basename(path), len(undef), len(miss)))
    for m in sorted(miss)[:12]:
        print('      MISSING  %s' % m)
    if len(miss) > 12:
        print('      ... and %d more' % (len(miss) - 12))
