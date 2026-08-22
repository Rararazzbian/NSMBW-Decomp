#pragma once
#include <game/sLib/s_StateInterfaces.hpp>
#include <game/sLib/s_StateIDChk.hpp>

class sStateMgrIf_c {
public:
    virtual ~sStateMgrIf_c() {}
    virtual void initializeState() = 0;
    virtual void finalizeState() = 0;
    virtual void changeState(const sStateIDIf_c &newStateID) = 0;
    virtual void refreshState() = 0;
    virtual void executeState() = 0;
    virtual sStateIf_c *getState() const = 0;
    virtual const sStateIDIf_c *getNewStateID() const = 0;
    virtual const sStateIDIf_c *getStateID() const = 0;
    virtual const sStateIDIf_c *getOldStateID() const = 0;
};

template <class T, class Method, template <class> class Factory, class Check>
class sStateMgr_c : public sStateMgrIf_c {
public:
    sStateMgr_c(T &owner, const sStateIDIf_c &initialState) :
        mFactory(owner),
        mMethod(mCheck, mFactory, initialState) {}

    virtual void initializeState() { mMethod.initializeStateMethod(); }
    virtual void finalizeState() { mMethod.finalizeStateMethod(); }
    virtual void changeState(const sStateIDIf_c &newState) { mMethod.changeStateMethod(newState); }
    virtual void refreshState() { mMethod.refreshStateMethod(); }
    virtual void executeState() { mMethod.executeStateMethod(); }

    virtual bool isState(const sStateIDIf_c &state) const { return *getStateID() == state; }

    virtual sStateIf_c *getState() const { return mMethod.getState(); }
    virtual const sStateIDIf_c *getNewStateID() const { return mMethod.getNewStateID(); }
    virtual const sStateIDIf_c *getStateID() const { return mMethod.getStateID(); }
    virtual const sStateIDIf_c *getOldStateID() const { return mMethod.getOldStateID(); }

private:
    Check mCheck;
    Factory<T> mFactory;
    Method mMethod;
};
