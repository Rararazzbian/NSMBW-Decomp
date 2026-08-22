"""Value-check every LANDED REL unit's pooled constants against retail.

For each slice entry in slices/d_*NP.json that has a `.text` range:
  * our side    -- the object the build already produced, bin/compiled/<mod>/...
  * retail side -- the dtk split objects covering that `.text` range,
                   bin/dtkspl/<mod>/obj/auto_00_<start>_text.o
Functions are paired by CONTENT (modulo symbol names), exactly as
wip/wm_units/verify_anon.py does, because every retail REL function is an
anonymous fn_2_*.
"""
import json
import os
import re
import sys
import glob

ROOT = os.path.abspath('.')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness
import poolcheck as PC

CACHE = os.path.join(ROOT, 'scratch', 'poolcheck_rel', '_dis')
os.makedirs(CACHE, exist_ok=True)


def dis(obj):
    key = obj.replace(os.sep, '#').replace('/', '#').replace(':', '')
    out = os.path.join(CACHE, key + '.txt')
    if not os.path.exists(out) or os.path.getsize(out) == 0:
        ok, log = harness.disasm(obj, out)
        if not ok:
            return None
    return out


FN = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
END = re.compile(r'^\.endfn')
INS = re.compile(r'/\* ([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)')


def parse(path, lo=None, hi=None):
    """{name: [(bytes, text), ...]} optionally restricted to [lo, hi)."""
    fns, cur, body, addr = {}, None, None, None
    for line in open(path, encoding='utf-8', errors='replace'):
        s = line.strip()
        m = FN.match(s)
        if m:
            cur, body, addr = m.group(1), [], None
            continue
        if END.match(s):
            if cur and body and (lo is None or (lo <= addr < hi)):
                fns[cur] = body
            cur = None
            continue
        if cur is not None:
            mi = INS.search(line)
            if mi:
                if addr is None:
                    addr = int(mi.group(1), 16)
                body.append((mi.group(2).strip(), mi.group(3).strip()))
    return fns


def split_objs(mod, lo, hi):
    objs = glob.glob(os.path.join(ROOT, 'bin', 'dtkspl', mod, 'obj', 'auto_*_text.o'))
    starts = []
    for o in objs:
        m = re.search(r'auto_\d+_([0-9A-Fa-f]{8})_text\.o$', o.replace(os.sep, '/'))
        if m:
            starts.append((int(m.group(1), 16), o))
    starts.sort()
    hits = []
    for k, (a, o) in enumerate(starts):
        b = starts[k + 1][0] if k + 1 < len(starts) else 1 << 32
        if a < hi and b > lo:
            hits.append(o)
    return hits


def main():
    grand = {'units': 0, 'skipped': 0, 'compared': 0, 'mismatched': 0,
             'unresolved': 0, 'pairs': 0, 'loads': 0, 'name_equal': 0}
    for sl in sorted(glob.glob(os.path.join(ROOT, 'slices', 'd_*NP.json'))):
        mod = os.path.basename(sl)[:-5]
        print('\n##### %s #####' % mod)
        j = json.load(open(sl, encoding='utf-8'))
        for entry in j['slices']:
            src = entry['source']
            rng = entry.get('memoryRanges', {}).get('.text')
            if not rng:
                print('%-46s SKIP  no .text range' % src)
                grand['skipped'] += 1
                continue
            lo, hi = (int(x, 16) for x in rng.split('-'))
            obj = os.path.join(ROOT, 'bin', 'compiled', mod,
                               os.path.splitext(src)[0].replace('/', os.sep) + '.o')
            if not os.path.exists(obj):
                print('%-46s SKIP  no built object' % src)
                grand['skipped'] += 1
                continue
            dtxt = dis(obj)
            if not dtxt:
                print('%-46s SKIP  disasm failed' % src)
                grand['skipped'] += 1
                continue
            target = {}
            for t in split_objs(mod, lo, hi):
                tt = dis(t)
                if tt:
                    target.update(parse(tt, lo, hi))
            draft = parse(dtxt)
            if not target:
                print('%-46s SKIP  no retail functions in 0x%X-0x%X' % (src, lo, hi))
                grand['skipped'] += 1
                continue

            objspace = PC.ObjectSpace(obj)
            tres, dres = PC.retail_resolver(), PC.draft_resolver(objspace)
            res = PC.Result()
            res.target_fns, res.draft_fns = len(target), len(draft)
            pairs = PC.pair_functions(target, draft)
            for tn, dn in pairs:
                t, d = target.get(tn), draft.get(dn)
                if t is None or d is None or len(t) != len(d):
                    res.skipped_len += 1
                    continue
                res.pairs += 1
                gate = ([b for b, _ in t] == [b for b, _ in d]
                        or harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in d]))
                res.examined += 1
                PC.compare_refs(tn, t, d, tres, dres, gate, res)

            grand['units'] += 1
            grand['compared'] += res.compared
            grand['mismatched'] += len(res.mismatched)
            grand['unresolved'] += len(res.unresolved)
            grand['pairs'] += res.pairs
            grand['loads'] += res.float_loads
            grand['name_equal'] += res.name_equal
            flag = ''
            if res.mismatched:
                flag = '  <<<< WRONG CONSTANT'
            elif res.examined and res.compared == 0 and res.float_loads:
                flag = '  <<<< CHECKED NOTHING'
            print('%-46s %3d/%-3d fns  %4d cmp  %2d mism  %2d unres  %4d loads%s'
                  % (src, res.pairs, len(target), res.compared, len(res.mismatched),
                     len(res.unresolved), res.float_loads, flag))
            for nm, i, t, d, gm, tb, db in res.mismatched:
                print('      MISMATCH %s @%d  retail %s=%s  draft %s=%s'
                      % (nm, i, t.loc, tb.hex().upper(), d.loc, db.hex().upper()))
            for nm, i, why, t, d in res.unresolved:
                print('      UNRESOLVED %s @%d  %s' % (nm, i, why))
                print('          retail: %s' % (t.describe() if t else '(none)'))
                print('          draft : %s' % (d.describe() if d else '(none)'))
    print('\n' + '=' * 78)
    print('TOTAL: %(units)d units, %(pairs)d paired functions, %(loads)d float loads,\n'
          '       %(compared)d constants compared, %(name_equal)d same-symbol skips,\n'
          '       %(mismatched)d MISMATCHED, %(unresolved)d UNRESOLVED, '
          '%(skipped)d units skipped' % grand)


main()
