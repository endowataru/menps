
#pragma once

#include <menps/mecom2/rma/rma_itf_id.hpp>
#include <menps/mecom2/rma/mpi/basic_mpi_rma.hpp>
#include <menps/mecom2/rma/basic_unique_public_ptr.hpp>
#include <menps/mecom2/rma/rma_private_heap_alloc.hpp>
#include <menps/medev2/mpi.hpp>
#include <menps/mefdn/memory/distance_in_bytes.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class mpi_rma
    : public basic_mpi_rma<P>
    , public rma_private_heap_alloc<P>
{
    using mpi_itf_type = typename P::mpi_itf_type;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
public:
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename U, typename T>
    static U* member(T* const p, U (T::* const q)) noexcept {
        return &(p->*q);
    }
    
    template <typename Conf>
    explicit mpi_rma(Conf&& conf)
        : mf_(conf.req)
        , win_(conf.win)
        , comm_(conf.comm)
        , rank_(0)
    {
        this->mf_.comm_rank({ this->comm_, &this->rank_ });
    }
    
    mpi_facade_type& get_mpi_facade() {
        return this->mf_;
    }
    
    MPI_Win get_win() {
        return win_;
    }
    
    bool is_local_proc(const int rank) const noexcept {
        return this->rank_ == rank;
    }
    
    void* untyped_allocate(const mefdn::size_t size) {
        const auto p = new mefdn::byte[size];
        const auto size_aint = static_cast<MPI_Aint>(size);
        this->mf_.win_attach({ win_, p, size_aint });
        return p;
    }
    
    void untyped_deallocate(void* const p) {
        this->mf_.win_detach({ win_, p });
        delete[] static_cast<mefdn::byte*>(p);
    }
    
    template <typename T>
    T* attach(T* const first, T* const last) {
        const auto size = mefdn::distance_in_bytes(first, last);
        this->mf_.win_attach({ win_, first, size });
        return first;
    }
    
    void detach(void* const first) {
        this->mf_.win_detach({ win_, first });
    }
    
    size_type serialized_size_in_bytes(void* /*ptr*/) {
        return sizeof(void*);
    }
    void serialize(void* const ptr, void* const buf) {
        *reinterpret_cast<void**>(buf) = ptr;
    }
    template <typename T>
    T* deserialize(proc_id_type /*proc*/, const void* const buf) {
        return *reinterpret_cast<T* const *>(buf);
    }
    
    void progress()
    {
        this->mf_.progress();
    }
    
private:
    mpi_facade_type& mf_;
    MPI_Win win_;
    MPI_Comm comm_;
    int rank_;
};

template <typename P>
using mpi_rma_ptr = mefdn::unique_ptr<mpi_rma<P>>;

template <typename P>
inline mpi_rma_ptr<P> make_mpi_rma(
    typename P::mpi_itf_type::mpi_facade_type&  req
,   const MPI_Win                               win
,   const MPI_Comm                              comm
) {
    struct conf {
        typename P::mpi_itf_type::mpi_facade_type&  req;
        MPI_Win                                     win;
        MPI_Comm                                    comm;
    };
    return mefdn::make_unique<mpi_rma<P>>(conf{ req, win, comm });
}



template <typename MpiItf>
struct mpi_rma_policy;

template <typename MpiItf, typename T>
struct mpi_unique_public_ptr_policy {
    using derived_type = basic_unique_public_ptr<mpi_unique_public_ptr_policy>;
    //using resource_type = T*;
    using resource_type = mefdn::remove_extent_t<T> *; // TODO
    
    using deleter_type = unique_public_ptr_deleter<mpi_unique_public_ptr_policy>;
    
    using allocator_type = mpi_rma<mpi_rma_policy<MpiItf>>;
};

template <typename MpiItf>
struct mpi_rma_policy
{
    using derived_type = mpi_rma<mpi_rma_policy>;
    
    using mpi_itf_type = MpiItf;
    using ult_itf_type = typename mpi_itf_type::ult_itf_type;
    
    using proc_id_type = int;
    using size_type = mefdn::size_t;
    
    template <typename T>
    using remote_ptr = T*;
    template <typename T>
    using local_ptr = T*;
    template <typename T>
    using public_ptr = T*;
    
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
    
    template <typename T>
    using unique_local_ptr = mefdn::unique_ptr<T>;
    
    template <typename T>
    using unique_public_ptr = basic_unique_public_ptr<mpi_unique_public_ptr_policy<MpiItf, T>>;
    
    template <typename U, typename T>
    static U* static_cast_to(T* const p) noexcept {
        return static_cast<U*>(p);
    }
};


template <typename P>
struct get_rma_itf_type<rma_itf_id_t::MPI, P>
    : mefdn::type_identity<
        mpi_rma<mpi_rma_policy<typename P::mpi_itf_type>>
    > { };

} // namespace mecom2
} // namespace menps

