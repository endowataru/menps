
#pragma once

#include <menps/meth/common.hpp>
#include <menps/mecom/rma.hpp>
#include <menps/mecom/rma/unique_local_ptr.hpp>
#include <menps/mecom/structure/alltoall_buffer.hpp>

namespace menps {
namespace meth {

class dist_exit_status
{
    typedef mecom::rma::atomic_default_t    flag_value_type;
    typedef mecom::rma::atomic_default_t    result_type;
    
public:
    dist_exit_status()
        : my_flag_(mecom::rma::allocate<flag_value_type>())
    {
        this->clear();
        
        flags_.collective_initialize(my_flag_.get());
        
        const auto root = root_proc();
        
        if (mecom::current_process_id() == root) {
            my_result_.reset(mecom::rma::allocate<result_type>());
        }
        
        auto ptr = my_result_.get();
        mecom::collective::broadcast(root, &ptr, 1);
        
        result_ = mecom::rma::use_remote_ptr(root, ptr);
    }
    
    ~dist_exit_status()
    {
        flags_.finalize();
    }
    
    void clear()
    {
        *my_flag_.get() = 0;
    }
    
    void set_finished(const result_type result)
    {
        {
            // Write the result.
            
            mecom::rma::unique_local_ptr<result_type> buf(
                mecom::rma::allocate<result_type>()
            );
            *buf.get() = result;
            
            mecom::rma::write(root_proc(), result_, buf.get(), 1);
        }
        
        for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc) {
            const mecom::rma::atomic_default_t one = 1;
            mecom::rma::atomic_write(proc, flags_.at_process(proc), one);
        }
    }
    
    bool is_ready() {
        return *my_flag_.get() == 1;
    }
    
    result_type get()
    {
        MEFDN_ASSERT(is_ready());
        
        mecom::rma::unique_local_ptr<result_type> buf(
            mecom::rma::allocate<result_type>()
        );
        
        mecom::rma::read(root_proc(), result_, buf.get(), 1);
        
        return *buf.get();
    }
    
private:
    static mecom::process_id_t root_proc() noexcept {
        return 0;
    }
    
    mecom::rma::unique_local_ptr<flag_value_type>       my_flag_;
    mecom::structure::alltoall_buffer<flag_value_type>  flags_;
    
    mecom::rma::unique_local_ptr<flag_value_type>       my_result_;
    mecom::rma::remote_ptr<flag_value_type>             result_;
};

} // namespace meth
} // namespace menps

