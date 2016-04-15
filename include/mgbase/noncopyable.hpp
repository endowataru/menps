
#pragma once

#include <mgbase/lang.hpp>

// Reference: Boost.Core

namespace mgbase {

// "ADL firewall"
namespace noncopyable_ {

#ifdef MGBASE_CXX11_SUPPORTED

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator = (const noncopyable) = delete;

protected:
    noncopyable() noexcept = default;
};

#else // MGBASE_CXX11_SUPPORTED

class noncopyable {
private:
    noncopyable(const noncopyable&);
    noncopyable& operator = (const noncopyable&);

protected:
    noncopyable() { }
};

#endif // MGBASE_CXX11_SUPPORTED

} // namespace noncopyable_

typedef noncopyable_::noncopyable   noncopyable;

} // naemspace mgbase

