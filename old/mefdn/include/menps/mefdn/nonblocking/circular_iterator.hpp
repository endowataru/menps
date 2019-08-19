
#pragma once

#include <menps/mefdn/iterator_facade.hpp>

namespace menps {
namespace mefdn {

template <typename Derived, typename T>
class circular_iterator
    : public iterator_facade<
        circular_iterator<Derived, T>
    ,   T
    ,   mefdn::random_access_iterator_tag
    >
{
protected:
    explicit circular_iterator(const mefdn::size_t index)
        : index_{index} { }
    
private:
    friend class mefdn::iterator_core_access;
    
    T& dereference() const noexcept {
        const auto i = index_ % size();
        return derived().at(i);
    }
    
    void increment() {
        advance(1);
    }
    
    void decrement() {
        advance(-1);
    }
    
    void advance(const mefdn::ptrdiff_t diff)
    {
        index_ = static_cast<mefdn::size_t>(
            static_cast<mefdn::ptrdiff_t>(index_) + diff
        );
    }
    
    bool equal(const circular_iterator& other) const noexcept
    {
        return index_ == other.index_;
    }
    
    mefdn::ptrdiff_t distance_to(const circular_iterator& other) const noexcept
    {
        const auto self_index = static_cast<mefdn::ptrdiff_t>(index_);
        const auto other_index = static_cast<mefdn::ptrdiff_t>(other.index_);
        
        return other_index - self_index;
    }
    
    mefdn::size_t size() const noexcept {
        return derived().size();
    }
    
    Derived& derived() noexcept {
        return static_cast<Derived&>(*this);
    }
    const Derived& derived() const noexcept {
        return static_cast<const Derived&>(*this);
    }
    
    mefdn::size_t index_;
};

} // namespace mefdn
} // namespace menps

