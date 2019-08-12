
#pragma once

#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mefdn {

class basic_unique_resource_access
{
public:
    template <typename Derived>
    static bool is_owned(Derived& self) {
        return self.is_owned();
    }
    
    template <typename Derived>
    static void set_owned(Derived& self) {
        self.set_owned();
    }
    
    template <typename Derived>
    static void set_unowned(Derived& self) {
        self.set_unowned();
    }
};

template <typename Policy>
class basic_unique_resource
{
    MEFDN_DEFINE_DERIVED(Policy)
    
    typedef typename Policy::resource_type  resource_type;
    typedef typename Policy::deleter_type   deleter_type;
    
public:
    basic_unique_resource()
        : t_{}
    { }
    
    basic_unique_resource(basic_unique_resource&& other) MEFDN_DEFAULT_NOEXCEPT
        : t_(other.release(), mefdn::move(other.get_deleter()))
    { 
        
    }
    
    basic_unique_resource(const basic_unique_resource&) = delete;
    
    basic_unique_resource(resource_type&& resource)
        : t_(mefdn::move(resource), deleter_type{})
    { }
    
protected:
    typedef typename mefdn::conditional<
        mefdn::is_reference<deleter_type>::value
    ,   deleter_type
    ,   const deleter_type&
    >::type
    const_ref_deleter_type;
    
    typedef typename mefdn::remove_reference<deleter_type>::type &&
    rv_ref_deleter_type;
    
public:
    basic_unique_resource(
        resource_type&&         resource
    ,   const_ref_deleter_type  deleter
    )
        : t_(resource, deleter)
    { }
    
    basic_unique_resource(
        resource_type&&     resource
    ,   rv_ref_deleter_type   deleter
    )
        : t_(resource, mefdn::move(deleter))
    { }
    
    ~basic_unique_resource()
    {
        this->reset();
    }
    
    basic_unique_resource& operator = (basic_unique_resource&& other)
    {
        this->reset(other.release());
        
        this->get_deleter() = mefdn::move(other.get_deleter());
        
        return *this;
    }
    
    basic_unique_resource& operator = (const basic_unique_resource&) = delete;
    
    deleter_type& get_deleter() noexcept {
        return mefdn::get<1>(t_);
    }
    const deleter_type& get_deleter() const noexcept {
        return mefdn::get<1>(t_);
    }
    
    explicit operator bool() const noexcept {
        return !!*this;
    }
    bool operator ! () const noexcept
    {
        auto& self = this->derived();
        
        return ! basic_unique_resource_access::is_owned(self);
    }
    
    void reset()
    {
        auto& self = this->derived();
        
        if (basic_unique_resource_access::is_owned(self))
        {
            // Destroy the resource.
            this->get_deleter()(mefdn::move(this->get_resource()));
            
            // Set this unowned because the resource was deleted.
            basic_unique_resource_access::set_unowned(self);
        }
    }
    
    void reset(resource_type&& new_resource)
    {
        this->reset();
        
        auto& self = this->derived();
        
        auto& resource = this->get_resource();
        
        // Move-assign the resource.
        resource = mefdn::move(new_resource);
        
        if (! basic_unique_resource_access::is_owned(self))
        {
            // If the resource was not owned,
            // mark this as owned now.
            // (Pointer-based implementations do nothing in set_owned().
            basic_unique_resource_access::set_owned(self);
        }
    }
    
    resource_type release()
    {
        auto ret = mefdn::move(this->get_resource());
        
        auto& self = this->derived();
        basic_unique_resource_access::set_unowned(self);
        
        return ret;
    }
    
    const resource_type& get() const noexcept
    {
        return this->get_resource();
    }
    
    const resource_type& operator -> () const noexcept
    {
        return get();
    }
    
    void swap(derived_type& other)
    {
        using std::swap;
        swap(this->t_, other.t_);
    }
    
protected:
    resource_type& get_resource() noexcept {
        return mefdn::get<0>(t_);
    }
    const resource_type& get_resource() const noexcept {
        return mefdn::get<0>(t_);
    }
    
private:
    mefdn::tuple<resource_type, deleter_type> t_;
};

} // namespace mefdn
} // namespace menps

