
#pragma once

#include <mgcom/rma/pointer.hpp>
#include <mgbase/callback.hpp>
#include <mgbase/type_traits.hpp>
#include <mgcom/ult.hpp>

namespace mgcom {
namespace rma {

struct untyped_read_params
{
    process_id_t                src_proc;
    untyped::remote_address     src_raddr;
    untyped::local_address      dest_laddr;
    index_t                     size_in_bytes;
};
struct untyped_write_params
{
    process_id_t                dest_proc;
    untyped::remote_address     dest_raddr;
    untyped::local_address      src_laddr;
    index_t                     size_in_bytes;
};

template <typename T>
struct atomic_read_params
{
    process_id_t                src_proc;
    remote_ptr<const T>         src_rptr;
    T*                          dest_ptr;
};
template <typename T>
struct atomic_write_params
{
    process_id_t                dest_proc;
    remote_ptr<T>               dest_rptr;
    T                           value;
};

template <typename T>
struct compare_and_swap_params
{
    process_id_t                target_proc;
    remote_ptr<T>               target_rptr;
    T                           expected;
    T                           desired;
    T*                          result_ptr;
};
template <typename T>
struct fetch_and_add_params {
    process_id_t                target_proc;
    remote_ptr<T>               target_rptr;
    T                           value;
    T*                          result_ptr;
};

struct async_untyped_read_params : untyped_read_params 
{
    mgbase::callback<void ()>   on_complete;
};
struct async_untyped_write_params : untyped_write_params
{
    mgbase::callback<void ()>   on_complete;
};

template <typename T>
struct async_atomic_read_params : atomic_read_params<T>
{
    mgbase::callback<void ()>   on_complete;
};
template <typename T>
struct async_atomic_write_params : atomic_write_params<T>
{
    mgbase::callback<void ()>   on_complete;
};

template <typename T>
struct async_compare_and_swap_params : compare_and_swap_params<T>
{
    mgbase::callback<void ()>   on_complete;
};
template <typename T>
struct async_fetch_and_add_params : fetch_and_add_params<T>
{
    mgbase::callback<void ()>   on_complete;
};

namespace untyped {

// TODO : Remove these.
typedef async_untyped_read_params   read_params;
typedef async_untyped_write_params  write_params;

} // namespace untyped

class requester
{
protected:
    requester() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    virtual ~requester() /*noexcept*/ = default;
    
    requester(const requester&) = delete;
    requester& operator = (const requester&) = delete;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_read(const async_untyped_read_params&)= 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_write(const async_untyped_write_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_read(
        const async_atomic_read_params<atomic_default_t>&
    ) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_write(
        const async_atomic_write_params<atomic_default_t>&
    ) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_compare_and_swap(
        const async_compare_and_swap_params<atomic_default_t>&
    ) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_fetch_and_add(
        const async_fetch_and_add_params<atomic_default_t>&
    ) = 0;
    
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

