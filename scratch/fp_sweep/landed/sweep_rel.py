"""bl-callee sweep of every LANDED REL unit (d_basesNP/d_enemiesNP/d_profileNP/
d_en_bossNP), using bin/dtkspl/<module>/obj retail split objects and
bin/compiled/<module>/... draft objects.

Pool-constant checking is NOT generalised here (see report): RELs are compiled
with `-sdata 0 -sdata2 0`, so `@sda21` addressing (what poolcheck.py checks)
is never used at all -- every REL float/double load is the two-instruction
`lis rX, lbl_2_<section>_<OFF>@ha` / `lfs fN, ...@l(rX)` form, which is
completely outside poolcheck.py's regex. One instance was hand-verified this
session (d_a_wm_ghost.cpp, see report) by reading the raw bytes at
`<section file offset> + <OFF>` directly out of original/<module>.rel.
Generalising that into an automated per-unit sweep needs matching retail's
anonymous `lbl_2_rodata_OFF` against the draft's often-NAMED equivalent (e.g.
`sGhostClipRadius`), which is a relocation-target correlation problem, not a
name-string comparison -- out of scope for the time available this round.
"""
import json
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
TOOLS = os.path.join(ROOT, 'tools', 'auto_decomp')
sys.path.insert(0, TOOLS)
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import harness
import poolcheck
import blcheck

DISCACHE = os.path.join(HERE, '_dis_rel')
os.makedirs(DISCACHE, exist_ok=True)


def rel_text_file_offset(module):
    d = open(os.path.join(ROOT, 'original', module + '.rel'), 'rb').read()
    numSections, sectionInfoOffset = struct.unpack('>II', d[0xC:0x14])
    off, size = struct.unpack('>II', d[sectionInfoOffset + 1 * 8:sectionInfoOffset + 1 * 8 + 8])
    return off & ~3


NAME_ADDR = re.compile(r'_([0-9A-Fa-f]{8})_text\.o$')


def build_rel_index(module):
    objdir = os.path.join(ROOT, 'bin', 'dtkspl', module, 'obj')
    out = []
    for fn in os.listdir(objdir):
        if not fn.endswith('_text.o'):
            continue
        m = NAME_ADDR.search(fn)
        if not m:
            continue
        lo = int(m.group(1), 16)
        p = os.path.join(objdir, fn)
        sz = poolcheck.__dict__  # placeholder, real size read below
        out.append((lo, p))
    # size via ELF .text section
    sized = []
    for lo, p in out:
        d = open(p, 'rb').read()
        shoff = struct.unpack('>I', d[0x20:0x24])[0]
        shent, shnum, shstrndx = struct.unpack('>HHH', d[0x2E:0x34])
        def sh(i):
            o = shoff + i * shent
            return struct.unpack('>IIIIIIIIII', d[o:o + 40])
        def name_at(tab, x):
            end = d.index(b'\0', tab + x)
            return d[tab + x:end].decode('utf-8', 'replace')
        sn = sh(shstrndx)[4]
        sz = 0
        for i in range(shnum):
            s = sh(i)
            if name_at(sn, s[0]) == '.text':
                sz = s[5]
                break
        sized.append((lo, lo + sz, p))
    sized.sort()
    return sized


def overlapping(index, lo, hi):
    return [p for (a, b, p) in index if a < hi and b > lo]


def disasm_cached(obj_path, tag):
    base = tag + '_' + os.path.splitext(os.path.basename(obj_path))[0]
    out = os.path.join(DISCACHE, base + '.txt')
    if not os.path.exists(out) or os.path.getsize(out) == 0:
        ok, log = harness.disasm(obj_path, out)
        if not ok:
            return None
    return out


