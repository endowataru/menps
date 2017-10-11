
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <mgbase::size_t>
struct sized_unsigned_integer;

template <> struct sized_unsigned_integer<1> { typedef mgbase::uint8_t  type; };
template <> struct sized_unsigned_integer<2> { typedef mgbase::uint16_t type; };
template <> struct sized_unsigned_integer<4> { typedef mgbase::uint32_t type; };
template <> struct sized_unsigned_integer<8> { typedef mgbase::uint64_t type; };

} // namespace mgbase

