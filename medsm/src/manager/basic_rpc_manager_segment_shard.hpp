
#pragma once

#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_rpc_manager_segment_shard
    : public mefdn::crtp_base<Policy>
{
    typedef typename Policy::page_type  page_type;
    typedef typename Policy::page_id_type  page_id_type;
    
protected:
    template <typename Conf>
    explicit basic_rpc_manager_segment_shard(const Conf& conf)
        : page_size_(conf.page_size)
        , num_pages_(conf.num_pages)
        , block_size_(conf.block_size)
        , pgs_(new page_type[num_pages_per_proc()])
    { }
    
public:
    mefdn::size_t get_page_size() const noexcept
    {
        return page_size_;
    }
    mefdn::size_t get_num_pages() const noexcept
    {
        return num_pages_;
    }
    mefdn::size_t get_block_size() const noexcept
    {
        return block_size_;
    }
    
    page_type& get_page(const page_id_type pg_id)
    {
        MEFDN_ASSERT(pg_id < num_pages_);
        
        const auto index = pg_id / Policy::number_of_processes();
        
        MEFDN_ASSERT(index < num_pages_per_proc());
        
        MEFDN_ASSERT(pg_id % Policy::number_of_processes() == Policy::current_process_id());
        
        return pgs_[index];
    }
    
private:
    mefdn::size_t num_pages_per_proc() {
        return mefdn::roundup_divide(num_pages_, Policy::number_of_processes());
    }
    
    mefdn::size_t page_size_;
    mefdn::size_t num_pages_;
    
    mefdn::size_t block_size_;
    
    mefdn::unique_ptr<page_type []>    pgs_;
};

} // namespace medsm
} // namespace menps

