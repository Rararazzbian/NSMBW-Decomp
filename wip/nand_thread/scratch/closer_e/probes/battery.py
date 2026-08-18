import os, sys
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, "tools", "auto_decomp"))
import harness

BASE = os.path.join(ROOT, "wip", "nand_thread", "scratch", "closer_e", "probes")

TEMPLATE = """
class Base {{
public:
{base_dtor_decl}
}};

class Derived : public Base {{
public:
{derived_dtor_decl}
    static int *m_instance;
}};

int *Derived::m_instance;

{derived_dtor_def}
"""

VARIANTS = {}

# --- baseline: reproduces the known-wrong (24-word) shape ---
VARIANTS['baseline'] = TEMPLATE.format(
    base_dtor_decl="    virtual ~Base();",
    derived_dtor_decl="    virtual ~Derived();",
    derived_dtor_def="Derived::~Derived() {\n    m_instance = 0;\n}\n")

# --- H2a: Base's destructor NOT virtual (probe only -- real vtable forbids this) ---
VARIANTS['base_dtor_nonvirtual'] = TEMPLATE.format(
    base_dtor_decl="    ~Base();",
    derived_dtor_decl="    virtual ~Derived();",
    derived_dtor_def="Derived::~Derived() {\n    m_instance = 0;\n}\n")

# --- H2b: Derived's own destructor NOT virtual (probe only) ---
VARIANTS['derived_dtor_nonvirtual'] = TEMPLATE.format(
    base_dtor_decl="    virtual ~Base();",
    derived_dtor_decl="    ~Derived();",
    derived_dtor_def="Derived::~Derived() {\n    m_instance = 0;\n}\n")

# --- H2c: Base's destructor defined INLINE with an empty body (transparent, not opaque) ---
VARIANTS['base_dtor_inline_empty'] = TEMPLATE.format(
    base_dtor_decl="    virtual ~Base() {}",
    derived_dtor_decl="    virtual ~Derived();",
    derived_dtor_def="Derived::~Derived() {\n    m_instance = 0;\n}\n")

# --- H3a: guarded store `if (this) m_instance = 0;` (CLOSE_C attempt 1, re-confirm) ---
VARIANTS['h3_guarded_store'] = TEMPLATE.format(
    base_dtor_decl="    virtual ~Base();",
    derived_dtor_decl="    virtual ~Derived();",
    derived_dtor_def="Derived::~Derived() {\n    if (this) {\n        m_instance = 0;\n    }\n}\n")

# --- H3b: store through explicitly-qualified static member ---
VARIANTS['h3_qualified_store'] = TEMPLATE.format(
    base_dtor_decl="    virtual ~Base();",
    derived_dtor_decl="    virtual ~Derived();",
    derived_dtor_def="Derived::~Derived() {\n    Derived::m_instance = 0;\n}\n")

# --- H3c: store through a helper inline function ---
VARIANTS['h3_helper_store'] = """
class Base {
public:
    virtual ~Base();
};

class Derived : public Base {
public:
    virtual ~Derived();
    static void clearInstance() { m_instance = 0; }
    static int *m_instance;
};

int *Derived::m_instance;

Derived::~Derived() {
    clearInstance();
}
"""

# --- H3d: self-store no-op dead code alongside the real store, testing 'this' pointer ---
VARIANTS['h3_selfstore_padding'] = TEMPLATE.format(
    base_dtor_decl="    virtual ~Base();",
    derived_dtor_decl="    virtual ~Derived();\n    int mState;",
    derived_dtor_def="Derived::~Derived() {\n    m_instance = 0;\n    mState = mState;\n}\n")

# --- H3e: two sequential static stores (mimics mError/mState double-clear idioms elsewhere) ---
VARIANTS['h3_two_static_stores'] = """
class Base {
public:
    virtual ~Base();
};

class Derived : public Base {
public:
    virtual ~Derived();
    static int *m_instance;
    static int s_flag;
};

int *Derived::m_instance;
int Derived::s_flag;

Derived::~Derived() {
    m_instance = 0;
    s_flag = 0;
}
"""


def run(name, src):
    path = os.path.join(BASE, "bat_%s.cpp" % name)
    obj = os.path.join(BASE, "bat_%s.o" % name)
    txt = os.path.join(BASE, "bat_%s.txt" % name)
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
    dtor = None
    for n, s in fns:
        if n.startswith("__dt__7DerivedFv"):
            dtor = (n, s)
    if not dtor:
        print("[%s] no Derived dtor found, fns=%s" % (name, fns))
        return
    body = harness.extract(txt, dtor[0])
    has_double_guard = False
    beq_count = sum(1 for l in body if l.startswith("beq"))
    print("[%s] size=0x%x words=%d beq_count=%d" % (name, dtor[1], len(body), beq_count))
    for l in body:
        print("      " + l)


if __name__ == "__main__":
    only = sys.argv[1:] if len(sys.argv) > 1 else list(VARIANTS.keys())
    for name in only:
        run(name, VARIANTS[name])
        print()