def sweep_module(module, jsonfile, compiled_subdir):
    d = json.load(open(os.path.join(ROOT, 'slices', jsonfile), encoding='utf-8'))
    text_file_off = rel_text_file_offset(module)
    idx = build_rel_index(module)
    units = []
    for u in d['slices']:
        tr = u['memoryRanges'].get('.text')
        if not tr:
            continue
        lo_s, hi_s = tr.split('-')
        lo, hi = text_file_off + int(lo_s, 16), text_file_off + int(hi_s, 16)
        units.append((u['source'], lo, hi))

    n_ok = n_no_compiled = n_no_retail = 0
    total_bl_checked = total_bl_mismatch = 0
    findings = []
    for source, lo, hi in units:
        draft_o = os.path.join(ROOT, 'bin', 'compiled', module, compiled_subdir,
                                os.path.splitext(source)[0] + '.o')
        if not os.path.exists(draft_o):
            # try without the module-name-prefixed subdir stripped
            alt = os.path.join(ROOT, 'bin', 'compiled', module,
                                os.path.splitext(source)[0] + '.o')
            draft_o = alt if os.path.exists(alt) else draft_o
        if not os.path.exists(draft_o):
            n_no_compiled += 1
            continue
        retail_objs = overlapping(idx, lo, hi)
        if not retail_objs:
            n_no_retail += 1
            continue
        retail_txts = [disasm_cached(p, module) for p in retail_objs]
        if any(t is None for t in retail_txts):
            n_no_retail += 1
            continue
        draft_txt = disasm_cached(draft_o, module + '_draft')
        if draft_txt is None:
            n_no_retail += 1
            continue

        draft = poolcheck.parse_fns(draft_txt)
        target = {}
        for t in retail_txts:
            target.update(poolcheck.parse_fns(t))

        pairs = []
        for tname in target:
            if tname in draft:
                pairs.append((tname, tname))
                continue
            cand = next((dd for dd in draft if '__' in dd and dd.split('__')[0] == tname), None)
            if cand:
                pairs.append((tname, cand))

        n_ok += 1
        for tname, dname in pairs:
            t, dd = target[tname], draft[dname]
            if len(t) != len(dd):
                continue
            raw_eq = [b for b, _ in t] == [b for b, _ in dd]
            canon_eq = (harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in dd]))
            if not (raw_eq or canon_eq):
                continue
            bad_bl = blcheck.compare_bl(t, dd)
            total_bl_checked += sum(1 for _, tx in t if blcheck.BL_REF.match(tx))
            for i, tt, dtv in bad_bl:
                total_bl_mismatch += 1
                findings.append((source, tname, i, tt, dtv, raw_eq, canon_eq))

    return {
        'module': module, 'n_units': len(units), 'n_ok': n_ok,
        'n_no_compiled': n_no_compiled, 'n_no_retail': n_no_retail,
        'bl_checked': total_bl_checked, 'bl_mismatch': total_bl_mismatch,
        'findings': findings,
    }


def main():
    results = []
    for module, jsonfile, subdir in [
        ('d_basesNP', 'd_basesNP.json', 'd_basesNP'),
        ('d_enemiesNP', 'd_enemiesNP.json', 'd_enemiesNP'),
        ('d_profileNP', 'd_profileNP.json', 'd_profileNP'),
        ('d_en_bossNP', 'd_en_bossNP.json', 'd_en_bossNP'),
    ]:
        r = sweep_module(module, jsonfile, subdir)
        results.append(r)
        print(f"=== {r['module']} ===")
        print(f"{r['n_units']} landed units; checked {r['n_ok']}; "
              f"no compiled obj {r['n_no_compiled']}; no retail split {r['n_no_retail']}")
        print(f"BL: {r['bl_checked']} checked, {r['bl_mismatch']} mismatched")
        for source, tname, i, tt, dtv, raw_eq, canon_eq in r['findings']:
            gate = 'RAW-BYTES-ONLY' if raw_eq and not canon_eq else 'raw+canon'
            print(f"  BL MISMATCH: {source} {tname} instr {i} retail={tt!r} draft={dtv!r} gate={gate}")
        print()

    out = os.path.join(HERE, 'rel_report.json')
    with open(out, 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2)
    print(f'Written: {out}')


if __name__ == '__main__':
    main()
