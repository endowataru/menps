
#pragma once

#include <menps/mecom/rma/address.hpp>

namespace menps {
namespace mecom {
namespace rma {

namespace untyped {

struct register_region_params {
    void*   local_ptr;
    index_t size_in_bytes;
};

struct use_remote_region_params {
    process_id_t      proc_id;
    const region_key& key;
};

struct deregister_region_params {
    const local_region& region;
};

} // namespace untyped

class registrator
{
public:
    virtual ~registrator() /*noexcept*/ = default;
    
    registrator(const registrator&) = delete;
    registrator& operator = (const registrator&) = delete;
    
    MEFDN_NODISCARD
    virtual untyped::local_region register_region(const untyped::register_region_params&) = 0;
    
    MEFDN_NODISCARD
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params&) = 0;
    
    virtual void deregister_region(const untyped::deregister_region_params&) = 0;

    static registrator& get_instance() noexcept {
        return *reg_;
    }
    
    static void set_instance(registrator& reg) {
        reg_ = &reg;
    }
    
protected:
    registrator() noexcept = default;
    
private:
    static registrator* reg_;
};

} // namespace rma
} // namespace mecom
} // namespace menps

