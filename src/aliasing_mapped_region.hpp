
#pragma once

#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class aliasing_mapped_region
{
public:
    struct config {
        const char*     filename;
        mgbase::size_t  size_in_bytes;
        void*           app_ptr;
        void*           sys_ptr;
    };
    
    explicit aliasing_mapped_region(const config&);
    
    ~aliasing_mapped_region();
    
    void* get_app_ptr() const MGBASE_NOEXCEPT;
    
    void* get_sys_ptr() const MGBASE_NOEXCEPT;
    
    mgbase::size_t size_in_bytes() const MGBASE_NOEXCEPT;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgdsm

