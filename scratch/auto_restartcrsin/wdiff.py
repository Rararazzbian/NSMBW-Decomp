#!/usr/bin/env python3
"""Word-level byte diff of one function between two disasm listings."""
import re
import sys


def load(txt, fn):
    lines = open(txt).read().splitlines()
    start = None
    out = []
    for i, ln in enumerate(lines):
        m = re.match(r'\.fn\s+' + re.escape(fn) + r',', ln)
        if m:
            start = True
            continue
        if start:
            if ln.startswith('.endfn'):
                break
            m2 = re.search(r'/\*\s*[0-9A-Fa-f]{8} [0-9A-Fa-f]{8}\s+((?:[0-9A-Fa-f]{2} ){3}[0-9A-Fa-f]{2}) \*/', ln)
            if m2:
                hexs = m2.group(1).replace(' ', '')
                out.append((ln, hexs))
    return out


def main():
    tgt, drf, fn = sys.argv[1], sys.argv[2], sys.argv[3]
    t = load(tgt, fn)
    d = load(drf, fn)
    print(f"target {len(t)} words, draft {len(d)} words")
    n = min(len(t), len(d))
    ndiff = 0
    for i in range(n):
        tl, th = t[i]
        dl, dh = d[i]
        if th != dh:
            ndiff += 1
            # strip the leading addr/offset comment; show mnemonic part
            tm = re.sub(r'^/\*\s*\S+ \S+\s+[0-9A-F ]{12} \*/\t', '', tl)
            dm = re.sub(r'^/\*\s*\S+ \S+\s+[0-9A-F ]{12} \*/\t', '', dl)
            print(f"[{i:3d}] {th} | {dh}   TGT: {tm:<40s} DRF: {dm}")
    if len(t) != len(d):
        print("LENGTH MISMATCH")
        for i in range(n, max(len(t), len(d))):
            tl = t[i][0] if i < len(t) else '-'
            dl = d[i][0] if i < len(d) else '-'
            tm = re.sub(r'^/\*\s*\S+ \S+\s+[0-9A-F ]{12} \*/\t', '', tl)
            dm = re.sub(r'^/\*\s*\S+ \S+\s+[0-9A-F ]{12} \*/\t', '', dl)
            print(f"[{i:3d}] {'TGT: '+tm if i < len(t) else ''}  {'DRF: '+dm if i < len(d) else ''}")
    print(f"total differing words: {ndiff}")


main()
