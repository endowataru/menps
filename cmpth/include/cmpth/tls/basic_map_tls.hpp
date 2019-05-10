
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct tls_error : std::exception { };

template <typename P>
struct basic_map_tls_map
{
    using tls_key_type = typename P::tls_key_type;
    using tls_map_impl_type = typename P::tls_map_impl_type;
    
public:
    void* get(const tls_key_type key)
    {
        using fdn::end;
        auto itr = this->m_.find(key);
        if (CMPTH_UNLIKELY(itr == end(this->m_))) {
            return nullptr;
        }
        return itr->second;
    }
    
    void set(const tls_key_type key, void* const val) {
        this->m_[key] = val;
    }
    
private:
    tls_map_impl_type    m_;
    // std::unordered_map<tls_key_type, void*>  m_;
};

template <typename P>
class basic_map_tls_key_pool
{
    using tls_key_type = typename P::tls_key_type;
    using spinlock_type = typename P::spinlock_type;
    
public:
    basic_map_tls_key_pool() noexcept = default;
    
    basic_map_tls_key_pool(const basic_map_tls_key_pool&) = delete;
    basic_map_tls_key_pool& operator = (const basic_map_tls_key_pool&) = delete;
    
    tls_key_type allocate() {
        fdn::lock_guard<spinlock_type> lk{this->mtx_};
        return this->free_key_++;
    }
    
    void deallocate() {
        // TODO: Unimplemented
    }
    
private:
    spinlock_type   mtx_;
    tls_key_type    free_key_ = 0;
};

template <typename P, typename VarP>
class basic_map_tls_thread_specific
{
    using worker_type = typename P::worker_type;
    using tls_key_type = typename P::tls_key_type;
    
    using value_type = typename VarP::value_type;
    
public:
    basic_map_tls_thread_specific() {
        auto& pool = P::get_tls_key_pool();
        this->key_ = pool.allocate();
    }
    
    ~basic_map_tls_thread_specific() {
        auto& pool = P::get_tls_key_pool();
        pool.deallocate();
    }
    
    basic_map_tls_thread_specific(const basic_map_tls_thread_specific&) = delete;
    basic_map_tls_thread_specific& operator = (const basic_map_tls_thread_specific&) = delete;
    
    value_type* get() const
    {
        auto& wk = worker_type::get_cur_worker();
        const auto tk = wk.get_cur_task_ref();
        const auto desc = tk.get_task_desc();
        
        const auto ret = desc->tls.get(this->key_);
        return static_cast<value_type*>(ret);
    }
    
    void set(value_type* const p) const
    {
        auto& wk = worker_type::get_cur_worker();
        const auto tk = wk.get_cur_task_ref();
        const auto desc = tk.get_task_desc();
        
        desc->tls.set(this->key_, p);
    }
    
private:
    tls_key_type key_;
};

} // namespace cmpth

