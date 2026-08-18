import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

BASE = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e", "probes")

VARIANTS = {}

VARIANTS['baseline'] = """
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
"""

# run() body NOT trivially foldable
VARIANTS['nonfoldable_body'] = """
static int g_counter;
class Base {
public:
    Base(int x);
    virtual ~Base();
    virtual void *run() { g_counter++; return &g_counter; }
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
"""

# override defined inline in the class body instead of out-of-line
VARIANTS['override_inline_in_class'] = """
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
    virtual void *run() { return (void *)1; }
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
"""

# override NOT marked 'virtual' explicitly (still virtual via inheritance)
VARIANTS['override_implicit_virtual'] = """
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
    void *run();
};

Derived::Derived(int x) : Base(x) {}
Derived::~Derived() {}
void *Derived::run() { return (void *)1; }
"""

# Base's constructor defined INLINE instead of external
VARIANTS['base_ctor_inline'] = """
class Base {
public:
    Base(int x) {}
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
"""

# Base has an additional inline NON-virtual member that Derived's ctor calls
VARIANTS['base_inline_nonvirtual_used'] = """
class Base {
public:
    Base(int x);
    virtual ~Base();
    virtual void *run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}
    void helper() {}
};

class Derived : public Base {
public:
    Derived(int x);
    virtual ~Derived();
    virtual void *run();
};

Derived::Derived(int x) : Base(x) { helper(); }
Derived::~Derived() {}
void *Derived::run() { return (void *)1; }
"""

# Derived does NOT override run() at all (sanity check -- should flush all 3)
VARIANTS['no_override_sanity'] = """
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
"""

# override calls the base version explicitly (as part of its body) -- NOT
# representative of the real target (no such call exists there) but useful
# to confirm what DOES trigger the flush, for contrast.
VARIANTS['override_calls_base_explicitly'] = """
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
void *Derived::run() { Base::run(); return (void *)1; }
"""

# Base::run() returns NULL instead of 0 (brief's 3rd knob, weak test)
VARIANTS['returns_null'] = """
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
"""


def run(name, src):
    path = os.path.join(BASE, "rt_%s.cpp" % name)
    obj = os.path.join(BASE, "rt_%s.o" % name)
    txt = os.path.join(BASE, "rt_%s.txt" % name)
    open(path, "w", encoding="utf-8").write(src)
    ok, log = harness.compile_draft(path, obj)
    if not ok:
        print("[%s] COMPILE FAILED" % name)
        print(log[:600])
        return
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print("[%s] DISASM FAILED: %s" % (name, dlog[:300]))
        return
    fns = harness.list_functions(txt, with_size=True)
    names = [n for n, s in fns]
    has_run_flush = any(n.startswith("run__4BaseFv") for n in names)
    has_onenter = any(n.startswith("onEnter__4BaseFv") for n in names)
    has_onexit = any(n.startswith("onExit__4BaseFv") for n in names)
    print("[%-30s] run_flush=%s onEnter=%s onExit=%s  all_fns=%s" % (
        name, has_run_flush, has_onenter, has_onexit, names))


if __name__ == "__main__":
    only = sys.argv[1:] if len(sys.argv) > 1 else list(VARIANTS.keys())
    for name in only:
        run(name, VARIANTS[name])
