"""Compare a draft's pooled CONSTANTS against retail's, by value, per function.

Why this exists
---------------
The matching gate cannot see a wrong constant, and this has now produced false
positives in three separate rounds on two different units.

`lfs f0, "@54951_8042CB1C"@sda21(r0)` assembles to `C0 00 00 00` with the offset
field ZEROED, so raw-byte equality is blind by construction. Canonicalised text
is blind too, but for a different reason: it renumbers pool symbols by order of
appearance, so a draft loading `0.0f` and a retail loading `1.0f` produce the
*same canonical text* as long as both are the first pool reference in the
function. Two independent gates, the same hole.

The value only exists in the binaries, so read it from them:

  * retail  -- dtk embeds the address in the symbol name (`@54951_8042CB1C`),
               so decode straight out of `original/wiimj2d.dol`;
  * draft   -- the symbol (`@7365`) is local to the object, so resolve it
               through the object's own symbol table into its section data.

Then walk the two instruction streams in lockstep and compare, position by
position, every place both sides reference a pool symbol.

    python poolcheck.py <draft.cpp> <shadow_include> <target.txt>
    python poolcheck.py ... --all       also check functions that already differ

By default only functions the gate calls MATCHED are checked, because those are
the dangerous ones -- a mismatch there is a false positive being counted as
progress. A mismatch in an already-differing function is just one more diff.

`lfs`/`lfd` selects the reading: a 4-byte float or an 8-byte double. Comparing
the wrong width invents disagreements, so the opcode decides, not a guess.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import harness
import pool

# `lfs f0, "@54951_8042CB1C"@sda21(r0)` / `lfd f2, "@7370"@sda21(r0)`
POOL_REF = re.compile(r'^\s*(lfs|lfd)\s+f\d+,\s*"?(@[\w]+)"?@sda2?1?\(r\d+\)')
# dtk names carry the VA after the last underscore; draft names do not.
VA_IN_NAME = re.compile(r'_([0-9A-Fa-f]{8})$')


def parse_fns(path):
    """{name: [(bytes, text), ...]} -- same shape tally.py uses."""
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


def object_pool(obj_path):
    """{symbol_name: bytes} for every data symbol in a relocatable object.

    The draft's pool entries are plain local symbols in `.sdata2` (or `.rodata`
    for a few), so their value is simply the section's bytes at the symbol's
    offset. Sized symbols only -- an unsized label tells us nothing to read.
    """
    d = open(obj_path, 'rb').read()
    shoff = struct.unpack('>I', d[0x20:0x24])[0]
    shent, shnum, shstr = struct.unpack('>HHH', d[0x2E:0x34])

    def sh(i):
        o = shoff + i * shent
        return struct.unpack('>IIIIIIIIII', d[o:o + 40])

    def name_at(tab, x):
        end = d.index(b'\0', tab + x)
        return d[tab + x:end].decode('utf-8', 'replace')

    sn = sh(shstr)[4]
    idx = {name_at(sn, sh(i)[0]): i for i in range(shnum)}
    if '.symtab' not in idx:
        return {}
    sym, strtab = sh(idx['.symtab']), sh(idx['.strtab'])[4]
    out = {}
    for k in range(sym[5] // 16):
        o = sym[4] + k * 16
        st_name, st_val, st_size, _, _, st_shndx = struct.unpack('>IIIBBH', d[o:o + 16])
        if st_shndx == 0 or st_shndx >= shnum or not st_size:
            continue
        sec = sh(st_shndx)
        if sec[1] == 8:      # SHT_NOBITS -- .bss has no bytes on disk
            continue
        nm = name_at(strtab, st_name)
        if nm.startswith('@'):
            out[nm] = d[sec[4] + st_val:sec[4] + st_val + max(st_size, 8)]
    return out


def decode(raw, width):
    if len(raw) < width:
        return None
    return struct.unpack('>f' if width == 4 else '>d', raw[:width])[0]


def retail_value(symbol, width, dol):
    m = VA_IN_NAME.search(symbol)
    if not m:
        return None, None
    va = int(m.group(1), 16)
    off = pool.va_to_off(va, dol[1])
    if off is None:
        return va, None
    return va, decode(dol[0][off:off + 8], width)


def compare_pools(target_fn, draft_fn, dpool, dol):
    """[(index, retail_va, retail_value, draft_value), ...] for disagreeing loads.

    Both arguments are `[(bytes, text), ...]` instruction lists of the SAME
    length -- the caller has already established that. Positions where only one
    side has a pool reference are skipped: that is an instruction-selection
    difference the ordinary gate already sees, not a wrong constant. So is an
    `lfs` opposite an `lfd`, which additionally makes the two widths
    incomparable.
    """
    out = []
    for i, ((_, ttext), (_, dtext)) in enumerate(zip(target_fn, draft_fn)):
        tm, dm = POOL_REF.match(ttext), POOL_REF.match(dtext)
        if not tm or not dm or tm.group(1) != dm.group(1):
            continue
        width = 4 if tm.group(1) == 'lfs' else 8
        va, tv = retail_value(tm.group(2), width, dol)
        raw = dpool.get(dm.group(2))
        dv = decode(raw, width) if raw else None
        if tv is None or dv is None:
            continue
        if tv != dv:
            out.append((i, va, tv, dv))
    return out


def main():
    args = [a for a in sys.argv[1:] if a != '--all']
    only_matched = len(args) == len(sys.argv[1:])
    if len(args) < 3:
        print(__doc__)
        return 2
    src, inc, target_txt = (os.path.abspath(a) for a in args[:3])

    work = os.path.join(os.path.dirname(src), '_poolcheck')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)

    draft, target = parse_fns(txt), parse_fns(target_txt)
    dpool, dol = object_pool(obj), pool.load()

    # Pair draft to target by name, then by the mangled-suffix form a static
    # helper takes (`fn_800C31C0` in retail vs `fn_800C31C0__FP10dLineMng_c`).
    pairs = []
    for tname in target:
        if tname in draft:
            pairs.append((tname, tname))
            continue
        cand = next((d for d in draft if '__' in d and d.split('__')[0] == tname), None)
        if cand:
            pairs.append((tname, cand))

    checked = mismatched = unresolved = 0
    findings = []
    for tname, dname in pairs:
        t, d = target[tname], draft[dname]
        if len(t) != len(d):
            continue
        gate_matched = ([b for b, _ in t] == [b for b, _ in d]
                        or harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in d]))
        if only_matched and not gate_matched:
            continue
        for i, ((_, ttext), (_, dtext)) in enumerate(zip(t, d)):
            tm, dm = POOL_REF.match(ttext), POOL_REF.match(dtext)
            if tm and dm and tm.group(1) == dm.group(1):
                width = 4 if tm.group(1) == 'lfs' else 8
                _, tv = retail_value(tm.group(2), width, dol)
                raw = dpool.get(dm.group(2))
                if tv is None or (raw and decode(raw, width)) is None and not raw:
                    unresolved += 1
                else:
                    checked += 1
        for i, va, tv, dv in compare_pools(t, d, dpool, dol):
            mismatched += 1
            findings.append((tname, i, va, tv, dv, gate_matched))

    for name, i, va, tv, dv, gm in findings:
        flag = 'FALSE POSITIVE' if gm else 'differing fn'
        print(f'{flag}: {name}')
        print(f'    instruction {i}: retail 0x{va:08X} = {tv!r}   draft = {dv!r}')
    print(f'\n{checked} pooled constants compared by VALUE across '
          f'{len(pairs)} paired functions')
    print(f'{mismatched} mismatched, {unresolved} could not be resolved on one side')
    return 1 if mismatched else 0


if __name__ == '__main__':
    sys.exit(main())
