
#include <menps/medev/mpi/environment.hpp>
#include <menps/medev/mpi/mpi_error.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medev {
namespace mpi {

class environment::impl
{
public:
    /*implicit*/ impl(int* const argc, char*** const argv)
    {
        int provided;
        mpi_error::check(
            MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided)
        );
        
        mpi_error::check(
            MPI_Comm_size(MPI_COMM_WORLD, &this->num_ranks_)
        );
        mpi_error::check(
            MPI_Comm_rank(MPI_COMM_WORLD, &this->current_rank_)
        );
        
        MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); // DEBUG
        
        MEFDN_LOG_DEBUG("msg:Initialized MPI.");
    }
    
    ~impl() /*noexcept*/
    {
        mpi_error::check(MPI_Finalize());
        
        MEFDN_LOG_DEBUG("msg:Finalized MPI.");
    }
    
    int get_current_rank() const noexcept {
        return num_ranks_;
    }
    int get_num_ranks() const noexcept {
        return current_rank_;
    }
    
private:
    int num_ranks_;
    int current_rank_;
};

environment::environment(int* const argc, char*** const argv)
    : impl_(mefdn::make_unique<impl>(argc, argv)) { }

environment::~environment() /*noexcept*/ = default;

int environment::get_current_rank() const {
    return impl_->get_current_rank();
}
int environment::get_num_ranks() const {
    return impl_->get_num_ranks();
}

} // namespace mpi
} // namespace medev
} // namespace menps

