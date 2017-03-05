
#pragma once

#include <mgbase/memory/shared_memory_object.hpp>
#include <mgbase/logger.hpp>

// for ftruncate
#include <unistd.h>

namespace mgdsm {

struct shm_object_error { };

class shm_object
{
public:
    struct config {
        const char*     filename;
        mgbase::size_t  size_in_bytes;
    };
    
    explicit shm_object(const config& conf)
    {
        using mgbase::shared_memory_object;
        
        try {
            obj_ = shared_memory_object(conf.filename, O_RDWR | O_CREAT | O_EXCL, 0664);
            
            MGBASE_LOG_VERBOSE("msg:Created shared memory object.");
        }
        catch (const mgbase::shared_memory_object_error&)
        {
            // Open the existing shared object.
            obj_ = shared_memory_object(conf.filename, O_RDWR | O_CREAT, 0664);
            
            // Unlink it.
            obj_ = shared_memory_object{};
            
            // Create again.
            obj_ = shared_memory_object(conf.filename, O_RDWR | O_CREAT | O_EXCL, 0664);
            
            MGBASE_LOG_VERBOSE("msg:Removed and created shared memory object.");
        }
        
        const int ret = ftruncate(obj_.get_fd(), conf.size_in_bytes);
        
        if (ret != 0) {
            MGBASE_LOG_FATAL(
                "msg:ftruncate() failed.\t"
                "ret:{}\t"
                "errno:{}"
            ,   ret
            ,   errno
            );
            
            obj_.~shared_memory_object();
            
            throw shm_object_error{};
        }
    }
    
    int get_fd() const MGBASE_NOEXCEPT {
        return obj_.get_fd();
    }
    
private:
    mgbase::shared_memory_object obj_;
};

} // namespace mgdsm

