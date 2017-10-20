
#pragma once

#include <menps/mefdn/lang.hpp>
#include <type_traits>

namespace menps {
namespace mefdn {

using std::is_convertible;
using std::remove_const;
using std::remove_extent;
using std::remove_cv;
using std::result_of;
using std::decay;
using std::is_reference;
using std::conditional;
using std::remove_reference;
using std::is_integral;
using std::is_const;
using std::enable_if;
using std::is_compound;
using std::is_array;
using std::is_pointer;
using std::is_enum;
using std::is_same;
using std::is_void;
using std::remove_pointer;
using std::add_lvalue_reference;

} // namespace mefdn
} // namespace menps

