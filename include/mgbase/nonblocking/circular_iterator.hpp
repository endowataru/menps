
#pragma once

#include <mgbase/iterator_facade.hpp>

namespace mgbase {

template <typename Derived, typename T>
class circular_iterator
    : public iterator_facade<
        circular_iterator<Derived, T>
    ,   T
    ,   mgbase::random_access_iterator_tag
    >
{
protected:
    explicit circular_iterator(const mgbase::size_t index)
        : index_{index} { }
    
private:
    friend class mgbase::iterator_core_access;
    
    T& dereference() const MGBASE_NOEXCEPT {
        const auto i = index_ % size();
        return derived().at(i);
    }
    
    void increment() {
        advance(1);
    }
    
    void decrement() {
        advance(-1);
    }
    
    void advance(const mgbase::ptrdiff_t diff)
    {
        index_ = static_cast<mgbase::size_t>(
            static_cast<mgbase::ptrdiff_t>(index_) + diff
        );
    }
    
    bool equal(const circular_iterator& other) const MGBASE_NOEXCEPT
    {
        return index_ == other.index_;
    }
    
    mgbase::ptrdiff_t distance_to(const circular_iterator& other) const MGBASE_NOEXCEPT
    {
        const auto self_index = static_cast<mgbase::ptrdiff_t>(index_);
        const auto other_index = static_cast<mgbase::ptrdiff_t>(other.index_);
        
        return other_index - self_index;
    }
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return derived().size();
    }
    
    Derived& derived() MGBASE_NOEXCEPT {
        return static_cast<Derived&>(*this);
    }
    const Derived& derived() const MGBASE_NOEXCEPT {
        return static_cast<const Derived&>(*this);
    }
    
    mgbase::size_t index_;
};

} // namespace mgbase

