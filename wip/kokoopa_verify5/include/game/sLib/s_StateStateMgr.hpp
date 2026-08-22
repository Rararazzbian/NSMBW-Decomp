#pragma once
#include <game/sLib/s_StateMgr.hpp>

template <class T, template <class, class> class Manager, class Method1, class Method2>
class sStateStateMgr_c : public sStateMgrIf_c {
public:
    sStateStateMgr_c(T &owner, const sStateIDIf_c &initialState) :
        mainMgr(owner, initialState),
        subMgr(owner, initialState),
        currentMgr(&mainMgr) {}

    virtual void initializeState() { currentMgr->initializeState(); }
    virtual void finalizeState() {
        if (isSubState()) {
            returnState();
        } else {
            currentMgr->finalizeState();
        }
    }
    virtual void changeState(const sStateIDIf_c &newState) { currentMgr->changeState(newState); }
    virtual void refreshState() { currentMgr->refreshState(); }
    virtual void executeState() { currentMgr->executeState(); }

    virtual bool isState(const sStateIDIf_c &state) const { return *currentMgr->getStateID() == state; }

    virtual void changeToSubState(const sStateIDIf_c &newState) {
        currentMgr = &subMgr;
        currentMgr->changeState(newState);
    }

    virtual void returnState() {
        if (isSubState()) {
            currentMgr->finalizeState();
            currentMgr = &mainMgr;
        }
    }

    virtual bool isSubState() const { return currentMgr == &subMgr; }

    virtual const sStateIDIf_c *getMainStateID() const { return mainMgr.getStateID(); }

    virtual sStateIf_c *getState() const { return currentMgr->getState(); }
    virtual const sStateIDIf_c *getNewStateID() const { return currentMgr->getNewStateID(); }
    virtual const sStateIDIf_c *getStateID() const { return currentMgr->getStateID(); }
    virtual const sStateIDIf_c *getOldStateID() const { return currentMgr->getOldStateID(); }

private:
    Manager<T, Method1> mainMgr;
    Manager<T, Method2> subMgr;
    sStateMgrIf_c *currentMgr;
};
