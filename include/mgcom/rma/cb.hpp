
#pragma once

#include <mgbase/atomic.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

struct remote_read_params {
    process_id_t    src_proc;
    remote_address  src_raddr;
    local_address   dest_laddr;
    index_t         num_bytes;
};

struct remote_write_params {
    process_id_t    dest_proc;
    remote_address  dest_raddr;
    local_address   src_laddr;
    index_t         num_bytes;
};

namespace detail {

template <typename Config>
struct read_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_read_params params;
        Config::set_params(cb, &params);
        
        return try_remote_read_async(
            params.src_proc
        ,   params.src_raddr
        ,   params.dest_laddr
        ,   params.num_bytes
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};

template <typename Config>
struct write_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_write_params params;
        Config::set_params(cb, &params);
        
        return try_remote_write_async(
            params.dest_proc
        ,   params.dest_raddr
        ,   params.src_laddr
        ,   params.num_bytes
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};

} // namespace detail

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_read_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::read_config<Config> >(cb);
}

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_write_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::write_config<Config> >(cb);
}

} // namespace untyped

template <typename T>
struct remote_read_params
{
    process_id_t    src_proc;
    remote_ptr<T>   src_rptr;
    local_ptr<T>    dest_lptr;
    index_t         num_elems;
};

template <typename T>
struct remote_write_params
{
    process_id_t    dest_proc;
    remote_ptr<T>   dest_rptr;
    local_ptr<T>    src_lptr;
    index_t         num_elems;
};


template <typename T>
struct remote_atomic_read_params
{
    process_id_t        src_proc;
    remote_ptr<const T> src_lptr;
    T*                  dest_ptr;
};

template <typename T>
struct remote_atomic_write_params
{
    process_id_t        dest_proc;
    remote_ptr<T>       dest_rptr;
    T                   value;
};


template <typename T>
struct remote_compare_and_swap_params
{
    process_id_t        target_proc;
    remote_ptr<T>       target_rptr;
    T                   expected;
    T                   desired;
    T*                  result_ptr;
};

template <typename T>
struct remote_fetch_and_add_params
{
    process_id_t        target_proc;
    remote_ptr<T>       target_rptr;
    T                   value;
    T*                  result_ptr;
};



namespace detail {

template <typename Config>
struct read_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    typedef typename Config::element_type   element_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_read_params<element_type> params;
        Config::set_params(cb, &params);
        
        return try_remote_read_async(
            params.src_proc
        ,   params.src_rptr
        ,   params.dest_lptr
        ,   params.num_elems
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};

template <typename Config>
struct write_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    typedef typename Config::element_type   element_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_write_params<element_type> params;
        Config::set_params(cb, &params);
        
        return try_remote_write_async(
            params.dest_proc
        ,   params.dest_rptr
        ,   params.src_lptr
        ,   params.num_elems
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};


template <typename Config>
struct atomic_read_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    typedef typename Config::element_type   element_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_atomic_read_params<element_type> params;
        Config::set_params(cb, &params);
        
        return try_remote_atomic_read_async(
            params.src_proc
        ,   params.src_rptr
        ,   params.dest_ptr
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};


template <typename Config>
struct atomic_write_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    typedef typename Config::element_type   element_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_atomic_write_params<element_type> params;
        Config::set_params(cb, &params);
        
        return try_remote_atomic_write_async(
            params.dest_proc
        ,   params.dest_rptr
        ,   params.value
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};


template <typename Config>
struct compare_and_swap_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    typedef typename Config::element_type   element_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_compare_and_swap_params<element_type> params;
        Config::set_params(cb, &params);
        
        return try_remote_compare_and_swap_async(
            params.target_proc
        ,   params.target_rptr
        ,   params.expected
        ,   params.desired
        ,   params.result_ptr
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};

template <typename Config>
struct fetch_and_add_config
{
    typedef typename Config::cb_type        cb_type;
    typedef typename Config::result_type    result_type;
    typedef typename Config::element_type   element_type;
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static bool try_call(cb_type& cb)
    {
        remote_fetch_and_add_params<element_type> params;
        Config::set_params(cb, &params);
        
        return try_remote_fetch_and_add_async(
            params.target_proc
        ,   params.target_rptr
        ,   params.value
        ,   params.result_ptr
        ,   mgbase::make_notification(&get_flag(cb))
        );
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    static result_type on_complete(cb_type& cb) {
        return Config::on_complete(cb);
    }
    
    MGBASE_ALWAYS_INLINE
    static mgbase::wait_flag& get_flag(cb_type& cb) MGBASE_NOEXCEPT {
        return Config::get_flag(cb);
    }
};

} // namespace detail

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_read_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::read_config<Config> >(cb);
}

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_write_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::write_config<Config> >(cb);
}

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_atomic_read_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::atomic_read_config<Config> >(cb);
}

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_atomic_write_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::atomic_write_config<Config> >(cb);
}

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_compare_and_swap_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::compare_and_swap_config<Config> >(cb);
}

template <typename Config>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
typename Config::result_type remote_fetch_and_add_as(typename Config::cb_type& cb) {
    return mgbase::try_call_and_wait< detail::fetch_and_add_config<Config> >(cb);
}

} // namespace rma
} // namespace mgcom

