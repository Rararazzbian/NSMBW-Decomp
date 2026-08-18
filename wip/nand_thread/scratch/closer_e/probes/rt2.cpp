// Two base constructor overloads, only one used, matching EGG::Thread's
// real shape (Thread(unsigned long,int,int,Heap*) and Thread(OSThread*,int)).
class Heap;
class Base {
public:
    Base(unsigned long a, int b, int c, Heap *d);
    Base(void *osthread, int e);
    virtual ~Base();
    virtual void *run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}
    unsigned char mPad[0x48];
};

class Derived : public Base {
public:
    Derived(int msgCount, Heap *heap);
    virtual ~Derived();
    virtual void *run();
    static void create(Heap *heap);
};

Derived::Derived(int msgCount, Heap *heap) : Base(0x4000, 0, msgCount, heap) {}
Derived::~Derived() {}
void *Derived::run() { return (void *)1; }
void Derived::create(Heap *heap) {
    Derived *d = new Derived(0, heap);
}
