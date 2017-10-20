
#include "mpi_base.hpp"
#include "mpi_error.hpp"

#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/thread/this_thread.hpp>
#include <menps/mefdn/profiling/clock.hpp>

#include <unistd.h>
#include <string>

namespace menps {
namespace mecom {
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
        ,   reinterpret_cast<mefdn::uintptr_t>(pthread_self())
            // TODO: use mefdn::this_thread::get_id()
        ,   number_++
        ,   mefdn::get_cpu_clock() // TODO
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
    
    mefdn::logger::set_state_callback(get_state{*this});
    
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); // DEBUG
    
    static const mefdn::size_t len = 128;
    char hostname[len];
    gethostname(hostname, len);
    
    MEFDN_LOG_DEBUG(
        "msg:Initialized MPI.\t"
        "host:{}"
    ,   hostname
    );
}

mpi_endpoint::~mpi_endpoint()
{
    mpi_error::check(MPI_Finalize());
    
    MEFDN_LOG_DEBUG("msg:Finalized MPI.");
}

mefdn::unique_ptr<mpi_endpoint> make_endpoint(int* const argc, char*** const argv)
{
    return mefdn::make_unique<mpi_endpoint>(argc, argv);
}

} // namespace mpi

endpoint* endpoint::endpoint_; // TODO

} // namespace mecom
} // namespace menps

