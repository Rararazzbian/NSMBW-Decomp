"""Compare a draft's pooled CONSTANTS against retail's, by value, per function.

Why this exists
---------------
The matching gate cannot see a wrong constant, and this has now produced false
positives in several rounds on several units.

`lfs f0, "@54951_8042CB1C"@sda21(r0)` assembles to `C0 00 00 00` with the offset
field ZEROED, so raw-byte equality is blind by construction. Canonicalised text
is blind too, but for a different reason: it renumbers pool symbols by order of
appearance, so a draft loading `0.0f` and a retail loading `1.0f` produce the
*same canonical text* as long as both are the first pool reference in the
function. Two independent gates, the same hole.

The value only exists in the binaries, so read it from them, and compare BIT
PATTERNS -- `0.5f` spelled `1.0f/2.0f` folds to the same bits and is not a
defect, while `0.0f` against `-0.0f` compares equal as floats and is one.

DOL and REL are DIFFERENT PROBLEMS -- read this before trusting a clean run
--------------------------------------------------------------------------
For a long time this tool recognised exactly ONE reference form:

    lfs f1, "@54951_8042CB1C"@sda21(r0)

which is the DOL's small-data addressing. The four RELs are compiled with
`-sdata 0 -sdata2 0`, so **they contain no `@sda2` reference anywhere**. On every
REL unit the tool therefore matched nothing, compared nothing, and printed
`0 pooled constants compared across 0 paired functions` -- a clean-looking
report that had checked precisely nothing. Every REL unit ever "verified" was
verified without it. That silent no-op is the specific behaviour this file now
goes out of its way to make impossible: see `Result.problems()`.

A REL reaches a pooled constant in one of two ways, and BOTH are handled here:

  1. **symbolic**  `lis r3, lbl_2_rodata_87C0@ha` / `lfs f0, lbl_2_rodata_87C0@l(r3)`
     -- the whole address is in the symbol, the displacement field is zero.
  2. **base + displacement**  `lis r31, lbl_2_rodata_87B0@ha` /
     `addi r31, r31, lbl_2_rodata_87B0@l` ... later ... `lfs f1, 0x20(r31)`
     -- one base register is set up in the prologue and every constant in the
     pool is reached as a numeric displacement off it.

Form 2 is why a regex alone can never do this job: the instruction that names
the pool and the instruction that reads it are hundreds of words apart, so the
tool has to TRACK which section base is live in which register. That tracking is
also what tells a pooled load apart from an ordinary member load -- `lfs f0,
0xac(r30)` where `r30` is `this` looks identical and must not be counted.

Resolving a reference to bytes
------------------------------
  * retail DOL -- dtk embeds the VA in the symbol name (`@54951_8042CB1C`, or
    `lbl_802F0C80`), so decode straight out of `original/wiimj2d.dol`;
  * retail REL -- dtk names it `lbl_<module>_<section>_<offset>`, which is
    self-describing; read `original/<module>.rel` at that section and offset.
    Nothing needs to be relocated: a REL constant has a section and an offset,
    never an address;
  * draft (either) -- the symbol (`@7365`, `...rodata.0`, or a real name) is
    local to the object, so resolve it through the object's own symbol table
    into its section data.

Then walk the two instruction streams in lockstep and compare, position by
position, every place both sides reference a pooled constant.

    python poolcheck.py <draft.cpp> <shadow_include> <target.txt> [target.txt ...]
    python poolcheck.py ... --module d_basesNP     REL units: MANDATORY, the
                                                   compiler flags differ
    python poolcheck.py ... --obj a.o --txt a.txt  use a prebuilt object instead
                                                   of compiling
    python poolcheck.py ... --pairs pairs.txt      explicit `target draft` pairs
    python poolcheck.py ... --all                  also check already-differing fns

By default only functions the gate calls MATCHED are checked, because those are
the dangerous ones -- a mismatch there is a false positive being counted as
progress. A mismatch in an already-differing function is just one more diff.

`lfs`/`lfd` selects the reading: a 4-byte float or an 8-byte double. Comparing
the wrong width invents disagreements, so the opcode decides, not a guess.

Exit codes: 0 clean, 1 wrong constant(s), 2 unresolved reference(s),
3 the tool checked nothing it should have checked (the silent no-op).
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
# KEPT for the callers that import it (wip/line_mng_shared/tally.py and friends).
# It recognises the DOL small-data form ONLY; the scanner below is what the rest
# of this file uses.
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


# --------------------------------------------------------------- object reading

def read_object(obj_path):
    """(sections, symbols) for a relocatable object.

    sections: [(sh_type, file_offset, size), ...] indexed by section number
    symbols:  {name: (shndx, value, size)}

    Everything the draft side needs comes from here. A draft's pool entry may be
    an anonymous compiler symbol (`@13693`), a section symbol with an addend
    baked into its name (`...rodata.0`), or an ordinary named object -- all three
    are plain symbol-table entries, so one lookup covers them.
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
    names = [name_at(sn, sh(i)[0]) for i in range(shnum)]
    sections = [(sh(i)[1], sh(i)[4], sh(i)[5]) for i in range(shnum)]
    idx = {n: i for i, n in enumerate(names)}
    syms = {}
    if '.symtab' in idx:
        sym, strtab = sh(idx['.symtab']), sh(idx['.strtab'])[4]
        for k in range(sym[5] // 16):
            o = sym[4] + k * 16
            st_name, st_val, st_size, _, _, st_shndx = struct.unpack('>IIIBBH', d[o:o + 16])
            nm = name_at(strtab, st_name)
            if nm and nm not in syms:
                syms[nm] = (st_shndx, st_val, st_size)
    return d, sections, syms, names


def object_pool(obj_path):
    """{symbol_name: bytes} for every sized `@`-prefixed data symbol.

    KEPT for compare_pools() and for the wip/ scripts that import it. The general
    resolver is ObjectSpace below; this is the narrow legacy view.
    """
    d, sections, syms, _ = read_object(obj_path)
    out = {}
    for nm, (shndx, val, size) in syms.items():
        if not nm.startswith('@') or not size or shndx == 0 or shndx >= len(sections):
            continue
        stype, soff, _ssize = sections[shndx]
        if stype == 8:       # SHT_NOBITS -- .bss has no bytes on disk
            continue
        out[nm] = d[soff + val:soff + val + max(size, 8)]
    return out


def decode(raw, width):
    if raw is None or len(raw) < width:
        return None
    return struct.unpack('>f' if width == 4 else '>d', raw[:width])[0]


# ------------------------------------------------------------------- locations

class Loc:
    """A resolved place to read constant bytes from, plus a printable name.

    `plus(n)` is what makes base-register addressing work: the base register
    holds a Loc, and the load's displacement produces a new Loc n bytes along.
    """

    __slots__ = ('reader', 'section', 'offset', 'label', 'secname', 'nobits')

    def __init__(self, reader, section, offset, label, secname=None, nobits=False):
        self.reader, self.section, self.offset, self.label = reader, section, offset, label
        self.secname = secname if secname is not None else str(section)
        # True when the section has NO static image -- `.bss`/`.sbss`, whose
        # contents are produced at runtime by `__sinit`. A float loaded from
        # there has no value in any binary, so it is not checkable in principle,
        # only reportable. Saying that plainly beats implying the tool broke.
        self.nobits = nobits

    def plus(self, n):
        return Loc(self.reader, self.section, self.offset + n,
                   '%s+0x%X' % (self.label, n) if n else self.label, self.secname,
                   self.nobits)

    def read(self, size):
        return self.reader(self.section, self.offset, size)

    def __repr__(self):
        return '%s (%s+0x%X)' % (self.label, self.secname, self.offset)


class DolSpace:
    """Retail DOL: one flat virtual address space."""

    def __init__(self):
        self.data, self.sections = pool.load()

    def _read(self, _section, va, size):
        off = pool.va_to_off(va, self.sections)
        if off is None:
            return None
        return self.data[off:off + size]

    def at_va(self, va, label):
        # A VA that is inside no DOL section is `.bss`/`.sbss`: the DOL stores no
        # bytes for them at all.
        return Loc(self._read, 'VA', va, label, 'VA',
                   pool.va_to_off(va, self.sections) is None)


class RelSpace:
    """One retail `.rel`: section-relative, no address space at all."""

    def __init__(self, module_num):
        self.img = pool.rel(module_num)

    def _read(self, section, off, size):
        return self.img.read(section, off, size)

    def at(self, section, off, label):
        idx = self.img.section_index.get('.' + section.lstrip('.'))
        nobits = idx is not None and idx < len(self.img.sections) \
            and self.img.sections[idx][0] == 0
        return Loc(self._read, section, off, label, '.' + section.lstrip('.'), nobits)


class ObjectSpace:
    """Our own compiled object: resolve a symbol name into its section bytes."""

    def __init__(self, obj_path):
        self.data, self.sections, self.syms, self.secnames = read_object(obj_path)

    def secname(self, shndx):
        return self.secnames[shndx] if shndx < len(self.secnames) else '?%d' % shndx

    def _read(self, shndx, off, size):
        stype, soff, ssize = self.sections[shndx]
        if stype == 8:                       # .bss -- no bytes on disk
            return None
        if off < 0 or off + size > ssize:
            return None
        return self.data[soff + off:soff + off + size]

    def at_symbol(self, name):
        s = self.syms.get(name)
        if s is None:
            return None
        shndx, val, _size = s
        if shndx == 0 or shndx >= len(self.sections):   # undefined / external
            return None
        return Loc(self._read, shndx, val, name, self.secname(shndx),
                   self.sections[shndx][0] == 8)     # SHT_NOBITS


# ------------------------------------------------------- reference recognition

FLOAT_LOAD = re.compile(r'^(lfs|lfd)\s+f\d+,\s*(.+)$')
# `SYM@l(r3)`, `"@54951_..."@sda21(r0)`
OPERAND_SYM = re.compile(r'^"?(.+?)"?@(sda21|sda2|l|ha)\((r\d+)\)$')
# `0x20(r31)` / `-0x8(r31)` / `8(r31)`
OPERAND_DISP = re.compile(r'^(-?(?:0x[0-9A-Fa-f]+|\d+))\((r\d+)\)$')

LIS_HA = re.compile(r'^lis\s+(r\d+),\s*"?(.+?)"?@ha$')
ADDI_LO = re.compile(r'^addi\s+(r\d+),\s*(r\d+),\s*"?(.+?)"?@l$')
ADDI_IMM = re.compile(r'^addi\s+(r\d+),\s*(r\d+),\s*(-?(?:0x[0-9A-Fa-f]+|\d+))$')
MR = re.compile(r'^mr\s+(r\d+),\s*(r\d+)$')
FIRST_GPR = re.compile(r'^([a-z][a-z0-9._]*)\s+(r\d+)\b')

# Written by a `bl`. r13 is the small-data base and r1 the stack pointer; neither
# is ever a literal-pool base, and pretending otherwise only loses tracking.
VOLATILE = {'r0'} | {'r%d' % i for i in range(3, 13)}
# Mnemonics whose FIRST operand is a source, not a destination. Getting this
# wrong in the other direction is safe -- it drops a base and reports UNRESOLVED
# -- so the list only needs the cases that are common enough to matter.
NO_GPR_WRITE = ('st', 'b', 'cmp', 'mt', 'tw', 'sync', 'isync', 'dcb', 'icbi',
                'eieio', 'nop', 'cr', 'sc', 'rfi', 'lfs', 'lfd', 'stf', 'fc')

# Retail pool symbols.
DOL_POOL_VA = re.compile(r'^@\d+_([0-9A-Fa-f]{8})$')     # @54951_8042CB1C
DOL_LABEL = re.compile(r'^lbl_([0-9A-Fa-f]{8})$')        # lbl_802F0C80
REL_LABEL = pool.REL_LABEL                               # lbl_2_rodata_87B0
# Draft pool symbols: the anonymous compiler pool, and MWCC's section symbols.
DRAFT_POOL = re.compile(r'^@\d+$')                       # @13693
DRAFT_SECT = re.compile(r'^\.\.\.[A-Za-z0-9_]+\.\d+$')   # ...rodata.0


def is_pool_symbol(name):
    """True if this name is an ANONYMOUS pool reference on either side.

    These are exactly the names the match gate is blind to: canonicalise()
    renumbers them by order of appearance (or, for a REL's `lbl_2_rodata_87B0`,
    fails to normalise them at all, leaving raw-byte equality -- with its zeroed
    displacement field -- as the only gate that can pass). A reference where both
    sides spell the SAME real symbol name needs no value check, because the name
    is compared directly and a name determines the value.
    """
    return bool(DOL_POOL_VA.match(name) or DOL_LABEL.match(name)
                or REL_LABEL.match(name) or DRAFT_POOL.match(name)
                or DRAFT_SECT.match(name))


def target_module(target_txts):
    """The REL module number a target disassembly belongs to, or None for the DOL.

    Every REL pool label is `lbl_<module>_<section>_<offset>`, so the target
    names its own module. Used to refuse a run whose compiler flags are wrong:
    the RELs build with `-O4,p -sdata 0 -sdata2 0` and compiling a REL unit with
    the DOL's flags silently produces a different program -- on d_a_wm_course it
    pairs 14 functions instead of 23 and compares 2 constants instead of 25.
    """
    found = set()
    for path in target_txts:
        for line in open(path, encoding='utf-8', errors='replace'):
            for m in re.finditer(r'\blbl_(\d+)_(?:text|ctors|dtors|rodata|data|bss)_'
                                 r'[0-9A-Fa-f]+\b', line):
                found.add(int(m.group(1)))
    if len(found) > 1:
        raise SystemExit('target names more than one REL module: %s' % sorted(found))
    return found.pop() if found else None


def retail_resolver(dol=None):
    """symbol name -> Loc, for a dtk disassembly of retail (DOL or any REL).

    The symbol says which binary it lives in, so no module argument is needed:
    `@54951_8042CB1C` and `lbl_802F0C80` are DOL virtual addresses,
    `lbl_2_rodata_87B0` is module 2's `.rodata` at offset 0x87B0.
    """
    dolspace = {'v': dol}
    relspaces = {}

    def resolve(name):
        m = REL_LABEL.match(name)
        if m:
            mod = int(m.group(1))
            if mod not in relspaces:
                try:
                    relspaces[mod] = RelSpace(mod)
                except (KeyError, OSError):
                    relspaces[mod] = None
            sp = relspaces[mod]
            return None if sp is None else sp.at(m.group(2), int(m.group(3), 16), name)
        m = DOL_POOL_VA.match(name) or DOL_LABEL.match(name)
        if m:
            if dolspace['v'] is None:
                dolspace['v'] = DolSpace()
            return dolspace['v'].at_va(int(m.group(1), 16), name)
        return None

    return resolve


def draft_resolver(objspace):
    return lambda name: objspace.at_symbol(name)


class Ref:
    """One float load that reaches a constant, or should have."""

    __slots__ = ('op', 'width', 'symbol', 'via', 'loc')

    def __init__(self, op, symbol, via, loc):
        self.op, self.symbol, self.via, self.loc = op, symbol, via, loc
        self.width = 4 if op == 'lfs' else 8

    def bits(self):
        return None if self.loc is None else self.loc.read(self.width)

    def describe(self):
        where = repr(self.loc) if self.loc is not None else 'UNRESOLVED'
        return '%s %s [%s] %s' % (self.op, self.symbol or '(base%+d)' % 0, self.via, where)


def scan(instrs, resolve):
    """{index: Ref} for every `lfs`/`lfd` in `instrs` that reaches a pool.

    Walks the stream tracking which section base is live in which register, so
    that `lfs f1, 0x20(r31)` hundreds of words after `addi r31, r31, SYM@l` is
    understood, while `lfs f0, 0xac(r30)` -- a member load off `this` -- is
    correctly not counted at all.

    Also returns the count of float loads seen, INCLUDING the ones judged not to
    be pool references. The caller needs that number: "zero constants compared"
    is only acceptable if the unit genuinely has no float loads in it.
    """
    bases, hi = {}, {}
    out, nloads = {}, 0
    for i, raw in enumerate(instrs):
        text = raw.strip()

        m = FLOAT_LOAD.match(text)
        if m:
            nloads += 1
            op, operand = m.group(1), m.group(2).strip()
            ms = OPERAND_SYM.match(operand)
            if ms:
                name = ms.group(1)
                out[i] = Ref(op, name, ms.group(2), resolve(name))
            else:
                md = OPERAND_DISP.match(operand)
                if md:
                    disp, reg = int(md.group(1), 0), md.group(2)
                    base = bases.get(reg)
                    if base is not None:
                        out[i] = Ref(op, None, 'base %s%+#x' % (reg, disp), base.plus(disp))
            continue                     # a float load writes no GPR

        m = LIS_HA.match(text)
        if m:
            hi[m.group(1)] = m.group(2)
            bases.pop(m.group(1), None)
            continue

        m = ADDI_LO.match(text)
        if m:
            rd, rs, name = m.groups()
            bases.pop(rd, None)
            if hi.get(rs) == name:
                loc = resolve(name)
                if loc is not None:
                    bases[rd] = loc
            hi.pop(rd, None)
            continue

        m = ADDI_IMM.match(text)
        if m:
            rd, rs, imm = m.groups()
            base = bases.get(rs)
            hi.pop(rd, None)
            if base is not None and rd not in ('r1', 'r13'):
                bases[rd] = base.plus(int(imm, 0))
            else:
                bases.pop(rd, None)
            continue

        m = MR.match(text)
        if m:
            rd, rs = m.groups()
            hi.pop(rd, None)
            base = bases.get(rs)
            if base is not None:
                bases[rd] = base
            else:
                bases.pop(rd, None)
            continue

        if text.startswith(('bl ', 'bl\t', 'bctrl', 'blrl')) or text == 'bl':
            for r in VOLATILE:
                bases.pop(r, None)
                hi.pop(r, None)
            continue

        m = FIRST_GPR.match(text)
        if m and not m.group(1).startswith(NO_GPR_WRITE):
            bases.pop(m.group(2), None)
            hi.pop(m.group(2), None)
    return out, nloads


# ------------------------------------------------------------------- comparing

def retail_value(symbol, width, dol):
    """Legacy DOL-only helper. KEPT for the wip/ scripts that import it."""
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

    LEGACY DOL-ONLY ENTRY POINT, kept because wip/line_mng_shared/tally.py calls
    it with exactly this signature. It sees `@sda21` references and nothing else,
    so it is blind on every REL unit. New code should use compare_refs().
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


class Result:
    """Everything one run found, and -- crucially -- what is WRONG with the run.

    The old failure mode was reporting `0 compared across 0 paired functions` and
    exiting 0. That reads like a pass. `problems()` exists so a run that checked
    nothing can never again be mistaken for a run that checked everything and
    found nothing.
    """

    def __init__(self):
        self.pairs = 0
        self.examined = 0          # pairs actually value-checked
        self.skipped_len = 0
        self.compared = 0          # both sides resolved, bits compared
        self.name_equal = 0        # both sides name the SAME real symbol
        self.unresolved = []       # (fn, index, why, target ref, draft ref)
        self.mismatched = []       # (fn, index, target ref, draft ref, gate_matched)
        self.float_loads = 0       # every lfs/lfd seen on the target side
        self.pool_sites = 0        # positions where at least one side pools
        self.target_fns = 0
        self.draft_fns = 0

    def problems(self):
        out = []
        if self.target_fns == 0:
            out.append('NO TARGET FUNCTIONS: the target disassembly parsed to nothing.')
        if self.draft_fns == 0:
            out.append('NO DRAFT FUNCTIONS: the draft object disassembled to nothing.')
        if self.pairs == 0 and self.target_fns:
            out.append(
                'NO FUNCTIONS PAIRED: %d target and %d draft functions, and not one '
                'pairing. Nothing was checked. Supply --pairs, or use the content '
                'pairing (--pair-by-content, on by default).'
                % (self.target_fns, self.draft_fns))
        if self.examined and self.compared == 0 and self.float_loads:
            out.append(
                'CHECKED NOTHING: %d float load(s) across %d paired function(s) and '
                'NOT ONE pooled constant was compared. That is the silent no-op this '
                'tool exists to prevent -- treat this run as having verified nothing.'
                % (self.float_loads, self.pairs))
        if self.unresolved:
            out.append('%d POOL REFERENCE(S) UNRESOLVED -- see the list above. An '
                       'unresolved reference is NOT a pass.' % len(self.unresolved))
        return out

    def warnings(self):
        """Coverage facts a reader must see before calling a clean run a pass.

        A partial pairing is legitimate -- a dtk split object routinely carries a
        few functions from the neighbouring TU -- but "23 target functions, 1
        paired, 0 constants, clean" and "23 target functions, 23 paired, 0
        constants, clean" are completely different claims, and the old summary
        line rendered them identically.
        """
        out = []
        if self.target_fns and self.examined < self.target_fns:
            out.append('COVERAGE: %d of %d target function(s) value-checked; %d were '
                       'not checked at all (unpaired, length-mismatched, or already '
                       'differing).'
                       % (self.examined, self.target_fns,
                          self.target_fns - self.examined))
        return out

    def exit_code(self):
        if self.mismatched:
            return 1
        if any(p.startswith(('NO ', 'CHECKED NOTHING')) for p in self.problems()):
            return 3
        if self.unresolved:
            return 2
        return 0


def compare_refs(name, target_fn, draft_fn, tresolve, dresolve, gate_matched, res):
    """Value-check one paired function, position by position.

    Both arguments are `[(bytes, text), ...]` instruction lists of the SAME
    length -- the caller has already established that.
    """
    trefs, tloads = scan([t for _, t in target_fn], tresolve)
    drefs, _ = scan([t for _, t in draft_fn], dresolve)
    res.float_loads += tloads

    for i in sorted(set(trefs) | set(drefs)):
        t, d = trefs.get(i), drefs.get(i)
        res.pool_sites += 1

        if t is None or d is None:
            side = 'draft' if t is None else 'retail'
            other = d or t
            # One side reaches a pool here and the other does not. If the other
            # side's instruction is not even a float load, the ordinary gate
            # already sees that as an instruction difference -- but if it IS a
            # float load, this is a reference we could not follow, and it must
            # be reported rather than skipped.
            res.unresolved.append(
                (name, i, '%s side has no resolvable pool reference at this '
                          'position' % side, t, d))
            continue

        if t.op != d.op:
            # lfs opposite lfd: an instruction-selection difference the ordinary
            # gate already sees, and the two widths are not comparable anyway.
            continue

        if t.symbol and d.symbol and t.symbol == d.symbol \
                and not is_pool_symbol(t.symbol):
            # Same real symbol name on both sides. The name is compared directly
            # by the gate and a name determines the value.
            res.name_equal += 1
            continue

        tb, db = t.bits(), d.bits()
        if tb is None or db is None:
            failed = [(s, r) for s, r in (('retail', t), ('draft', d)) if r.bits() is None]
            bad = [s for s, _ in failed]
            if all(r.loc is not None and r.loc.nobits for _, r in failed):
                why = ('%s side loads from a section with NO STATIC IMAGE (.bss) -- '
                       'the value is produced at runtime by __sinit, so it exists in '
                       'no binary and CANNOT be value-checked' % '/'.join(bad))
            else:
                why = 'could not read the constant on the %s side' % '/'.join(bad)
            res.unresolved.append((name, i, why, t, d))
            continue

        res.compared += 1
        if tb != db:
            res.mismatched.append((name, i, t, d, gate_matched, tb, db))


# ---------------------------------------------------------------------- pairing

def pair_functions(target, draft, by_content=True):
    """[(target name, draft name), ...].

    Three passes, cheapest first:
      1. exact name;
      2. the mangled-suffix form a static helper takes (`fn_800C31C0` in retail
         vs `fn_800C31C0__FP10dLineMng_c` in a draft);
      3. by CONTENT, modulo symbol names -- which is the only thing that works
         for a REL unit, where every target function is an anonymous `fn_2_*`
         and the draft emits real mangled names. Same reasoning as
         wip/wm_units/verify_anon.py; ascending order is preferred among equal
         candidates so a tie between two byte-identical bodies cannot invent a
         pairing.
    """
    pairs, used = [], set()
    for tname in target:
        if tname in draft:
            pairs.append((tname, tname))
            used.add(tname)
            continue
        cand = next((d for d in draft
                     if d not in used and '__' in d and d.split('__')[0] == tname), None)
        if cand:
            pairs.append((tname, cand))
            used.add(cand)
    if not by_content:
        return pairs

    done = {t for t, _ in pairs}
    dnames = [d for d in draft if d not in used]
    dnorm = {d: tuple(harness.canonicalise([x for _, x in draft[d]])) for d in dnames}
    last = -1
    for tname in target:
        if tname in done:
            continue
        want = tuple(harness.canonicalise([x for _, x in target[tname]]))
        cands = [k for k, d in enumerate(dnames) if dnames[k] not in used and dnorm[d] == want]
        if not cands:
            twant = tuple(b for b, _ in target[tname])
            cands = [k for k, d in enumerate(dnames)
                     if d not in used and tuple(b for b, _ in draft[d]) == twant]
        if not cands:
            continue
        asc = [k for k in cands if k > last]
        hit = asc[0] if asc else cands[0]
        last = max(last, hit)
        used.add(dnames[hit])
        pairs.append((tname, dnames[hit]))
    return pairs


# -------------------------------------------------------------------------- cli

def run(obj, draft_txt, target_txts, only_matched=True, pairs_file=None,
        by_content=True):
    draft = parse_fns(draft_txt)
    target = {}
    for path in target_txts:
        target.update(parse_fns(path))

    objspace = ObjectSpace(obj)
    tresolve, dresolve = retail_resolver(), draft_resolver(objspace)

    res = Result()
    res.target_fns, res.draft_fns = len(target), len(draft)

    if pairs_file:
        pairs = []
        for line in open(pairs_file, encoding='utf-8'):
            line = line.split('#')[0].strip()
            if line:
                t, d = line.split()[:2]
                pairs.append((t, d))
    else:
        pairs = pair_functions(target, draft, by_content=by_content)

    skipped_len = 0
    for tname, dname in pairs:
        t, d = target.get(tname), draft.get(dname)
        if t is None or d is None or len(t) != len(d):
            skipped_len += 1
            continue
        res.pairs += 1
        gate_matched = ([b for b, _ in t] == [b for b, _ in d]
                        or harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in d]))
        if only_matched and not gate_matched:
            continue
        res.examined += 1
        compare_refs(tname, t, d, tresolve, dresolve, gate_matched, res)
    res.skipped_len = skipped_len
    return res


