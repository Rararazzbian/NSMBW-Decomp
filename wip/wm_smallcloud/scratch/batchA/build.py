"""Compile helper for d_basesNP that uses the SLICE's actual compiler flags
(including -sdata 0 -sdata2 0), since tools/auto_decomp/harness.py's CFLAGS
constant omits them and that caused a false sda21-addressing mismatch.
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

MWCC = H.MWCC
INCLUDES = H.INCLUDES

CFLAGS = ['-c', '-sdata', '0', '-sdata2', '0', '-proc', 'gekko', '-fp', 'hard',
          '-O4,p', '-inline', 'noauto', '-char', 'signed', '-rtti', 'off',
          '-enum', 'int', '-Cpp_exceptions', 'off', '-ipa', 'file', '-enc', 'SJIS',
          '-DREVOLUTION', '-I-']


def compile_draft(src, obj, extra_inc=()):
    args = [MWCC] + CFLAGS + [src, '-o', obj]
    for inc in list(extra_inc) + INCLUDES:
        args += ['-i', inc.replace('/', os.sep)]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or '') + (p.stderr or '')
