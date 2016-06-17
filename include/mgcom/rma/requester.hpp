
#pragma once

#include <mgcom/rma/pointer.hpp>
#include <mgbase/operation.hpp>
#include <mgbase/type_traits.hpp>
#include <mgbase/optional.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

struct read_params
{
    process_id_t        src_proc;
    remote_address      src_raddr;
    local_address       dest_laddr;
    index_t             size_in_bytes;
    mgbase::operation   on_complete;
};

struct write_params
{
    process_id_t        dest_proc;
    remote_address      dest_raddr;
    local_address       src_laddr;
    index_t             size_in_bytes;
    mgbase::operation   on_complete;
};

} // namespace untyped

template <typename T>
struct read_params
{
    process_id_t        src_proc;
    remote_ptr<T>       src_rptr;
    local_ptr<T>        dest_lptr;
    index_t             num_elems;
    mgbase::operation   on_complete;
};

template <typename T>
struct write_params
{
    process_id_t        dest_proc;
    remote_ptr<T>       dest_rptr;
    local_ptr<T>        src_lptr;
    index_t             num_elems;
    mgbase::operation   on_complete;
};

template <typename T>
struct atomic_read_params
{
    process_id_t        src_proc;
    remote_ptr<const T> src_rptr;
    T*                  dest_ptr;
    mgbase::operation   on_complete;
};

template <typename T>
struct atomic_write_params
{
    process_id_t        dest_proc;
    remote_ptr<T>       dest_rptr;
    T                   value;
    mgbase::operation   on_complete;
};

template <typename T>
struct compare_and_swap_params
{
    process_id_t        target_proc;
    remote_ptr<T>       target_rptr;
    T                   expected;
    T                   desired;
    T*                  result_ptr;
    mgbase::operation   on_complete;
};

template <typename T>
struct fetch_and_add_params
{
    process_id_t        target_proc;
    remote_ptr<T>       target_rptr;
    T                   value;
    T*                  result_ptr;
    mgbase::operation   on_complete;
};

class requester
    : mgbase::noncopyable
{
public:
    virtual ~requester() MGBASE_EMPTY_DEFINITION
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_read_async(const untyped::read_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_write_async(const untyped::write_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_atomic_read_async(const atomic_read_params<atomic_default_t>&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_atomic_write_async(const atomic_write_params<atomic_default_t>&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>&) = 0;
    
    static requester& get_instance() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(req_ != MGBASE_NULLPTR);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;
};

} // namespace rma
} // namespace mgcom

