#include <game/bases/d_a_cursor.hpp>
#include <game/bases/d_2d.hpp>
#include <game/bases/d_pad.hpp>
#include <game/mLib/m_pad.hpp>

ACTOR_PROFILE(CURSOR, dAcCursor_c, 0);

int dAcCursor_c::create() {
    static const char *l_file_name_list[LYT_COUNT] = {
        "P1_Def.brlyt", "P1_Cat.brlyt",
        "P2_Def.brlyt", "P2_Cat.brlyt",
        "P3_Def.brlyt", "P3_Cat.brlyt",
        "P4_Def.brlyt", "P4_Cat.brlyt",
    };

    if (!mResLoader.request("Layout/cursor/cursor.arc")) {
        return NOT_READY;
    }

    for (int i = 0; i < LYT_COUNT; i++) {
        mLyts[i].mpResAcc = &mResLoader;
        mLyts[i].build(l_file_name_list[i], nullptr);
        mLyts[i].mDrawOrder = 0x99;
        mLyts[i].calc();
    }

    setLyt(P1_DEF);
    l_cursor = this;
    m_608 = 0;
    return SUCCEEDED;
}

int dAcCursor_c::doDelete() {
    if (!mResLoader.remove()) {
        return NOT_READY;
    }
    l_cursor = nullptr;
    return SUCCEEDED;
}

int dAcCursor_c::execute() {
    u8 oldAlpha = l_alpha;
    bool hide = false;

    if (mPad::g_currentCore->getDpdNumMarks() > 0) {
        mVec2_c pos = dPad::ex_c::m_currentEx->mPointerPos;
        if (pos.x < -304.0f) {
            hide = true;
        }
        if (pos.x > 304.0f) {
            hide = true;
        }
        if (pos.y < -228.0f) {
            hide = true;
        }
        if (pos.y > 228.0f) {
            hide = true;
        }
        l_pos = pos;
    } else {
        hide = true;
    }

    if (hide) {
        if (l_alpha != 0) {
            if (l_alpha > 10) {
                l_alpha -= 10;
            } else {
                l_alpha = 0;
            }
        }
    } else {
        l_alpha = 255;
    }

    if (oldAlpha != l_alpha) {
        d2d::setAlpha(&mLyts[P1_DEF], l_alpha);
        d2d::setAlpha(&mLyts[P1_CAT], l_alpha);
    }
    return SUCCEEDED;
}

int dAcCursor_c::draw() {
    if (l_alpha != 0) {
        mVec3_c &pos = mpCurLyt->mPos;
        pos.x = l_pos.x;
        pos.y = l_pos.y;
        mpCurLyt->calc();
        mpCurLyt->entry();
    }
    return SUCCEEDED;
}

void dAcCursor_c::setLyt(Lyt_e lyt) {
    mpCurLyt = &mLyts[lyt];
}

dAcCursor_c::~dAcCursor_c() {}

dAcCursor_c *dAcCursor_c::l_cursor;
u8 dAcCursor_c::l_alpha;
mVec2_c dAcCursor_c::l_pos(0.0f, 0.0f);
