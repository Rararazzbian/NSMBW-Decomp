#pragma once

#include <types.h>

class dPanelObjList_c {
public:
    dPanelObjList_c();
    ~dPanelObjList_c();

    u16 getValue() const;
    bool isChange() const;
    void setChange(bool change);
    f32 getPosX() const;
    f32 getPosY() const;
    f32 getPosZ() const;
    void setPosXY(f32 x, f32 y);
    void setPos(f32 x, f32 y, f32 z);
    int getType() const;
    void setScaleFoot(f32 scale);
    void setScaleAngle(f32 scale, s16 angle);
    f32 getScale() const;
    f32 getAngleF() const;
    s16 getAngleS() const;
    u8 getParts() const;

public:
    dPanelObjList_c *mpPrev;
    dPanelObjList_c *mpNext;
    u16 mValue;
    u8 mType;
    u8 mChange;
    f32 mPosX;
    f32 mPosY;
    f32 mPosZ;
    f32 mScale;
    s16 mAngle;
    u8 mParts;
    u8 mPad1F;
};