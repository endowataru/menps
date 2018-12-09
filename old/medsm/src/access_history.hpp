
#pragma once

#include "protector/protector_space.hpp"
#include <vector>

#include "history/write_history.hpp"
#include "history/read_history.hpp"

namespace menps {
namespace medsm {

class access_history
{
    typedef mefdn::mutex   mutex_type;
    
public:
    struct conf {
        protector_space&    sp;
    };
    
    explicit access_history(const conf& cnf)
        : rh_(read_history::config{ cnf.sp })
        , wh_(write_history::config{ cnf.sp })
    { }
    
public:
    void add_new_read(const abs_block_id ablk_id)
    {
        rh_.add(ablk_id);
    }
    
    void read_barrier()
    {
        ult::suspend_and_call<void>(do_read_barrier{*this});
    }
    void async_read_barrier(mefdn::callback<void ()> cb)
    {
        rh_.commit(cb);
    }
    
private:
    struct do_read_barrier
    {
        access_history& self;
        
        template <typename Cont>
        MEFDN_NODISCARD
        ult::async_status<void> operator() (Cont&& cont) const {
            self.rh_.commit(mefdn::forward<Cont>(cont));
            return ult::make_async_deferred<void>();
        }
    };
    
    read_history rh_;
        
public:
    void add_new_write(const abs_block_id ablk_id)
    {
        wh_.add(ablk_id);
    }
    
    void write_barrier()
    {
        ult::suspend_and_call<void>(do_write_barrier{*this});
    }
    void async_write_barrier(mefdn::callback<void ()> cb)
    {
        wh_.commit(cb);
    }
    
private:
    struct do_write_barrier
    {
        access_history& self;
        
        template <typename Cont>
        MEFDN_NODISCARD
        ult::async_status<void> operator() (Cont&& cont) const {
            self.wh_.commit(mefdn::forward<Cont>(cont));
            return ult::make_async_deferred<void>();
        }
    };
    
    write_history wh_;
    
    #if 0
public:
    void add_new_read(const abs_block_id ablk_id)
    {
        mefdn::lock_guard<mutex_type> lk(this->read_ids_mtx_);
        
        this->read_ids_.push_back(ablk_id);
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
        MEFDN_LOG_INFO("msg:Started read barrier.");
        
        {
            mefdn::lock_guard<mutex_type> barier_lk(this->read_barrier_mtx_);
            
            std::vector<abs_block_id> ids;
            {
                mefdn::lock_guard<mutex_type> ids_lk(this->read_ids_mtx_);
                ids = mefdn::move(this->read_ids_);
            }
            
            MEFDN_RANGE_BASED_FOR(const auto& id, ids)
            {
                this->conf_.sp.do_for_block_at(id, read_barrier_closure{});
            }
        }
        
        MEFDN_LOG_INFO("msg:Finished read barrier.");
    }
    
private:
    mutex_type read_barrier_mtx_;
    mutex_type read_ids_mtx_;
    std::vector<abs_block_id> read_ids_;
    
public:
    void add_new_write(const abs_block_id ablk_id)
    {
        mefdn::lock_guard<mutex_type> lk(this->write_ids_mtx_);
        
        this->write_ids_.push_back(ablk_id);
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
        MEFDN_LOG_INFO("msg:Started write barrier.");
        
        {
            mefdn::lock_guard<mutex_type> barrier_lk(this->write_barrier_mtx_);
            
            std::vector<abs_block_id> ids;
            {
                mefdn::lock_guard<mutex_type> ids_lk(this->write_ids_mtx_);
                ids = mefdn::move(this->write_ids_);
            }
            
            MEFDN_RANGE_BASED_FOR(const auto& id, ids)
            {
                this->conf_.sp.do_for_block_at(id, write_barrier_closure{});
            }
        }
        
        MEFDN_LOG_INFO("msg:Finished write barrier.");
    }
    
    
    mutex_type write_barrier_mtx_;
    mutex_type write_ids_mtx_;
    std::vector<abs_block_id> write_ids_;
    
private:
    const conf conf_;
    #endif
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

} // namespace medsm
} // namespace menps

