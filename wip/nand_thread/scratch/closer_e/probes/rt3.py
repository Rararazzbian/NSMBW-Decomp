import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

BASE = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e", "probes")

VARIANTS = {}

# Override the LAST slot (onExit) instead of the first (run)
VARIANTS['override_last_slot'] = """
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
    virtual void onExit();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void Derived::onExit() {}
"""

# Override the MIDDLE slot (onEnter) only
VARIANTS['override_middle_slot'] = """
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
    virtual void onEnter();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void Derived::onEnter() {}
"""

# Override run AND onEnter, leave only onExit inherited
VARIANTS['override_two_of_three'] = """
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
    virtual void onEnter();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void *Derived::run() { return (void*)1; }
void Derived::onEnter() {}
"""

# Override ALL THREE
VARIANTS['override_all_three'] = """
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
    virtual void onEnter();
    virtual void onExit();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void *Derived::run() { return (void*)1; }
void Derived::onEnter() {}
void Derived::onExit() {}
"""

def run(name, src):
    path = os.path.join(BASE, "rt3_%s.cpp" % name)
    obj = os.path.join(BASE, "rt3_%s.o" % name)
    txt = os.path.join(BASE, "rt3_%s.txt" % name)
    open(path, "w", encoding="utf-8").write(src)
    ok, log = harness.compile_draft(path, obj)
    if not ok:
        print("[%s] COMPILE FAILED" % name); print(log[:600]); return
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print("[%s] DISASM FAILED: %s" % (name, dlog[:300])); return
    names = [n for n, s in harness.list_functions(txt, with_size=True)]
    print("[%-25s] %s" % (name, names))

if __name__ == "__main__":
    for name in VARIANTS:
        run(name, VARIANTS[name])
