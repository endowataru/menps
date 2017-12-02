
#pragma once

#include <menps/mecom2/rpc/tag_rpc_client.hpp>
#include <menps/mecom2/rpc/tag_rpc_server.hpp>
#include <menps/mecom2/rpc/mpi/mpi_rpc_request_comm.hpp>
#include <menps/mecom2/rpc/mpi/mpi_rpc_reply_comm.hpp>
#include <menps/mecom2/rpc/rpc_invoker.hpp>
#include <menps/mecom2/rpc/default_message.hpp>
#include <menps/mecom2/rpc/blocking_tag_allocator.hpp>
#include <menps/mecom2/rpc/rpc_typed_handler.hpp>
#include <menps/medev2/mpi/direct_requester.hpp>

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



class mpi_rpc_comm;

struct mpi_rpc_comm_policy : mpi_rpc_policy_base {
    using derived_type = mpi_rpc_comm;
};

struct mpi_rpc_error : std::exception {}; // TODO

class mpi_rpc_comm
    : public mpi_rpc_request_comm<mpi_rpc_comm_policy>
    , public mpi_rpc_reply_comm<mpi_rpc_comm_policy>
    , public default_message_allocator<mpi_rpc_comm_policy>
{
public:
    template <typename Conf>
    explicit mpi_rpc_comm(Conf&& conf)
        : mi_(conf.mi)
        , comm_(conf.comm)
        , req_tag_(conf.request_tag)
    {
        MPI_Comm_rank(comm_, &rank_);
    }
    
    int get_request_tag() const noexcept { return req_tag_; }
    
    MPI_Comm get_request_comm() const noexcept { return comm_; }
    MPI_Comm get_reply_comm() const noexcept { return comm_; }
    
    int current_process_id() const noexcept {
        return rank_;
    }
    
    medev2::mpi::direct_requester& get_mpi_interface() const noexcept {
        return mi_;
    }
    
private:
    medev2::mpi::direct_requester& mi_;
    const MPI_Comm comm_;
    /*const*/ int rank_;
    const int req_tag_;
};

class mpi_rpc;

struct mpi_rpc_policy : mpi_rpc_policy_base {
    using derived_type = mpi_rpc;
    using server_type = mpi_rpc_comm;
};


using mpi_rpc_invoker = rpc_invoker<mpi_rpc_policy>;


class mpi_rpc
    : public tag_rpc_server<mpi_rpc_policy>
    , public tag_rpc_client<mpi_rpc_policy>
    , public blocking_tag_allocator<mpi_rpc_policy>
    , public rpc_typed_handler<mpi_rpc_policy>
{
    using tag_alloc = blocking_tag_allocator<mpi_rpc_policy>;
    
public:
    template <typename Conf>
    explicit mpi_rpc(Conf&& conf)
        : tag_alloc(conf)
        , inv_(conf)
        , comm_(conf)
    { }
    
    mpi_rpc_invoker& get_invoker() noexcept {
        return inv_;
    }
    mpi_rpc_comm& get_request_comm() noexcept { return comm_; }
    mpi_rpc_comm& get_reply_comm() noexcept { return comm_; }
    
    mpi_rpc_comm& get_server() noexcept { return comm_; }
    mpi_rpc_comm& get_client() noexcept { return comm_; }
    
private:
    mpi_rpc_invoker inv_;
    mpi_rpc_comm comm_;
};

} // namespace mecom2
} // namespace menps

