
#pragma once

#include <menps/mecom2/rma/rma_typed_allocator.hpp>
#include <menps/mecom2/rma/basic_unique_local_ptr.hpp>
#include <menps/mecom2/rma/mpi/basic_mpi_rma_handle.hpp>
#include <menps/mecom2/rma/rma_blocking_itf.hpp>
#include <menps/medev2/mpi.hpp>

namespace menps {
namespace mecom2 {

class mpi_rma;

class mpi_rma_handle;

struct mpi_rma_handle_policy
{
    using derived_type = mpi_rma_handle;
    using size_type = mefdn::size_t;
    using process_id_type = int;
    
    template <typename T>
    using remote_ptr = T*;
    
    template <typename T>
    using local_ptr = T*;
    
    template <typename Ptr>
    using element_type_of = mefdn::remove_pointer_t<Ptr>;
};

class mpi_rma_handle
    : public basic_mpi_rma_handle<mpi_rma_handle_policy>
{
public:
    /*implicit*/ mpi_rma_handle(mpi_rma& rma)
        : rma_(rma)
    { }
    
    inline medev2::mpi::direct_requester& get_mpi_interface();
    
    inline MPI_Win get_win();
    
private:
    mpi_rma& rma_;
};

class mpi_rma;

template <typename T>
struct mpi_unique_local_ptr_policy {
    using derived_type = basic_unique_local_ptr<mpi_unique_local_ptr_policy>;
    //using resource_type = T*;
    using resource_type = mefdn::remove_extent_t<T> *; // TODO
    
    using deleter_type = unique_local_ptr_deleter<mpi_unique_local_ptr_policy>;
    
    using allocator_type = mpi_rma;
};

template <typename T>
using mpi_unique_local_ptr = basic_unique_local_ptr<mpi_unique_local_ptr_policy<T>>;

struct mpi_rma_policy
{
    using derived_type = mpi_rma;
    using size_type = mefdn::size_t;
    using process_id_type = int;
    
    template <typename T>
    using remote_ptr = T*;
    
    template <typename T>
    using local_ptr = T*;
    
    template <typename T>
    using unique_local_ptr = mpi_unique_local_ptr<T>;
    
    template <typename U, typename T>
    static U* static_cast_to(T* const p) noexcept {
        return static_cast<U*>(p);
    }
};

class mpi_rma
    : public rma_blocking_itf<mpi_rma_policy>
    , public rma_typed_allocator<mpi_rma_policy>
{
public:
    template <typename Conf>
    explicit mpi_rma(Conf&& conf)
        : req_(conf.req)
        , win_(conf.win)
    { }
    
    medev2::mpi::direct_requester& get_mpi_interface() {
        return req_;
    }
    
    MPI_Win get_win() {
        return win_;
    }
    
    mpi_rma_handle make_handle() {
        return mpi_rma_handle(*this);
    }
    
    void* untyped_allocate(const mefdn::size_t size) {
        const auto p = new mefdn::byte[size];
        this->req_.win_attach({ win_, p, size });
        return p;
    }
    
    void untyped_deallocate(void* const p) {
        this->req_.win_detach({ win_, p });
        delete[] static_cast<mefdn::byte*>(p);
    }
    
private:
    medev2::mpi::direct_requester& req_;
    MPI_Win win_;
};



medev2::mpi::direct_requester& mpi_rma_handle::get_mpi_interface() {
    return this->rma_.get_mpi_interface();
}

MPI_Win mpi_rma_handle::get_win() {
    return this->rma_.get_win();
}


inline mpi_rma make_mpi_rma(medev2::mpi::direct_requester& req, MPI_Win win) {
    struct conf {
        medev2::mpi::direct_requester& req;
        MPI_Win win;
    };
    return mpi_rma(conf{ req, win });
}

} // namespace mecom2
} // namespace menps

