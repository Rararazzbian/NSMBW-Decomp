import re

pat = re.compile(
    r'^(\S+)\s*=\s*(\S+):0x([0-9A-Fa-f]+);\s*//\s*type:(\S+)'
    r'(?:\s+size:(0x[0-9A-Fa-f]+))?(?:\s+scope:(\S+))?(?:\s+data:(\S+))?'
)

with open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtk\wiimj2d_symbols.txt',
          encoding='utf-8', errors='replace') as fh:
    for ln in fh:
        m = pat.match(ln.strip())
        if not m:
            continue
        sec = m.group(2)
        if sec in ('.ctors', '.dtors', '.ctors$10'):
            print(ln.rstrip())
