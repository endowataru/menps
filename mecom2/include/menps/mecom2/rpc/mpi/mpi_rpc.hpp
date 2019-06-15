
#pragma once

#include <menps/mecom2/rpc/tag_rpc_client.hpp>
#include <menps/mecom2/rpc/tag_rpc_server.hpp>
#include <menps/mecom2/rpc/mpi/mpi_rpc_request_comm.hpp>
#include <menps/mecom2/rpc/mpi/mpi_rpc_reply_comm.hpp>
#include <menps/mecom2/rpc/rpc_invoker.hpp>
#include <menps/mecom2/rpc/default_message.hpp>
#include <menps/mecom2/rpc/blocking_tag_allocator.hpp>
#include <menps/mecom2/rpc/rpc_typed_handler.hpp>

namespace menps {
namespace mecom2 {

using mpi_rpc_handler_id_t = mefdn::size_t;

struct mpi_rpc_request_header
{
    mpi_rpc_handler_id_t    handler_id;
    int                     reply_tag;
};

template <typename T>
struct mpi_rpc_request_message_policy {
    using derived_type = basic_message<mpi_rpc_request_message_policy>;
    using data_type = T;
    using header_type = mpi_rpc_request_header;
    using deleter_type = default_message_deleter<mpi_rpc_request_message_policy>;
    
    template <typename U>
    using rebind = basic_message<mpi_rpc_request_message_policy<U>>;
};

template <typename T>
using mpi_rpc_request_message = basic_message<mpi_rpc_request_message_policy<T>>;


template <typename T>
struct mpi_rpc_reply_message_policy {
    using derived_type = basic_message<mpi_rpc_reply_message_policy>;
    using data_type = T;
    using header_type = void;
    using deleter_type = default_message_deleter<mpi_rpc_reply_message_policy>;
    
    template <typename U>
    using rebind = basic_message<mpi_rpc_reply_message_policy<U>>;
};

template <typename T>
using mpi_rpc_reply_message = basic_message<mpi_rpc_reply_message_policy<T>>;


struct mpi_rpc_policy_base {
    using size_type = mefdn::size_t;
    using process_id_type = int;
    using tag_type = int;
    using handler_id_type = mpi_rpc_handler_id_t;
    
    template <typename T>
    using server_request_message = mpi_rpc_request_message<T>;
    
    template <typename T>
    using server_reply_message = mpi_rpc_reply_message<T>;
    
    template <typename T>
    using client_request_message = mpi_rpc_request_message<T>;
    
    template <typename T>
    using client_reply_message = mpi_rpc_reply_message<T>;
};



template <typename MpiItf>
class mpi_rpc_comm;

template <typename MpiItf>
struct mpi_rpc_comm_policy : mpi_rpc_policy_base {
    using derived_type = mpi_rpc_comm<MpiItf>;
};

template <typename MpiItf>
class mpi_rpc_comm
    : public mpi_rpc_request_comm<mpi_rpc_comm_policy<MpiItf>>
    , public mpi_rpc_reply_comm<mpi_rpc_comm_policy<MpiItf>>
    , public default_message_allocator<mpi_rpc_comm_policy<MpiItf>>
{
    using mpi_itf_type = MpiItf;
    using mpi_facade_type = typename mpi_itf_type::mpi_facade_type;
    
public:
    template <typename Conf>
    explicit mpi_rpc_comm(Conf&& conf)
        : mf_(conf.mf)
        , comm_(conf.comm)
        , req_tag_(conf.request_tag)
    {
        mf_.comm_rank({ comm_, &rank_ });
    }
    
    int get_request_tag() const noexcept { return req_tag_; }
    
    MPI_Comm get_request_comm() const noexcept { return comm_; }
    MPI_Comm get_reply_comm() const noexcept { return comm_; }
    
    int this_proc_id() const noexcept {
        return rank_;
    }
    
    mpi_facade_type& get_mpi_interface() const noexcept {
        return mf_;
    }
    
private:
    mpi_facade_type& mf_;
    const MPI_Comm comm_;
    /*const*/ int rank_ = 0;
    const int req_tag_;
};


template <typename MpiItf>
class mpi_rpc;

template <typename MpiItf>
struct mpi_rpc_policy : mpi_rpc_policy_base {
    using derived_type = mpi_rpc<MpiItf>;
    using server_type = mpi_rpc_comm<MpiItf>;
};

template <typename MpiItf>
class mpi_rpc
    : public tag_rpc_server<mpi_rpc_policy<MpiItf>>
    , public tag_rpc_client<mpi_rpc_policy<MpiItf>>
    , public blocking_tag_allocator<mpi_rpc_policy<MpiItf>>
    , public rpc_typed_handler<mpi_rpc_policy<MpiItf>>
{
    using tag_alloc = blocking_tag_allocator<mpi_rpc_policy<MpiItf>>;
    
    using rpc_invoker_type = rpc_invoker<mpi_rpc_policy<MpiItf>>;
    using rpc_comm_type = mpi_rpc_comm<MpiItf>;
    
public:
    template <typename Conf>
    explicit mpi_rpc(Conf&& conf)
        : tag_alloc(conf)
        , inv_(conf)
        , comm_(conf)
    { }
    
    rpc_invoker_type& get_invoker() noexcept { return this->inv_; }
    rpc_comm_type& get_request_comm() noexcept { return this->comm_; }
    rpc_comm_type& get_reply_comm() noexcept { return this->comm_; }
    
    rpc_comm_type& get_server() noexcept { return this->comm_; }
    rpc_comm_type& get_client() noexcept { return this->comm_; }
    
private:
    rpc_invoker_type inv_;
    rpc_comm_type comm_;
};

template <typename MpiItf>
inline mefdn::unique_ptr<mpi_rpc<MpiItf>>
make_mpi_rpc(
    typename MpiItf::mpi_facade_type&   mf
,   MPI_Comm                            comm
) {
    struct conf {
        typename MpiItf::mpi_facade_type& mf;
        MPI_Comm comm;
        mefdn::size_t max_num_handlers;
        int reply_tag_start;
        int reply_tag_end;
        int request_tag;
    };
    
    return mefdn::make_unique<mecom2::mpi_rpc<MpiItf>>(
        conf{ mf, comm, 1024 , 1, 1000, 0} // TODO: magic numbers
    );
}

} // namespace mecom2
} // namespace menps

