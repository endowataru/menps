
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/assert.hpp>
#include <unordered_set>

namespace mgdsm {

template <typename T>
class id_set
{
    typedef std::unordered_set<T>   set_type;
    
public:
    typedef typename set_type::const_iterator   const_iterator;
    
    void insert(const T id)
    {
        MGBASE_ASSERT(!this->includes(id));
        ids_.insert(id);
    }
    
    void erase(const T id)
    {
        MGBASE_ASSERT(this->includes(id));
        ids_.erase(id);
    }
    
    bool empty() const MGBASE_NOEXCEPT
    {
        return ids_.empty();
    }
    
    bool includes(const T id) const MGBASE_NOEXCEPT
    {
        return ids_.find(id) != ids_.end();
    }
    
    bool is_only(const T id) const MGBASE_NOEXCEPT {
        return ids_.size() == 1
            && *ids_.begin() == id;
    }
    
    const_iterator begin() const MGBASE_NOEXCEPT {
        return ids_.begin();
    }
    const_iterator end() const MGBASE_NOEXCEPT {
        return ids_.end();
    }
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return ids_.size();
    }
    
private:
    set_type ids_;
};

typedef id_set<mgcom::process_id_t> process_id_set;

} // namespace mgdsm

