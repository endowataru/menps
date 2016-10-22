
#pragma once

#include <mgbase/lang.hpp>

namespace mgult {

template <typename Traits>
class basic_scheduler_initializer
{
    typedef typename Traits::derived_type   derived_type;
    
public:
    void initialize()
    {
        auto& self = this->derived();
        
        const auto stack_size = self.get_stack_size();
        
        call_stack_.reset(new mgbase::uint8_t[stack_size]);
        
        const auto ctx =
            self.make_context(
                call_stack_.get() + stack_size
            ,   stack_size
            ,   &call_loop
            );
        
        const auto r = self.template jump_context<derived_type>(ctx, &self);
        
        this->ctx_ =
            self.template cast_context<derived_type, derived_type>(
                r.fctx
            );
    }
    
    void finalize()
    {
        auto& self = this->derived();
        
        self.template jump_context<derived_type>(ctx_, &self);
        
        call_stack_.reset();
    }
    
private:
    MGBASE_NORETURN
    static void call_loop(typename Traits::template context_argument<derived_type, derived_type>::type arg)
    {
        auto& self = *arg.data;
        
        const auto ctx =
            self.template cast_context<derived_type, derived_type>(
                arg.fctx
            );
        
        self.ctx_ = ctx;
        
        self.loop(functor{self});
        
        self.template jump_context<derived_type>(self.ctx_, &self);
        
        // this context is abandoned.
    }
    
    struct functor
    {
        derived_type& self;
        
        void operator() ()
        {
            const auto r =
                self.template jump_context<derived_type>(self.ctx_, &self);
            
            self.ctx_ =
                self.template cast_context<derived_type, derived_type>(
                    r.fctx
                );
        }
    };
    
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    typename Traits::template context<derived_type, derived_type>::type ctx_;
    
    mgbase::unique_ptr<mgbase::uint8_t []> call_stack_;
};

} // namespace mgult

