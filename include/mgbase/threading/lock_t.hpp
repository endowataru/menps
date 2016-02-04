
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

struct adopt_lock_t { };

struct defer_lock_t { };

struct try_to_lock_t { };

namespace /*unnamed*/ {

adopt_lock_t adopt_lock;
defer_lock_t defer_lock;
try_to_lock_t try_to_lock;

} // unnamed namespace

// extern adopt_lock_t adopt_lock; // never defined
// extern defer_lock_t defer_lock; // never defined
// extern try_to_lock_t try_to_lock; // never defined

} // namespace mgbase


