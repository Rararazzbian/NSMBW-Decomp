#include <game/bases/d_nand_thread.hpp>
int probeDerived(dNandThread_c *t) {
    bool ok1 = (t->mError == 0);
    if (!ok1) {
        return !ok1;
    }
    t->mFileExists = true;
    return 0;
}