def report(res):
    for name, i, t, d, gm, tb, db in res.mismatched:
        print('%s: %s' % ('FALSE POSITIVE' if gm else 'differing fn', name))
        print('    instruction %d: retail %s = %s %r' % (
            i, t.loc, tb.hex().upper(), decode(tb, t.width)))
        print('                    draft  %s = %s %r' % (
            d.loc, db.hex().upper(), decode(db, d.width)))
    for name, i, why, t, d in res.unresolved:
        print('UNRESOLVED: %s instruction %d -- %s' % (name, i, why))
        print('    retail: %s' % (t.describe() if t else '(no pool reference)'))
        print('    draft : %s' % (d.describe() if d else '(no pool reference)'))

    print('\n%d pooled constants compared by VALUE across %d paired functions'
          % (res.compared, res.pairs))
    print('%d mismatched, %d could not be resolved on one side'
          % (len(res.mismatched), len(res.unresolved)))
    print('(%d pair(s) value-checked; %d reference(s) skipped as the same named '
          'symbol on both sides; %d float load(s) seen; %d pair(s) skipped on length)'
          % (res.examined, res.name_equal, res.float_loads, res.skipped_len))
    if res.pairs and not res.examined:
        print('NOTE: every paired function already DIFFERS, so none was value-checked. '
              'Re-run with --all to check them anyway.')
    for w in res.warnings():
        print(w)

    problems = res.problems()
    if problems:
        print('\n' + '!' * 72)
        for p in problems:
            print('!! ' + p)
        print('!' * 72)
    elif res.compared == 0 and res.float_loads == 0:
        print('\nNote: this unit contains no float loads at all in its paired, '
              'matched functions -- there was genuinely nothing to check.')
    return res.exit_code()


