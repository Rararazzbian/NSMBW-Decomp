#include <types.h>
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
