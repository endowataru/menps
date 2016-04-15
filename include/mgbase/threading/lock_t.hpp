
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_CXX11_SUPPORTED

#include <mutex>

namespace mgbase {

using std::adopt_lock_t;
using std::defer_lock_t;
using std::try_to_lock_t;

using std::adopt_lock;
using std::defer_lock;
using std::try_to_lock;

} // namespace mgbase

#else // MGBASE_CXX11_SUPPORTED

namespace mgbase {

struct adopt_lock_t { };

struct defer_lock_t { };

struct try_to_lock_t { };

namespace /*unnamed*/ {

MGBASE_UNUSED adopt_lock_t adopt_lock;
MGBASE_UNUSED defer_lock_t defer_lock;
MGBASE_UNUSED try_to_lock_t try_to_lock;

} // unnamed namespace

// extern adopt_lock_t adopt_lock; // never defined
// extern defer_lock_t defer_lock; // never defined
// extern try_to_lock_t try_to_lock; // never defined

} // namespace mgbase

#endif // MGBASE_CXX11_SUPPORTED

