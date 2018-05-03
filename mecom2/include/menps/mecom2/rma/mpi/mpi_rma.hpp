
#pragma once

#include <menps/mecom2/rma/mpi/basic_mpi_rma.hpp>
#include <menps/mecom2/rma/basic_unique_local_ptr.hpp>
#include <menps/mecom2/rma/mpi/basic_mpi_rma_handle.hpp>
#include <menps/mecom2/rma/rma_blocking_itf.hpp>
#include <menps/medev2/mpi.hpp>
#include <menps/mefdn/memory/distance_in_bytes.hpp>


namespace menps {
namespace mecom2 {

class mpi_rma;

class mpi_rma_handle;

struct mpi_rma_policy_base
{
    using mpi_itf_type = medev2::mpi::direct_requester;
    using ult_itf_type = default_ult_itf;
    
    using proc_id_type = int;
    using size_type = mefdn::size_t;
    
    using rma_sn_type = mefdn::ptrdiff_t;
    
    template <typename T>
    using remote_ptr = T*;
    
    template <typename T>
    using local_ptr = T*;
    
    template <typename Ptr>
    using element_type_of = mefdn::remove_pointer_t<Ptr>;
    
    using request_type = MPI_Request;
    
    template <typename T>
    static MPI_Datatype get_mpi_datatype() {
        return medev2::mpi::get_datatype<T>()();
    }
    
    static MPI_Aint to_mpi_aint(const void* const p) noexcept {
        return reinterpret_cast<MPI_Aint>(p);
    }
    
    static mefdn::intptr_t to_intptr(const void* const p) noexcept {
        return reinterpret_cast<mefdn::intptr_t>(p);
    }
};

struct mpi_rma_handle_policy : mpi_rma_policy_base
{
    using derived_type = mpi_rma_handle;
};

class mpi_rma_handle
    : public basic_mpi_rma_handle<mpi_rma_handle_policy>
{
    using policy_type = mpi_rma_handle_policy;
    
public:
    using mpi_itf_type = policy_type::mpi_itf_type;
    
    /*implicit*/ mpi_rma_handle(mpi_rma& rma)
        : rma_(rma)
    { }
    
    inline mpi_itf_type& get_mpi_interface();
    
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

struct mpi_rma_policy : mpi_rma_policy_base
{
    using derived_type = mpi_rma;
    
    template <typename T>
    using unique_local_ptr = mpi_unique_local_ptr<T>;
    
    template <typename U, typename T>
    static U* static_cast_to(T* const p) noexcept {
        return static_cast<U*>(p);
    }
    
    using handle_type = mpi_rma_handle;
};

class mpi_rma
    #if 1
    : public basic_mpi_rma<mpi_rma_policy>
    #else
    : public rma_blocking_itf<mpi_rma_policy>
    #endif
{
    using policy_type = mpi_rma_policy;
    
public:
    using mpi_itf_type = policy_type::mpi_itf_type;
    
    #if 1
    #else
    using rma_blocking_itf<mpi_rma_policy>::proc_id_type;
    // TODO: Remove this temporary solution
    #endif
    
    template <typename Conf>
    explicit mpi_rma(Conf&& conf)
        : req_(conf.req)
        , win_(conf.win)
    { }
    
    mpi_itf_type& get_mpi_interface() {
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
        const auto size_aint = static_cast<MPI_Aint>(size);
        this->req_.win_attach({ win_, p, size_aint });
        return p;
    }
    
    void untyped_deallocate(void* const p) {
        this->req_.win_detach({ win_, p });
        delete[] static_cast<mefdn::byte*>(p);
    }
    
    void* attach(void* const first, void* const last) {
        const auto size = mefdn::distance_in_bytes(first, last);
        this->req_.win_attach({ win_, first, size });
        return first;
    }
    
    void detach(void* const first) {
        this->req_.win_detach({ win_, first });
    }
    
    void progress()
    {
        MEFDN_LOG_VERBOSE("msg:Invoke MPI progress.");
        
        // Call an MPI function to forward the progress.
        this->req_.iprobe({ MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE });
        // TODO: Is this the best implementation?
    }
    
private:
    mpi_itf_type& req_;
    MPI_Win win_;
};



mpi_rma_handle::mpi_itf_type& mpi_rma_handle::get_mpi_interface() {
    return this->rma_.get_mpi_interface();
}

MPI_Win mpi_rma_handle::get_win() {
    return this->rma_.get_win();
}


using mpi_rma_ptr = mefdn::unique_ptr<mpi_rma>;

inline mpi_rma_ptr make_mpi_rma(mpi_rma::mpi_itf_type& req, MPI_Win win) {
    struct conf {
        mpi_rma::mpi_itf_type& req;
        MPI_Win win;
    };
    return mefdn::make_unique<mpi_rma>(conf{ req, win });
}

} // namespace mecom2
} // namespace menps

