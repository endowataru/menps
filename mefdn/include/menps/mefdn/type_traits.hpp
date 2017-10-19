
#pragma once

#include <menps/mefdn/lang.hpp>
#include <type_traits>

#include <menps/mefdn/type_traits/integer_sequence.hpp>

namespace menps {
namespace mefdn {

using std::is_convertible;
using std::remove_const;
using std::remove_extent;
using std::remove_cv;
using std::result_of;
using std::decay;
using std::is_trivially_copyable;

} // namespace mefdn
} // namespace menps

