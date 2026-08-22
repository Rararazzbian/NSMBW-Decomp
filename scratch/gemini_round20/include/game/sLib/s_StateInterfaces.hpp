#pragma once

class sStateIDIf_c {
public:
    virtual ~sStateIDIf_c() {}

    virtual bool isNull() const = 0;
    virtual bool isEqual(const sStateIDIf_c &other) const = 0;
    virtual int operator==(const sStateIDIf_c &other) const = 0;
    virtual int operator!=(const sStateIDIf_c &other) const = 0;
    virtual bool isSameName(const char *name) const = 0;
    virtual const char *name() const = 0;
    virtual unsigned int number() const = 0;
};

class sStateIf_c {
public:
    virtual ~sStateIf_c() {}
    virtual void initialize() = 0;
    virtual void execute() = 0;
    virtual void finalize() = 0;
};

class sStateFctIf_c {
public:
    virtual ~sStateFctIf_c() {}
    virtual sStateIf_c *build(sStateIDIf_c const &id) = 0;
    virtual void dispose(sStateIf_c *&id) = 0;
};

class sStateIDChkIf_c {
public:
    virtual ~sStateIDChkIf_c() {}
    virtual bool isNormalID(const sStateIDIf_c &id) const = 0;
};

class sStateMethodIf_c {
public:
    virtual ~sStateMethodIf_c() {}
};
