"""Content-based (verify_anon.py-style) bl-callee sweep of landed REL units.

Retail REL symbol names are almost entirely stripped (fn_2_<OFF> placeholders,
same as the wm_units family), so name-based pairing (sweep_rel.py) checks
almost nothing. This uses wip/wm_units/verify_anon.py's MATCH algorithm
(content-identical-modulo-relocation-symbol pairing, in address order) to get
real pairings, then compares bl targets on each MATCHED pair.
"""
import json
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import verify_anon as V
import harness

DISCACHE = os.path.join(HERE, '_dis_rel')

BL_LINE = re.compile(r'^\s*(bl|bla)\s+"?([^\s,"]+)"?\s*$')
ADDR_SUFFIX = re.compile(r'_[0-9A-Fa-f]{8}$')
PLACEHOLDER = re.compile(r'^(?:fn|func|lbl)_[0-9A-Fa-f]{8}$')


def norm_target(name):
    name = name.strip().strip('"')
    if PLACEHOLDER.match(name):
        return name
    return ADDR_SUFFIX.sub('', name)


def bl_list(ins):
    out = []
    for i, line in enumerate(ins):
        m = BL_LINE.match(line)
        if m:
            out.append((i, norm_target(m.group(2))))
    return out


def rel_text_file_offset(module):
    d = open(os.path.join(ROOT, 'original', module + '.rel'), 'rb').read()
    numSections, sectionInfoOffset = struct.unpack('>II', d[0xC:0x14])
    off, size = struct.unpack('>II', d[sectionInfoOffset + 8:sectionInfoOffset + 16])
    return off & ~3


NAME_ADDR = re.compile(r'_([0-9A-Fa-f]{8})_text\.o$')


def build_rel_index(module):
    objdir = os.path.join(ROOT, 'bin', 'dtkspl', module, 'obj')
    hits = []
    for fn in os.listdir(objdir):
        m = NAME_ADDR.search(fn)
        if fn.endswith('_text.o') and m:
            hits.append((int(m.group(1), 16), os.path.join(objdir, fn)))
    hits.sort()
    return hits


def overlapping_objs(index, lo, hi):
    out = []
    for i, (a, p) in enumerate(index):
        b = index[i + 1][0] if i + 1 < len(index) else a + (1 << 24)
        if a < hi and b > lo:
            out.append(p)
    return out


def sweep_module(module, jsonfile):
    d = json.load(open(os.path.join(ROOT, 'slices', jsonfile), encoding='utf-8'))
    text_file_off = rel_text_file_offset(module)
    idx = build_rel_index(module)

    n_ok = n_no_compiled = n_no_retail = 0
    total_matched = total_target_fns = 0
    bl_checked = bl_mismatch = 0
    findings = []

    for u in d['slices']:
        source = u['source']
        tr = u['memoryRanges'].get('.text')
        if not tr:
            continue
        lo_s, hi_s = tr.split('-')
        lo, hi = text_file_off + int(lo_s, 16), text_file_off + int(hi_s, 16)

        draft_o = os.path.join(ROOT, 'bin', 'compiled', module,
                                os.path.splitext(source)[0] + '.o')
        if not os.path.exists(draft_o):
            n_no_compiled += 1
            continue
        objs = overlapping_objs(idx, lo, hi)
        if not objs:
            n_no_retail += 1
            continue

        target = []
        ok = True
        for obj in objs:
            out = os.path.join(DISCACHE, module + '_' + os.path.basename(obj) + '.txt')
            if not os.path.exists(out) or os.path.getsize(out) == 0:
                dok, _ = harness.disasm(obj, out)
                if not dok:
                    ok = False
                    break
            target += V.functions(out, with_addr=True)
        if not ok:
            n_no_retail += 1
            continue
        target = sorted(x for x in target if lo <= x[0] < hi)

        dout = os.path.join(DISCACHE, module + '_draft_' + os.path.splitext(os.path.basename(source))[0] + '.txt')
        if not os.path.exists(dout) or os.path.getsize(dout) == 0:
            dok, _ = harness.disasm(draft_o, dout)
            if not dok:
                n_no_retail += 1
                continue
        drafts = V.functions(dout)

        n_ok += 1
        total_target_fns += len(target)
        used, last = set(), -1
        for addr, name, ins in target:
            want = V.norm(ins)
            candidates = [i for i, (dn, di) in enumerate(drafts)
                          if i not in used and V.eq_mod_tail_blr(want, V.norm(di))]
            ascending = [i for i in candidates if i > last]
            hit = ascending[0] if ascending else (candidates[0] if candidates else None)
            if hit is None:
                continue
            last = hit
            used.add(hit)
            total_matched += 1
            # compare bl targets on RAW (non-normalised-for-symbols) text
            t_bl = bl_list(ins)
            d_bl = bl_list(drafts[hit][1])
            bl_checked += len(t_bl)
            for (ti, tt), (di, dt) in zip(t_bl, d_bl):
                if tt != dt:
                    bl_mismatch += 1
                    findings.append((source, name, drafts[hit][0], ti, tt, dt))

    return {
        'module': module, 'n_ok': n_ok, 'n_no_compiled': n_no_compiled,
        'n_no_retail': n_no_retail, 'total_target_fns': total_target_fns,
        'total_matched': total_matched, 'bl_checked': bl_checked,
        'bl_mismatch': bl_mismatch, 'findings': findings,
    }


def main():
    results = []
    for module, jsonfile in [
        ('d_basesNP', 'd_basesNP.json'), ('d_enemiesNP', 'd_enemiesNP.json'),
        ('d_profileNP', 'd_profileNP.json'), ('d_en_bossNP', 'd_en_bossNP.json'),
    ]:
        r = sweep_module(module, jsonfile)
        results.append(r)
        print(f"=== {r['module']} ===")
        print(f"units checked {r['n_ok']}; no compiled obj {r['n_no_compiled']}; "
              f"no retail split {r['n_no_retail']}")
        print(f"functions: {r['total_matched']}/{r['total_target_fns']} content-matched "
              f"(verify_anon-style)")
        print(f"BL: {r['bl_checked']} checked, {r['bl_mismatch']} mismatched")
        for source, tname, dname, ti, tt, dt in r['findings']:
            print(f"  BL MISMATCH: {source}  retail-fn {tname} <- draft {dname}  "
                  f"instr {ti}  retail calls {tt!r}  draft calls {dt!r}")
        print()

    with open(os.path.join(HERE, 'rel_report2.json'), 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2)


if __name__ == '__main__':
    main()