def main():
    argv = sys.argv[1:]
    only_matched = '--all' not in argv
    by_content = '--no-content-pairing' not in argv
    argv = [a for a in argv if a not in ('--all', '--no-content-pairing')]

    def opt(flag):
        if flag in argv:
            i = argv.index(flag)
            v = argv[i + 1]
            del argv[i:i + 2]
            return v
        return None

    module = opt('--module') or 'wiimj2d'
    obj_in = opt('--obj')
    txt_in = opt('--txt')
    pairs_file = opt('--pairs')

    if obj_in and txt_in:
        if len(argv) < 1:
            print(__doc__)
            return 2
        targets = [os.path.abspath(a) for a in argv]
        res = run(os.path.abspath(obj_in), os.path.abspath(txt_in), targets,
                  only_matched, pairs_file, by_content)
        return report(res)

    if len(argv) < 3:
        print(__doc__)
        return 2
    src, inc = os.path.abspath(argv[0]), os.path.abspath(argv[1])
    targets = [os.path.abspath(a) for a in argv[2:]]

    mod = target_module(targets)
    if mod is not None:
        want = os.path.splitext(pool.module_meta()[mod][0])[0]
        if module != want:
            print('REFUSING TO RUN: this target is REL module %d (%s), but the draft '
                  'would be compiled with %s\'s flags.\n'
                  'The RELs build with -O4,p -sdata 0 -sdata2 0; the DOL does not, and '
                  'the wrong set\ncompiles a DIFFERENT PROGRAM, so every result below '
                  'would be meaningless.\n'
                  'Re-run with:  --module %s' % (mod, want, module, want))
            return 3
    elif module != 'wiimj2d':
        print('WARNING: --module %s given, but the target names no REL label -- it '
              'looks like a DOL target.' % module)

    work = os.path.join(os.path.dirname(src), '_poolcheck')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc], module=module)
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)
    return report(run(obj, txt, targets, only_matched, pairs_file, by_content))


if __name__ == '__main__':
    sys.exit(main())
