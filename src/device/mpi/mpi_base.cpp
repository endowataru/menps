
#include "mpi_base.hpp"
#include "mpi_error.hpp"

#include <mgbase/logging/logger.hpp>
#include <mgbase/threading/this_thread.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

std::string get_state()
{
    static index_t number = 0;
    
    fmt::MemoryWriter w;
    w.write(
        "proc:{}\tthread:{:x}\tlog_id:{}\t"
    ,   current_process_id()
    ,   mgbase::this_thread::get_id().to_integer()
    ,   number++
    );
    return w.str();
}

class impl
    : mgbase::noncopyable
{
public:
    void initialize(int* argc, char*** argv)
    {
        int provided;
        mpi_error::check(
            MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided)
        );
        
        int size, rank;
        mpi_error::check(MPI_Comm_size(MPI_COMM_WORLD, &size));
        mpi_error::check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
        
        current_process_id_ = static_cast<process_id_t>(rank);
        number_of_processes_ = static_cast<index_t>(size);
        
        mgbase::logger::set_state_callback(get_state);
        
        MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN); // DEBUG
    }
    
    void finalize()
    {
        mpi_error::check(MPI_Finalize());
    }
    
    process_id_t current_process_id() const MGBASE_NOEXCEPT {
        return current_process_id_;
    }
    index_t number_of_processes() const MGBASE_NOEXCEPT {
        return number_of_processes_;
    }
    
private:
    process_id_t current_process_id_;
    index_t number_of_processes_;
};

impl g_impl;

} // unnamed namespace

void initialize(int* argc, char*** argv)
{
    g_impl.initialize(argc, argv);
}

void finalize()
{
    g_impl.finalize();
}

}

process_id_t current_process_id() MGBASE_NOEXCEPT {
    return mpi::g_impl.current_process_id();
}

index_t number_of_processes() MGBASE_NOEXCEPT {
    return mpi::g_impl.number_of_processes();
}

} // namespace mgcom

