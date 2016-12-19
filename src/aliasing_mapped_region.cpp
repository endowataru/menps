
#include "aliasing_mapped_region.hpp"

#include <mgbase/logger.hpp>

#include <mgbase/memory/shared_memory_object.hpp>
#include <mgbase/memory/mapped_memory.hpp>

// for shm_open, shm_unlink, mmap
#include <sys/mman.h>
#include <fcntl.h>

// for stat
#include <sys/stat.h>

// for ftruncate
#include <unistd.h>

#include <errno.h>

namespace mgdsm {

class aliasing_mapped_region_error
    : public std::exception { };

class aliasing_mapped_region::impl
{
public:
    explicit impl(const config& conf)
        : conf_(conf)
    {
        using mgbase::shared_memory_object;
        
        try {
            obj_ = shared_memory_object(conf_.filename, O_RDWR | O_CREAT | O_EXCL, 0664);
            
            MGBASE_LOG_VERBOSE("msg:Created shared memory object.");
        }
        catch (const mgbase::shared_memory_object_error&)
        {
            // Open the existing shared object.
            obj_ = shared_memory_object(conf_.filename, O_RDWR | O_CREAT, 0664);
            
            // Unlink it.
            obj_ = shared_memory_object{};
            
            // Create again.
            obj_ = shared_memory_object(conf_.filename, O_RDWR | O_CREAT | O_EXCL, 0664);
            
            MGBASE_LOG_VERBOSE("msg:Removed and created shared memory object.");
        }
        
        const int ret = ftruncate(obj_.get_fd(), conf_.size_in_bytes);
        
        if (ret != 0) {
            MGBASE_LOG_FATAL(
                "msg:ftruncate() failed.\t"
                "ret:{}\t"
                "errno:{}"
            ,   ret
            ,   errno
            );
            
            obj_.~shared_memory_object();
            
            throw aliasing_mapped_region_error{};
        }
        
        app_ = mgbase::mapped_memory::map(
            conf_.app_ptr
        ,   conf_.size_in_bytes
        ,   PROT_NONE//PROT_READ | PROT_WRITE
        ,   MAP_FIXED | MAP_SHARED
        ,   obj_.get_fd()
        ,   0
        );
        
        sys_ = mgbase::mapped_memory::map(
            conf_.sys_ptr
        ,   conf_.size_in_bytes
        ,   PROT_READ | PROT_WRITE
        ,   MAP_FIXED | MAP_SHARED
        ,   obj_.get_fd()
        ,   0
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Allocated cache area.\t"
            "fd:{}\t"
            "app:{:x}\t"
            "sys:{:x}"
        ,   obj_.get_fd()
        ,   reinterpret_cast<mgbase::uintptr_t>(app_.get())
        ,   reinterpret_cast<mgbase::uintptr_t>(sys_.get())
        );
    }
    
    ~impl()
    {
        MGBASE_LOG_DEBUG(
            "msg:Deallocating cache area."
        );
    }
    
    void* get_app_ptr() const MGBASE_NOEXCEPT {
        return app_.get();
    }
    void* get_sys_ptr() const MGBASE_NOEXCEPT {
        return sys_.get();
    }
    
    mgbase::size_t get_size_in_bytes() const MGBASE_NOEXCEPT {
        return conf_.size_in_bytes;
    }
    
private:
    const config conf_;
    
    mgbase::shared_memory_object obj_;
    mgbase::mapped_memory app_;
    mgbase::mapped_memory sys_;
};

aliasing_mapped_region::aliasing_mapped_region(const config& conf)
    : impl_(new impl(conf))
    { }

aliasing_mapped_region::~aliasing_mapped_region() = default;


void* aliasing_mapped_region::get_app_ptr() const MGBASE_NOEXCEPT
{
    return impl_->get_app_ptr();
}

void* aliasing_mapped_region::get_sys_ptr() const MGBASE_NOEXCEPT
{
    return impl_->get_sys_ptr();
}

mgbase::size_t aliasing_mapped_region::size_in_bytes() const MGBASE_NOEXCEPT
{
    return impl_->get_size_in_bytes();
}

} // namespace mgdsm

