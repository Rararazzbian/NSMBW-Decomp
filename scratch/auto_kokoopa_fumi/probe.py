#!/usr/bin/env python3
"""Build one variant, score it, and print the operate register map.

    python3 probe.py s1            # builds s1.cpp -> s1.o/s1.txt
"""
import re
import subprocess
import sys

FN = 'operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c'


def perm(txt_path):
    txt = open(txt_path).read()
    m = re.search(r'\.fn ' + FN + r'.*?(?=\.fn |\Z)', txt, re.S)
    if not m:
        return 'NO-FN'
    body = m.group(0)
    o = re.search(r'mr\s+r(\d+),\s*r4\b', body)
    e = re.search(r'mr\s+r(\d+),\s*r5\b', body)
    p = re.search(r'lwz\s+r(\d+),\s*0x4\(r3\)', body)
    w = len(re.findall(r'/\*\s*[0-9A-F]{8}', body))
    return 'out=r%s en=r%s pl=r%s (%dw)' % (
        o.group(1) if o else '-', e.group(1) if e else '-',
        p.group(1) if p else '-', w)


def main():
    name = sys.argv[1]
    r = subprocess.run([sys.executable, 'build.py', name + '.cpp', name],
                       capture_output=True, text=True)
    out = r.stdout.strip()
    if r.returncode != 0 or not out:
        print('%-6s BUILD-FAIL: %s' % (name, (r.stdout + r.stderr)[-400:]))
        return 1
    d = subprocess.run([sys.executable,
                        '/opt/NSMBW-Decomp/tools/auto_decomp/fndiff.py',
                        '/opt/NSMBW-Decomp/tools/auto_decomp/work/'
                        'dol_bases_d_kokoopa_sp_fumi_check/target.txt',
                        name + '.txt', '--all'],
                       capture_output=True, text=True)
    lines = [l for l in d.stdout.splitlines() if l.strip()]
    score = ' | '.join(lines) if lines else d.stdout[-200:]
    print('%-6s %s' % (name, perm(name + '.txt')))
    for l in lines:
        print('       %s' % l)
    return 0


if __name__ == '__main__':
    sys.exit(main())
