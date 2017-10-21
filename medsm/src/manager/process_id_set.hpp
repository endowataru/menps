
#pragma once

#include <menps/mecom/common.hpp>
#include <menps/mefdn/assert.hpp>
#include <unordered_set>

namespace menps {
namespace medsm {

template <typename T>
class id_set
{
    typedef std::unordered_set<T>   set_type;
    
public:
    typedef typename set_type::const_iterator   const_iterator;
    
    void insert(const T id)
    {
        MEFDN_ASSERT(!this->includes(id));
        ids_.insert(id);
    }
    
    void erase(const T id)
    {
        MEFDN_ASSERT(this->includes(id));
        ids_.erase(id);
    }
    
    bool empty() const noexcept
    {
        return ids_.empty();
    }
    
    bool includes(const T id) const noexcept
    {
        return ids_.find(id) != ids_.end();
    }
    
    bool is_only(const T id) const noexcept {
        return ids_.size() == 1
            && *ids_.begin() == id;
    }
    
    const_iterator begin() const noexcept {
        return ids_.begin();
    }
    const_iterator end() const noexcept {
        return ids_.end();
    }
    
    mefdn::size_t size() const noexcept {
        return ids_.size();
    }
    
private:
    set_type ids_;
};

typedef id_set<mecom::process_id_t> process_id_set;

} // namespace medsm
} // namespace menps

