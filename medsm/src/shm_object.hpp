
#pragma once

#include <menps/mefdn/memory/shared_memory_object.hpp>
#include <menps/mefdn/logger.hpp>

// for ftruncate
#include <unistd.h>

namespace menps {
namespace medsm {

struct shm_object_error { };

class shm_object
{
public:
    struct config {
        const char*     filename;
        mefdn::size_t  size_in_bytes;
    };
    
    explicit shm_object(const config& conf)
    {
        using mefdn::shared_memory_object;
        
        try {
            obj_ = shared_memory_object(conf.filename, O_RDWR | O_CREAT | O_EXCL, 0664);
            
            MEFDN_LOG_VERBOSE("msg:Created shared memory object.");
        }
        catch (const mefdn::shared_memory_object_error&)
        {
            // Open the existing shared object.
            obj_ = shared_memory_object(conf.filename, O_RDWR | O_CREAT, 0664);
            
            // Unlink it.
            obj_ = shared_memory_object{};
            
            // Create again.
            obj_ = shared_memory_object(conf.filename, O_RDWR | O_CREAT | O_EXCL, 0664);
            
            MEFDN_LOG_VERBOSE("msg:Removed and created shared memory object.");
        }
        
        const int ret = ftruncate(obj_.get_fd(), conf.size_in_bytes);
        
        if (ret != 0) {
            MEFDN_LOG_FATAL(
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
    
    int get_fd() const noexcept {
        return obj_.get_fd();
    }
    
private:
    mefdn::shared_memory_object obj_;
};

} // namespace medsm
} // namespace menps

