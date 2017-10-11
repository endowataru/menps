
#pragma once

#include <mgbase/lang.hpp>

namespace mgdsm {

typedef mgbase::size_t  segment_id_t;
typedef mgbase::size_t  page_id_t;
typedef mgbase::size_t  block_id_t;

struct abs_block_id
{
    segment_id_t    seg_id;
    page_id_t       pg_id;
    block_id_t      blk_id;
};

struct dsm_base_policy
{
    typedef segment_id_t    segment_id_type;
    typedef page_id_t       page_id_type;
    typedef block_id_t      block_id_type;
    
    typedef abs_block_id    abs_block_id_type;
};

} // namespace mgdsm

