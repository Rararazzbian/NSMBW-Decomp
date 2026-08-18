#include <game/bases/d_info.hpp>

unsigned long probe_pad11() { return (unsigned long)&((dInfo_c *)0)->pad11; }
unsigned long probe_pane() { return (unsigned long)&((dInfo_c *)0)->mScissorPane; }
unsigned long probe_drawinfo() { return (unsigned long)&((dInfo_c *)0)->mScissorDrawInfo; }
unsigned long probe_page() { return (unsigned long)&((dInfo_c *)0)->mCourseSelectPageNum; }
unsigned long probe_index() { return (unsigned long)&((dInfo_c *)0)->mCourseSelectIndexInPage; }
unsigned long probe_size() { return (unsigned long)sizeof(dInfo_c); }
