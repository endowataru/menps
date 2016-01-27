
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

struct adopt_lock_t { };
extern adopt_lock_t adopt_lock; // never defined

struct defer_lock_t { };
extern defer_lock_t defer_lock; // never defined

struct try_to_lock_t { };
extern try_to_lock_t try_to_lock; // never defined

} // namespace mgbase


