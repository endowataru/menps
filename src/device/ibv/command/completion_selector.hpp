
#pragma once

#include "completer.hpp"
#include <unordered_map>

namespace mgcom {
namespace ibv {

typedef mgbase::uint32_t    qp_num_t;

class completion_selector
{
public:
    completion_selector() = default;
    
    completion_selector(const completion_selector&) = delete;
    completion_selector& operator = (const completion_selector&) = delete;
    
    void set(const qp_num_t qp_num, completer& comp)
    {
        MGBASE_ASSERT(m_.find(qp_num) == m_.end());
        
        m_[qp_num] = &comp;
    }
    
    completer& get(const qp_num_t qp_num)
    {
        return *m_[qp_num];
    }
    
private:
    std::unordered_map<qp_num_t, completer*> m_;
};

} // namespace ibv
} // namespace mgcom

