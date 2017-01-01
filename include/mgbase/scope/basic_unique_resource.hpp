
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/tuple/tuple.hpp>
#include <mgbase/type_traits/conditional.hpp>
#include <mgbase/type_traits/is_reference.hpp>
#include <mgbase/type_traits/is_pointer.hpp>
#include <mgbase/type_traits/remove_reference.hpp>
#include <mgbase/type_traits/remove_pointer.hpp>
#include <mgbase/type_traits/add_lvalue_reference.hpp>
#include <mgbase/explicit_operator_bool.hpp>

namespace mgbase {

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

namespace detail {

template <
    typename Traits
,   bool Dereferencable =
        // resource_type is a pointer
        mgbase::is_pointer<typename Traits::resource_type>::value
        &&
        // resource_type is not void*
        ! mgbase::is_void<
            typename mgbase::remove_pointer<typename Traits::resource_type>::type
        >::value
>
class basic_unique_resource_deref
    : public mgbase::crtp_base<Traits>
{ };

template <typename Traits>
class basic_unique_resource_deref<Traits, true>
    : public mgbase::crtp_base<Traits>
{
    typedef typename Traits::derived_type   derived_type;
    typedef typename Traits::resource_type  resource_type;
    
    typedef typename mgbase::add_lvalue_reference<
        typename mgbase::remove_pointer<resource_type>::type
    >::type
    lv_ref_resource_type;
    
public:
    lv_ref_resource_type operator * () const MGBASE_NOEXCEPT {
        return * this->derived().get();
    }
};

} // namespace detail

template <typename Traits>
class basic_unique_resource
    : public detail::basic_unique_resource_deref<Traits>
{
    typedef typename Traits::derived_type   derived_type;
    typedef typename Traits::resource_type  resource_type;
    typedef typename Traits::deleter_type   deleter_type;
    
public:
    basic_unique_resource()
        : t_{}
    { }
    
    basic_unique_resource(basic_unique_resource&& other)
        : t_(other.release(), mgbase::move(other.get_deleter()))
    { 
        
    }
    
    basic_unique_resource(const basic_unique_resource&) = delete;
    
    basic_unique_resource(resource_type&& resource)
        : t_(mgbase::move(resource), deleter_type{})
    { }
    
protected:
    typedef typename mgbase::conditional<
        mgbase::is_reference<deleter_type>::value
    ,   deleter_type
    ,   const deleter_type&
    >::type
    const_ref_deleter_type;
    
    typedef typename mgbase::remove_reference<deleter_type>::type &&
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
        : t_(resource, mgbase::move(deleter))
    { }
    
    ~basic_unique_resource()
    {
        this->reset();
    }
    
    basic_unique_resource& operator = (basic_unique_resource&& other)
    {
        this->reset(other.release());
        
        this->get_deleter() = mgbase::move(other.get_deleter());
        
        return *this;
    }
    
    basic_unique_resource& operator = (const basic_unique_resource&) = delete;
    
    deleter_type& get_deleter() MGBASE_NOEXCEPT {
        return mgbase::get<1>(t_);
    }
    const deleter_type& get_deleter() const MGBASE_NOEXCEPT {
        return mgbase::get<1>(t_);
    }
    
    MGBASE_EXPLICIT_OPERATOR_BOOL()
    
    bool operator ! () const MGBASE_NOEXCEPT
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
            this->get_deleter()(mgbase::move(this->get_resource()));
            
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
        resource = mgbase::move(new_resource);
        
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
        auto ret = mgbase::move(this->get_resource());
        
        auto& self = this->derived();
        basic_unique_resource_access::set_unowned(self);
        
        return ret;
    }
    
    const resource_type& get() const MGBASE_NOEXCEPT
    {
        return this->get_resource();
    }
    
    const resource_type& operator -> () const MGBASE_NOEXCEPT
    {
        return get();
    }
    
    void swap(derived_type& other)
    {
        using std::swap;
        swap(this->t_, other.t_);
    }
    
protected:
    resource_type& get_resource() MGBASE_NOEXCEPT {
        return mgbase::get<0>(t_);
    }
    const resource_type& get_resource() const MGBASE_NOEXCEPT {
        return mgbase::get<0>(t_);
    }
    
private:
    mgbase::tuple<resource_type, deleter_type> t_;
};

} // namespace mgbase

