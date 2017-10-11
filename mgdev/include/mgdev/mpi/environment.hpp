
#pragma once

#include <mgbase/unique_ptr.hpp>

namespace mgdev {
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
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace mgdev

