#include <game/bases/d_actor.hpp>
#include <cstddef>

template <int S> struct SizeTest;
SizeTest<offsetof(dActor_c, mCc)> test_mCc;
SizeTest<offsetof(dActor_c, mBc)> test_mBc;
SizeTest<offsetof(dActor_c, mRc)> test_mRc;
