
#pragma once

#include <menps/mefdn/nontype.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_scheduler_initializer
{
    MEFDN_DEFINE_DERIVED(P)
    
    using context_type = typename P::context_type;
    using transfer_type = typename P::transfer_type;
    
public:
    void initialize()
    {
        auto& self = this->derived();
        
        const auto stack_size = self.get_stack_size();
        
        this->call_stack_.reset(new mefdn::uint8_t[stack_size]);
        
        self.save_context(
            call_stack_.get() + stack_size
        ,   stack_size
        ,   MEFDN_NONTYPE_TEMPLATE(&basic_scheduler_initializer::call_loop)
        ,   &self
        );
        // <-- (A) -->
    }
    
    void finalize()
    {
        auto& self = this->derived();
        
        self.swap_context(
            self.exit_ctx_ // (B)
        ,   MEFDN_NONTYPE_TEMPLATE(&basic_scheduler_initializer::on_swap_fin)
        ,   &self
        );
        // <-- (C) -->
        
        this->call_stack_.reset();
    }
    
private:
    static transfer_type on_restore_exit(void* const /*ignored*/) {
        return { nullptr };
    }
    
    MEFDN_NORETURN
    static transfer_type call_loop(const context_type init_ctx, derived_type* const self)
    {
        self->loop(functor{ *self, init_ctx });
        
        void* const null = nullptr;
        self->restore_context(
            self->fin_ctx_ // (C)
        ,   MEFDN_NONTYPE_TEMPLATE(&basic_scheduler_initializer::on_restore_exit)
        ,   null
        );
        
        // This context is abandoned.
        MEFDN_UNREACHABLE();
    }
    
    static transfer_type on_init_swap(const context_type exit_ctx, derived_type* const self)
    {
        self->exit_ctx_ = exit_ctx /* (B) */;
        
        return { nullptr };
    }
    
    struct functor
    {
        derived_type&   self;
        context_type    init_ctx;
        
        void operator() ()
        {
            self.swap_context(
                init_ctx // (A)
            ,   MEFDN_NONTYPE_TEMPLATE(&basic_scheduler_initializer::on_init_swap)
            ,   &self
            );
            // <-- (B) -->
        }
    };
    
    static transfer_type on_swap_fin(const context_type fin_ctx, derived_type* const self)
    {
        self->fin_ctx_ = fin_ctx /* (C) */;
        
        return { nullptr };
    }
    
    context_type exit_ctx_;
    context_type fin_ctx_;
    
    mefdn::unique_ptr<mefdn::uint8_t []> call_stack_;
};

} // namespace meult
} // namespace menps

