#!/usr/bin/env python3
"""Generate sN.cpp variants from v2.cpp's prefix + a body snippet.

    python3 gen.py 1 <<'EOF' ...body... EOF
"""
import sys

BASE = '/opt/NSMBW-Decomp/scratch/auto_kokoopa_fumi'
prefix = open(BASE + '/v2.cpp').read().split('KokoopaSpFumiCheck_c::~')[0]

name = sys.argv[1]
body = sys.stdin.read().rstrip('\n')
src = (prefix + 'KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c() {}\n\n' +
       body + '\n')
open('%s/%s.cpp' % (BASE, name), 'w').write(src)
print('wrote %s.cpp (%d lines)' % (name, src.count('\n')))
