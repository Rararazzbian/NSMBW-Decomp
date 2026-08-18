// Minimal reproduction of the EGG::Thread/dNandThread_c "run" weak-flush
// puzzle. Base has three inline virtuals (dtor aside); Derived overrides
// only one (run) and inherits the other two (onEnter/onExit) unchanged.
// Question: does the compiler weakly emit Base::run() even though it is
// overridden and never referenced?
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
    virtual void *run();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void *Derived::run() { return (void *)1; }
