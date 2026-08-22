import sys, os, re, importlib.util

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
VA_PATH = os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py')
spec = importlib.util.spec_from_file_location("va", VA_PATH)
va = importlib.util.module_from_spec(spec)
spec.loader.exec_module(va)

draft_path, target_addr_hex, obj, draft_name = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
target_addr = int(target_addr_hex, 0)

cache = os.path.join(ROOT, 'wip', 'wm_units', '_dis')
out = os.path.join(cache, os.path.basename(obj) + '.txt')
if not os.path.exists(out) or os.path.getsize(out) == 0:
    va.H.disasm(obj, out)
targets = va.functions(out, with_addr=True)
tfn = None
for addr, name, ins in targets:
    if addr == target_addr:
        tfn = (name, ins)
        break
if tfn is None:
    print("target not found at", hex(target_addr))
    sys.exit(1)

drafts = va.functions(draft_path)
dfn = None
for name, ins in drafts:
    if name == draft_name:
        dfn = (name, ins)
        break
if dfn is None:
    print("draft fn not found:", draft_name)
    sys.exit(1)

tn = va.norm(tfn[1])
dn = va.norm(dfn[1])
print("TARGET:", tfn[0], len(tn), " DRAFT:", dfn[0], len(dn))
maxlen = max(len(tn), len(dn))
for i in range(maxlen):
    t = tn[i] if i < len(tn) else '<none>'
    d = dn[i] if i < len(dn) else '<none>'
    mark = '   ' if t == d else ' * '
    print('%3d%s T: %-45s D: %-45s' % (i, mark, t, d))
