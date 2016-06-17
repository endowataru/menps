
#include "mpi_base.hpp"
#include "mpi_call.hpp"
#include "mpi_error.hpp"

#include <mgbase/logging/logger.hpp>
#include <mgbase/threading/this_thread.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

#include <mgbase/profiling/stopwatch.hpp>

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

std::string get_state()
{
    static index_t number = 0;
    
    fmt::MemoryWriter w;
    w.write(
        "proc:{}\tthread:{:x}\tlog_id:{}\tclock:{}\t"
    ,   current_process_id()
    ,   mgbase::this_thread::get_id().to_integer()
    ,   number++
    ,   mgbase::get_cpu_clock() // TODO
    );
    return w.str();
}

class mpi_endpoint
    : public mgcom::endpoint
{
public:
    mpi_endpoint(int* const argc, char*** const argv)
    {
        int provided;
        mpi_error::check(
            MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided)
        );
        
        int size, rank;
        mpi_error::check(MPI_Comm_size(MPI_COMM_WORLD, &size));
        mpi_error::check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
        
        this->set_current_process_id(static_cast<process_id_t>(rank));
        this->set_number_of_processes(static_cast<index_t>(size));
        
        mgbase::logger::set_state_callback(get_state);
        
        MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); // DEBUG
        
        endpoint::set_instance(*this); // TODO
        
        MGBASE_LOG_DEBUG("msg:Initialized MPI.");
    }
    
    virtual ~mpi_endpoint() MGBASE_NOEXCEPT
    {
        mpi_error::check(MPI_Finalize());
        
        MGBASE_LOG_DEBUG("msg:Finalized MPI.");
    }
};

} // unnamed namespace

mgbase::unique_ptr<endpoint> make_endpoint(int* const argc, char*** const argv)
{
    // TODO: replace with make_unique
    return mgbase::unique_ptr<endpoint>(new mpi_endpoint(argc, argv));
}

} // namespace mpi
} // namespace mgcom

