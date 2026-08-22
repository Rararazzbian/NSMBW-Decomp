import json, glob, re, os
ninja = open('build.ninja', encoding='utf-8', errors='replace').read()
allsrc = set()
for f in sorted(glob.glob('slices/*.json')):
    d = json.load(open(f))
    srcs = [s['source'] for s in d['slices']]
    allsrc |= set(srcs)
    missing = [s for s in srcs if os.path.basename(s) not in ninja]
    print("%s: %d slices, %d NOT in build.ninja %s" % (f, len(srcs), len(missing), missing))
comp = set(re.findall(r'source[/\\]([A-Za-z0-9_/\\.]+\.(?:cpp|c))', ninja))
comp = set(c.replace(chr(92), '/') for c in comp)
extra = sorted(c for c in comp if c not in allsrc)
print("compiled by ninja but not in any slice file: %d" % len(extra))
for e in extra[:20]:
    print("   ", e)
