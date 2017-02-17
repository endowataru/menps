
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rma/unique_local_ptr.hpp>
#include <mgcom/structure/alltoall_buffer.hpp>

namespace mgth {

class dist_exit_status
{
    typedef mgcom::rma::atomic_default_t    flag_value_type;
    typedef mgcom::rma::atomic_default_t    result_type;
    
public:
    dist_exit_status()
        : my_flag_(mgcom::rma::allocate<flag_value_type>())
    {
        this->clear();
        
        flags_.collective_initialize(my_flag_.get());
        
        const auto root = root_proc();
        
        if (mgcom::current_process_id() == root) {
            my_result_ = mgcom::rma::allocate<result_type>();
        }
        
        auto ptr = my_result_.get();
        mgcom::collective::broadcast(root, &ptr, 1);
        
        result_ = mgcom::rma::use_remote_ptr(root, ptr);
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
            
            mgcom::rma::unique_local_ptr<result_type> buf(
                mgcom::rma::allocate<result_type>()
            );
            *buf.get() = result;
            
            mgcom::rma::write(root_proc(), result_, buf.get(), 1);
        }
        
        for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc) {
            const mgcom::rma::atomic_default_t one = 1;
            mgcom::rma::atomic_write(proc, flags_.at_process(proc), one);
        }
    }
    
    bool is_ready() {
        return *my_flag_.get() == 1;
    }
    
    result_type get()
    {
        MGBASE_ASSERT(is_ready());
        
        mgcom::rma::unique_local_ptr<result_type> buf(
            mgcom::rma::allocate<result_type>()
        );
        
        mgcom::rma::read(root_proc(), result_, buf.get(), 1);
        
        return *buf.get();
    }
    
private:
    static mgcom::process_id_t root_proc() MGBASE_NOEXCEPT {
        return 0;
    }
    
    mgcom::rma::unique_local_ptr<flag_value_type>       my_flag_;
    mgcom::structure::alltoall_buffer<flag_value_type>  flags_;
    
    mgcom::rma::unique_local_ptr<flag_value_type>       my_result_;
    mgcom::rma::remote_ptr<flag_value_type>             result_;
};

} // namespace mgth

