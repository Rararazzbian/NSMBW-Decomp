#include <game/bases/d_heap_allocator.hpp>
struct Probe { char c; dHeapAllocator_c a; };
int probe2() { return (int)((char*)&((Probe*)0)->a - (char*)0); }
