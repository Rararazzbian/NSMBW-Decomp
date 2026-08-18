import os, glob

for f in sorted(glob.glob('bin/dtkspl/obj/*pad*') + glob.glob('bin/dtkspl/obj/*coin*') + glob.glob('bin/dtkspl/obj/*8016F*') + glob.glob('bin/dtkspl/obj/*80027*')):
    print(f)
