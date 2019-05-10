
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_worker_tls
{
    CMPTH_DEFINE_DERIVED(P)
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    
    struct tls_policy {
        using value_type = derived_type;
    };
    
    using tls_type =
        typename base_ult_itf_type::template thread_specific<tls_policy>;
    
public:
    void initialize_tls()
    {
        auto& self = this->derived();
        
        CMPTH_P_ASSERT(P, tls_.get() == nullptr);
        tls_.set(&self);
    }
    
    void finalize_tls()
    {
        this->check_cur_worker();
        tls_.set(nullptr);
    }
    
    static derived_type& get_cur_worker() noexcept {
        auto* const self = tls_.get();
        CMPTH_P_ASSERT(P, self != nullptr);
        return *self;
    }
    
    void check_cur_worker() const noexcept {
        CMPTH_P_ASSERT(P, this == &get_cur_worker());
    }
    
private:
    static tls_type tls_;
};

template <typename P>
typename basic_worker_tls<P>::tls_type basic_worker_tls<P>::tls_{};

} // namespace cmpth

