
#pragma once

#include "protector/protector_space.hpp"
#include <vector>

namespace mgdsm {

class access_history
{
    typedef mgbase::mutex   mutex_type;
    
public:
    struct conf {
        protector_space&    sp;
    };
    
    explicit access_history(const conf& cnf)
        : conf_(cnf)
    { }
    
    void add_new_read(const abs_block_id ablk_id)
    {
        mgbase::lock_guard<mutex_type> lk(this->read_ids_mtx_);
        
        this->read_ids_.push_back(ablk_id);
    }
    
    void add_new_write(const abs_block_id ablk_id)
    {
        mgbase::lock_guard<mutex_type> lk(this->write_ids_mtx_);
        
        this->write_ids_.push_back(ablk_id);
    }
    
private:
    struct read_barrier_closure
    {
        void operator() (protector_block_accessor& blk)
        {
            // Reconcile the page first, then flush it.
            
            // If reconcile is not needed, do nothing.
            blk.reconcile();
            
            // If flush is not needed, do nothing.
            blk.flush();
        }
    };
    
public:
    void read_barrier()
    {
        MGBASE_LOG_INFO("msg:Started read barrier.");
        
        {
            mgbase::lock_guard<mutex_type> barier_lk(this->read_barrier_mtx_);
            
            std::vector<abs_block_id> ids;
            {
                mgbase::lock_guard<mutex_type> ids_lk(this->read_ids_mtx_);
                ids = mgbase::move(this->read_ids_);
            }
            
            MGBASE_RANGE_BASED_FOR(const auto& id, ids)
            {
                this->conf_.sp.do_for_block_at(id, read_barrier_closure{});
            }
        }
        
        MGBASE_LOG_INFO("msg:Finished read barrier.");
    }
    
private:
    struct write_barrier_closure
    {
        void operator() (protector_block_accessor& blk)
        {
            // If reconcile is not needed, do nothing.
            blk.reconcile();
        }
    };
    
public:
    void write_barrier()
    {
        MGBASE_LOG_INFO("msg:Started write barrier.");
        
        {
            mgbase::lock_guard<mutex_type> barrier_lk(this->write_barrier_mtx_);
            
            std::vector<abs_block_id> ids;
            {
                mgbase::lock_guard<mutex_type> ids_lk(this->write_ids_mtx_);
                ids = mgbase::move(this->write_ids_);
            }
            
            MGBASE_RANGE_BASED_FOR(const auto& id, ids)
            {
                this->conf_.sp.do_for_block_at(id, write_barrier_closure{});
            }
        }
        
        MGBASE_LOG_INFO("msg:Finished write barrier.");
    }
    
private:
    const conf conf_;
    
    mutex_type read_barrier_mtx_;
    mutex_type read_ids_mtx_;
    std::vector<abs_block_id> read_ids_;
    
    mutex_type write_barrier_mtx_;
    mutex_type write_ids_mtx_;
    std::vector<abs_block_id> write_ids_;
};

void protector_block_accessor::add_new_read()
{
    auto& sp = seg_.get_space();
    auto& hist = sp.get_history();
    auto ablk_id = sh_blk_ac_.get_abs_block_id();
    
    hist.add_new_read(ablk_id);
}
void protector_block_accessor::add_new_write()
{
    auto& sp = seg_.get_space();
    auto& hist = sp.get_history();
    auto ablk_id = sh_blk_ac_.get_abs_block_id();
    
    hist.add_new_write(ablk_id);
}

} // namespace mgdsm
