
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/unique_ptr.hpp>
#include <mgbase/arithmetic.hpp>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_segment_shard
    : public mgbase::crtp_base<Policy>
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
    mgbase::size_t get_page_size() const MGBASE_NOEXCEPT
    {
        return page_size_;
    }
    mgbase::size_t get_num_pages() const MGBASE_NOEXCEPT
    {
        return num_pages_;
    }
    mgbase::size_t get_block_size() const MGBASE_NOEXCEPT
    {
        return block_size_;
    }
    
    page_type& get_page(const page_id_type pg_id)
    {
        MGBASE_ASSERT(pg_id < num_pages_);
        
        const auto index = pg_id / Policy::number_of_processes();
        
        MGBASE_ASSERT(index < num_pages_per_proc());
        
        MGBASE_ASSERT(pg_id % Policy::number_of_processes() == Policy::current_process_id());
        
        return pgs_[index];
    }
    
private:
    mgbase::size_t num_pages_per_proc() {
        return mgbase::roundup_divide(num_pages_, Policy::number_of_processes());
    }
    
    mgbase::size_t page_size_;
    mgbase::size_t num_pages_;
    
    mgbase::size_t block_size_;
    
    mgbase::unique_ptr<page_type []>    pgs_;
};

} // namespace mgdsm

