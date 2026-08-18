
class Base {
public:
    Base(int x);
    virtual ~Base();
    virtual void *run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}
};

class Derived : public Base {
public:
    Derived(int x);
    virtual ~Derived();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
