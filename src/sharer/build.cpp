
#include "sharer_block_accessor.hpp"

namespace mgdsm {

void f(sharer_block::accessor& blk_pr)
{
    blk_pr.fetch();
    blk_pr.touch();
    blk_pr.reconcile();
    blk_pr.flush();
}

} // namespace mgdsm

#if 0

namespace mgdsm {


#if 0
class requester_block_entry
    : public requester_block_state
{
public:
    mgcom::rma::local_ptr<void> get_twin_lptr() const MGBASE_NOEXCEPT {
        return twin_.get();
    }
    
private:
    mgcom::rma::unique_local_ptr<void> twin_;
};

class requester_block
    : private requester_block_entry
{
public:
    class accessor;
};

class requester_page_entry;

struct requester_page_entry_traits
{
    typedef requester_page_entry    derived_type;
    typedef requester_block         block_type;
    typedef requester_block_entry   block_entry_type;
    typedef block_id_t              block_id_type;
    typedef block_id_t              block_count_type;
};


class requester_page_entry
    : public basic_requester_page_entry<requester_page_entry_traits>
{
public:
    typedef mgbase::unique_lock<mgbase::spinlock>   unique_lock_type;
    
private:
    
};

class requester_segment
{
public:
    class accessor;
    
private:
    mgbase::unique_ptr<manager_segment> manager_;
};

struct requester_segment_accessor_traits
{
    typedef requester_segment::accessor        derived_type;
    typedef page_id_t                       page_id_type;
    
    typedef manager_segment::acquire_read_page_result   acquire_read_page_result_type;
    typedef manager_segment::acquire_write_page_result  acquire_write_page_result_type;
};

class requester_segment::accessor
    : public basic_requester_segment_accessor<requester_segment_accessor_traits>
{
public:
    manager_segment& get_manager() const MGBASE_NOEXCEPT {
        return *seg_.manager_;
    }
    
private:
    requester_segment& seg_;
};
#endif


class holed_range
{

};

holed_range merge_holed(holed_range, holed_range);

class mem_range
{
public:
    mem_range subrange(mgbase::size_t index_from, mgbase::size_t index_to);
    
    mgbase::size_t index_first();
    mgbase::size_t index_last();
};

holed_range fetch_block(requester_block::accessor);/* {
    
}*/

holed_range fetch_page(requester_page::accessor pr)
{
    if (pr.start_read()) {
        return pr.reduce_blocks(0, 0, &fetch_block, holed_range{}, &merge_holed);
    }
    else {
        return {};
    }
}


void f(requester_segment& seg)
{
    auto&& lk = seg.get_accessor();
    
    auto h = lk.reduce_pages(0, 0, &fetch_page, holed_range{}, &merge_holed);
}

/*void f(requester_page& page)
{
    auto&& lk = page.get_lock();
    
    lk.fetch(mem_range{});
}*/

}

#endif

