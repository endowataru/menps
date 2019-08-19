
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class basic_dsm_itf
{
    using ult_itf_type = typename P::ult_itf_type;
    using thread_type = typename ult_itf_type::thread;
    using barrier_type = typename ult_itf_type::barrier;
    
    using thread_num_type = typename P::thread_num_type;
    
public:
    using dsm_facade_type = typename P::dsm_facade_type;
    
    static void set_dsm_facade(dsm_facade_type& df) {
        df_ = &df;
    }
    static dsm_facade_type& get_dsm_facade() {
        MEFDN_ASSERT(df_ != nullptr);
        return *df_;
    }
    
    static void barrier()
    {
        auto& df = get_dsm_facade();
        df.barrier();
    }
    
    template <typename Func>
    static void spmd_parallel(Func&& func)
    {
        auto& df = get_dsm_facade();
        df.spmd_parallel(mefdn::forward<Func>(func));
    }
    
    template <typename Func>
    static void spmd_single_nowait(Func&& func) {
        if (get_thread_num() == 0) {
            mefdn::forward<Func>(func)();
        }
    }
    
    template <typename Func>
    static void spmd_single(Func&& func)
    {
        spmd_single_nowait(mefdn::forward<Func>(func));
        barrier();
    }
    
    #if 0
    template <typename T>
    void spmd_reduction_plus(T& var, T* part);
    #endif
    
    template <typename T>
    static T* conew() {
        auto& df = get_dsm_facade();
        const auto p = df.coallocate(sizeof(T));
        return new (p) T();
    }
    template <typename T>
    static void codelete(T* p) {
        p->~T();
        auto& df = get_dsm_facade();
        df.codeallocate(p);
    }
    
    template <typename T>
    static T* conew_array(const mefdn::size_t num_elems) {
        auto& df = get_dsm_facade();
        const auto p = df.coallocate(sizeof(T) * num_elems);
        // TODO: support non-trivial types
        MEFDN_ASSERT(mefdn::is_trivial<T>::value);
        return static_cast<T*>(p);
    }
    template <typename T>
    static void codelete_array(T* const ptr) {
        auto& df = get_dsm_facade();
        // TODO: support non-trivial types
        MEFDN_ASSERT(mefdn::is_trivial<T>::value);
        df.codeallocate(ptr);
    }
    
    template <typename T>
    static T* new_() {
        auto& df = get_dsm_facade();
        const auto p = df.allocate(sizeof(T));
        return new (p) T();
    }
    template <typename T>
    static void delete_(T* p) {
        p->~T();
        auto& df = get_dsm_facade();
        df.deallocate(p);
    }
    
    static thread_num_type get_thread_num() {
        auto& df = get_dsm_facade();
        return df.get_thread_num();
    }
    static thread_num_type max_num_threads() {
        auto& df = get_dsm_facade();
        return df.max_num_threads();
    }
    
private:
    static dsm_facade_type* df_;
};

template <typename P>
typename basic_dsm_itf<P>::dsm_facade_type* basic_dsm_itf<P>::df_ = nullptr;

} // namespace medsm2
} // namespace menps

