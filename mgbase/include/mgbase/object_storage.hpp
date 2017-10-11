
#pragma once

#include <mgbase/utility.hpp>
#include <type_traits>

namespace mgult {

template <typename T>
class object_storage
{
public:
    template <typename... Args>
    void construct(Args&&... args)
    {
        new (&s_) T(mgbase::forward<Args>(args)...);
    }
    
    void destruct()
    {
        get().~T();
    }
    
    T& get() MGBASE_NOEXCEPT {
        return reinterpret_cast<T&>(s_);
    }
    const T& get() const MGBASE_NOEXCEPT {
        return reinterpret_cast<const T&>(s_);
    }
    
private:
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type   storage_type;
    storage_type s_;
};

} // namespace mgult

