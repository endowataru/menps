
#pragma once

#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace mpi {

class environment
{
public:
    explicit environment(int* argc, char*** argv);
    
    ~environment() /*noexcept*/;
    
    int get_current_rank() const;
    
    int get_num_ranks() const;
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace medev
} // namespace menps

