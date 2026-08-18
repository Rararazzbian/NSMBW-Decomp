import os, sys, subprocess

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.append(os.path.join(ROOT, 'tools', 'auto_decomp'))

import harness

# Test matching mTex::base_c functions
test_code = r'''#include <types.h>
#include <revolution/OS.h>

namespace mTex {
    class base_c {
    public:
        int mWidth;
        int mHeight;
        int mTileWidth;
        int mTileHeight;
        int mTileSize;
        int mTileCountX;
        int mTileCountY;
        int mTotalTiles;

        void init(int w, int h, int tw, int th);
        int getTileNo(int x, int y) const;
        int getIdInTile(int x, int y) const;
        int xyToDotId(int x, int y) const;
    };
}

void mTex::base_c::init(int w, int h, int tw, int th) {
    mWidth = w;
    mHeight = h;
    mTileWidth = tw;
    mTileHeight = th;
    mTileSize = tw * th;
    if (tw != 0) {
        mTileCountX = (w + tw - 1) / (u32)tw;
    }
    if (th != 0) {
        mTileCountY = (h + th - 1) / (u32)th;
    }
    mTotalTiles = mTileCountX * mTileCountY;
}

int mTex::base_c::getTileNo(int x, int y) const {
    return (u32)x / (u32)mTileWidth + ((u32)y / (u32)mTileHeight) * mTileCountX;
}

int mTex::base_c::getIdInTile(int x, int y) const {
    return (u32)x % (u32)mTileWidth + ((u32)y % (u32)mTileHeight) * mTileWidth;
}

int mTex::base_c::xyToDotId(int x, int y) const {
    if (x < 0 || x >= mWidth || y < 0 || y >= mHeight) {
        return -1;
    }
    return getIdInTile(x, y) + getTileNo(x, y) * mTileSize;
}
'''

src_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'test_mtex.cpp')
obj_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'test_mtex.o')
txt_path = os.path.join(ROOT, 'scratch', 'gemini_round7', 'test_mtex.txt')

with open(src_path, 'w', encoding='utf-8') as f:
    f.write(test_code)

res = harness.compile_draft(src_path, obj_path)
print("Compile:", res)

DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
subprocess.run([DTK, 'elf', 'disasm', obj_path, txt_path], capture_output=True, text=True)

with open(txt_path) as f:
    print(f.read())
