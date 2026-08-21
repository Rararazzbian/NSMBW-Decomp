import os
import re

d = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtkspl\obj'
names = sorted(os.listdir(d))
pat = re.compile(r'^auto_sinit_(.+)_text\.o$')
hits = [n for n in names if 'sinit' in n.lower()]
print('sinit-ish objects:', len(hits))
for n in hits:
    print(' ', n)
