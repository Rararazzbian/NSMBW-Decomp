
#define NULL 0
class Base {
public:
    Base(int x);
    virtual ~Base();
    virtual void *run() { return NULL; }
    virtual void onEnter() {}
    virtual void onExit() {}
};

class Derived : public Base {
public:
    Derived(int x);
    virtual ~Derived();
    virtual void *run();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void *Derived::run() { return (void *)1; }
