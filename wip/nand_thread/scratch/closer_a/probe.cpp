#include <game/bases/d_nand_thread.hpp>

// J: direct (non-bool) chained comparisons on volatile mError -- does MWCC
// reuse the load for the second, un-materialized comparison?
s32 probeJ(dNandThread_c *t, void *info) {
    if (t->mError != 0) {
        if (t->mError == 6) {
            return 3;
        }
        return 1;
    }
    return 0;
}
