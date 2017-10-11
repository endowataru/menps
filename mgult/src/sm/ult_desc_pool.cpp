
#include <mgult/sm/impl/ult_desc_pool.hpp>

namespace mgult {
namespace sm {

ult_ptr_ref ult_desc_pool::create_ult()
{
    const mgbase::size_t stack_size = 2048 * 1024; // TODO
    
    const auto sp_start = new mgbase::uint8_t[stack_size];
    const auto sp_end = sp_start + stack_size;
    
    const auto desc_ptr = sp_end - sizeof(ult_desc_node);
    
    // Create a new thread descriptor at the end of its stack.
    auto desc = new (desc_ptr) ult_desc_node;
    
    // Set the end of the call stack.
    desc->stack_ptr = desc;
    desc->stack_size = stack_size - sizeof(ult_desc_node);
    
    initialize_desc(desc);
    
    MGBASE_LOG_VERBOSE(
        "msg:Allocate a new thread descriptor.\t"
        "desc:{:x}\t"
        "stack_ptr:{:x}\t"
        "stack_size:{}"
    ,   reinterpret_cast<mgbase::uintptr_t>(desc)
    ,   reinterpret_cast<mgbase::uintptr_t>(desc->stack_ptr)
    ,   desc->stack_size
    );
    
    ult_id id{ desc };
    return ult_ptr_ref(id);
}

void ult_desc_pool::destory_ult(ult_ptr_ref&& th)
{
    const auto id = th.get_id();
    
    auto desc = static_cast<ult_desc*>(id.ptr);
    
    const auto sp_end = static_cast<mgbase::uint8_t*>(desc->stack_ptr);
    const auto sp_start = sp_end - desc->stack_size;
    
    MGBASE_LOG_VERBOSE(
        "msg:Deallocate the thread descriptor.\t"
        "desc:{:x}\t"
        "stack_ptr:{:x}\t"
        "stack_size:{}\t"
        "sp_start:{:x}"
    ,   reinterpret_cast<mgbase::uintptr_t>(desc)
    ,   reinterpret_cast<mgbase::uintptr_t>(desc->stack_ptr)
    ,   desc->stack_size
    ,   reinterpret_cast<mgbase::uintptr_t>(sp_start)
    );
    
    delete[] sp_start;
    
    delete desc;
}

} // namespace sm
} // namespace mgult

