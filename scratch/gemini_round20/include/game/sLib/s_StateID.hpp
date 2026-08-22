#pragma once
#include <game/sLib/s_StateInterfaces.hpp>

class sStateID_c : public sStateIDIf_c {
public:
    class NumberMemo_c {
    public:
        NumberMemo_c() : curr(0) {}
        unsigned int get() {
            curr++;
            return curr;
        }
        unsigned int curr;
    };

    sStateID_c(const char *name);
    virtual ~sStateID_c();

    virtual bool isNull() const;
    virtual bool isEqual(const sStateIDIf_c &other) const;
    virtual int operator==(const sStateIDIf_c &other) const;
    virtual int operator!=(const sStateIDIf_c &other) const;
    virtual bool isSameName(const char *name) const;

    virtual const char *name() const;
    virtual unsigned int number() const;

protected:
    const char *mpName;
    unsigned int mNumber;

    static NumberMemo_c sm_numberMemo;
};

namespace sStateID {
    extern sStateID_c null;
}
