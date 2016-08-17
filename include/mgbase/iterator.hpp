
#pragma once

#include <mgbase/lang.hpp>
#include <iterator>

namespace mgbase {

using std::input_iterator_tag;
using std::output_iterator_tag;
using std::forward_iterator_tag;
using std::bidirectional_iterator_tag;
using std::random_access_iterator_tag;

using std::next;
using std::prev;

using std::distance;

#ifdef MGBASE_CXX11_STD_BEGIN_SUPPORTED

using std::begin;

#else

template <typename C>
inline auto begin(C& c) -> decltype(c.begin()) {
    return c.begin();
}

#endif

#ifdef MGBASE_CXX11_STD_END_SUPPORTED

using std::end;

#else

template <typename C>
inline auto end(C& c) -> decltype(c.end()) {
    return c.end();
}

#endif

} // namespace mgbase

