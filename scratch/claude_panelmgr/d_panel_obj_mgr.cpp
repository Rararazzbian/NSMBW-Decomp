#include <game/bases/d_panel_obj_mgr.hpp>

void dPanelObjMgr_c::addPanelObjList(dPanelObjList_c *list) {
    if (mpLast == 0) {
        mpLast = list;
        mpFirst = list;
    } else {
        list->mpPrev = mpLast;
        mpLast->mpNext = list;
        mpLast = list;
    }

    mCount++;
}

void dPanelObjMgr_c::removePanelObjList(dPanelObjList_c *list) {
    dPanelObjList_c *prev = list->mpPrev;
    dPanelObjList_c *next = list->mpNext;

    if (prev != 0) {
        prev->mpNext = next;
        list->mpPrev = 0;
    } else {
        mpFirst = next;
    }

    if (next != 0) {
        next->mpPrev = prev;
        list->mpNext = 0;
    } else {
        mpLast = prev;
    }

    mCount--;
}

void dPanelObjMgr_c::removeAllPanelObjList() {
    while (mpLast != 0) {
        removePanelObjList(mpLast);
    }
}

dPanelObjMgr_c::dPanelObjMgr_c() {
    mCount = 0;
    mpFirst = 0;
    mpLast = 0;
}

dPanelObjMgr_c::~dPanelObjMgr_c() {
    if (mCount != 0) {
        removeAllPanelObjList();
    }
}
