
#pragma once

#include <mgcom.hpp>

namespace mgcom {

struct mpi_error { };

namespace {

class mpi_base
    : mgbase::noncopyable
{
protected:
    void initialize(int* argc, char*** argv)
    {
        int provided;
        throw_if_error(::MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided));
        
        int size, rank;
        throw_if_error(::MPI_Comm_size(MPI_COMM_WORLD, &size));
        throw_if_error(::MPI_Comm_rank(MPI_COMM_WORLD, &rank));
        
        current_process_id_ = static_cast<process_id_t>(rank);
        number_of_processes_ = static_cast<index_t>(size);
    }
    
    void finalize()
    {
        throw_if_error(::MPI_Finalize());
    }
    
    void throw_if_error(int err) {
        if (err != MPI_SUCCESS)
            throw mpi_error();
    }
    
public:
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

}

}


