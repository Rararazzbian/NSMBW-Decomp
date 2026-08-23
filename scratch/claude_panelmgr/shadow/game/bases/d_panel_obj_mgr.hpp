#pragma once

#include <game/bases/d_panel_obj_list.hpp>

class dPanelObjMgr_c {
public:
    dPanelObjMgr_c();
    ~dPanelObjMgr_c();

    void addPanelObjList(dPanelObjList_c *list);
    void removePanelObjList(dPanelObjList_c *list);
    void removeAllPanelObjList();

public:
    int mCount;
    dPanelObjList_c *mpFirst;
    dPanelObjList_c *mpLast;
};
