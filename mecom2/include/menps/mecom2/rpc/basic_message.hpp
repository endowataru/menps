
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/scope/basic_unique_resource.hpp>
#include <menps/mefdn/basic_dereferencable.hpp>
#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
struct basic_message_policy : P {
    using resource_type = typename P::data_type *;
};

template <typename P, bool IsVoid>
class basic_message_header
{
    MEFDN_DEFINE_DERIVED(P)
    
    using header_type = typename P::header_type;
    
public:
    static mefdn::size_t header_size() noexcept {
        return sizeof(header_type);
    }
    
    header_type* header() const noexcept {
        auto& self = this->derived();
        const auto p = self.get();
        
        // The header is located before the data.
        const auto h = reinterpret_cast<header_type*>(p) - 1;
        return h;
    }
};

template <typename P>
class basic_message_header<P, true>
{
    MEFDN_DEFINE_DERIVED(P)
    
public:
    static mefdn::size_t header_size() noexcept {
        // Return 0 if the header is void (header-less).
        return 0;
    }
    
    void* header() const noexcept {
        auto& self = this->derived();
        const auto p = self.get();
        return p;
    }
};

template <typename P>
class basic_message
    : public mefdn::basic_unique_resource<basic_message_policy<P>>
    , public mefdn::basic_dereferencable<basic_message_policy<P>>
    , public basic_message_header<basic_message_policy<P>,
        mefdn::is_void<typename P::header_type>::value>
{
    using base = mefdn::basic_unique_resource<basic_message_policy<P>>;
    
public:
    using data_type = typename P::data_type;
    
    basic_message() /*noexcept (TODO)*/ = default;
    
    explicit basic_message(data_type* ptr, const mefdn::size_t size) noexcept
        : base(mefdn::move(ptr))
        , size_(size)
    { }
    
    basic_message(const basic_message&) = delete;
    basic_message& operator = (const basic_message&) = delete;
    
    basic_message(basic_message&&) /*noexcept*/ = default;
    basic_message& operator = (basic_message&&) /*noexcept*/ = default;
        // TODO: exception specification does not match
    
    mefdn::size_t size_in_bytes() const noexcept { return size_; }
    
    mefdn::size_t total_size_in_bytes() const noexcept {
        // Return the size including the header.
        return this->header_size() + size_;
    }
    
    template <typename U>
    typename P::template rebind<U> reinterpret_cast_to() &&
        // Note: This method "releases" *this.
    {
        return typename P::template rebind<U>(
            reinterpret_cast<U*>(
                // This class will not manage the resource any more.
                this->release()
            )
        ,   this->size_
        );
    }
    
private:
    friend mefdn::basic_unique_resource_access;
    
    bool is_owned() const noexcept {
        return this->get_resource();
    }
    
    void set_unowned() noexcept {
        this->get_resource() = nullptr;
    }
    void set_owned() noexcept {
        // do nothing
    }
    
    mefdn::size_t size_;
};

} // namespace mecom2
} // namespace menps

