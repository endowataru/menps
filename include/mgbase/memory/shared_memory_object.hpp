
#pragma once

#include <mgbase/utility.hpp>
#include <mgbase/logger.hpp>

// for shm_open, shm_unlink
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace mgbase {

class shared_memory_object_error
    : public std::exception { };

class shared_memory_object
{
public:
    shared_memory_object()
        : fd_{-1} { }
    
    shared_memory_object(const char* const filename, int oflag, mode_t mode)
    {
        fd_ = shm_open(filename, oflag, mode);
        
        if (fd_ < 0) {
            MGBASE_LOG_FATAL(
                "msg:shm_open() failed.\t"
                "fd:{}\t"
                "errno:{}"
            ,   fd_
            ,   errno
            );
            
            throw shared_memory_object_error{};
        }
        
        MGBASE_LOG_DEBUG(
            "msg:Called shm_open().\t"
            "filename:{}\t"
            "oflag:{}\t"
            "mode:{}\t"
            "fd:{}"
        ,   filename
        ,   oflag
        ,   mode
        ,   fd_
        );
        
        filename_ = filename;
    }
    
    shared_memory_object(const shared_memory_object&) = delete;
    shared_memory_object& operator = (const shared_memory_object&) = delete;
    
    shared_memory_object(shared_memory_object&& other)
        : fd_{-1}
    {
        *this = mgbase::move(other);
    }
    
    shared_memory_object& operator = (shared_memory_object&& other)
    {
        unlink();
        
        fd_ = other.fd_;
        filename_ = mgbase::move(other.filename_);
        
        return *this;
    }
    
    ~shared_memory_object()
    {
        unlink();
    }
    
    int get_fd() const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(fd_ >= 0);
        return fd_;
    }
    
private:
    void unlink()
    {
        if (fd_ >= 0) {
            shm_unlink(filename_.c_str());
        }
    }
    
    int fd_;
    std::string filename_;
};

} // namespace mgbase
