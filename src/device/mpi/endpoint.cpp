
#include "mpi_base.hpp"
#include "mpi_error.hpp"

#include <mgbase/logging/logger.hpp>
#include <mgbase/threading/this_thread.hpp>
#include <mgbase/profiling/clock.hpp>

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

struct get_state
{
    get_state(mpi_endpoint& self)
        : self_(self)
        , number_{0}
        { }
    
    std::string operator() ()
    {
        fmt::MemoryWriter w;
        w.write(
            "proc:{}\tthread:{:x}\tlog_id:{}\tclock:{}\t"
        ,   self_.current_process_id()
        ,   mgbase::this_thread::get_id().to_integer()
        ,   number_++
        ,   mgbase::get_cpu_clock() // TODO
        );
        return w.str();
    }
    
private:
    mpi_endpoint& self_;
    index_t number_;
};

} // unnamed namespace

mpi_endpoint::mpi_endpoint(int* const argc, char*** const argv)
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
    
    mgbase::logger::set_state_callback(get_state{*this});
    
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); // DEBUG
    
    MGBASE_LOG_DEBUG("msg:Initialized MPI.");
}

mpi_endpoint::~mpi_endpoint()
{
    mpi_error::check(MPI_Finalize());
    
    MGBASE_LOG_DEBUG("msg:Finalized MPI.");
}

mgbase::unique_ptr<mpi_endpoint> make_endpoint(int* const argc, char*** const argv)
{
    return mgbase::make_unique<mpi_endpoint>(argc, argv);
}

} // namespace mpi
} // namespace mgcom

