
#pragma once

#include <mgcom/rma/address.hpp>

namespace mgcom {
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
    : mgbase::noncopyable
{
public:
    virtual ~registrator() MGBASE_EMPTY_DEFINITION
    
    MGBASE_WARN_UNUSED_RESULT
    virtual untyped::local_region register_region(const untyped::register_region_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params&) = 0;
    
    virtual void deregister_region(const untyped::deregister_region_params&) = 0;

    static registrator& get_instance() MGBASE_NOEXCEPT {
        return *reg_;
    }
    
    static void set_instance(registrator& reg) {
        reg_ = &reg;
    }
    
protected:
    registrator() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
private:
    static registrator* reg_;
};

} // namespace rma
} // namespace mgcom

