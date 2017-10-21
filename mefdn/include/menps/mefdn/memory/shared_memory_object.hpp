
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/logger.hpp>

// for shm_open, shm_unlink
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace menps {
namespace mefdn {

class shared_memory_object_error
    : public std::exception { };

class shared_memory_object
{
public:
    shared_memory_object()
        : fd_{-1} { }
    
    shared_memory_object(const char* const filename, const int oflag, const mode_t mode)
    {
        fd_ = shm_open(filename, oflag, mode);
        
        if (fd_ < 0) {
            MEFDN_LOG_FATAL(
                "msg:shm_open() failed.\t"
                "filename:{}\t"
                "oflag:{}\t"
                "mode:{}\t"
                "fd:{}\t"
                "errno:{}"
            ,   filename
            ,   oflag
            ,   mode
            ,   fd_
            ,   errno
            );
            
            throw shared_memory_object_error{};
        }
        
        MEFDN_LOG_DEBUG(
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
        *this = mefdn::move(other);
    }
    
    shared_memory_object& operator = (shared_memory_object&& other)
    {
        unlink();
        
        fd_ = other.fd_;
        filename_ = mefdn::move(other.filename_);
        
        return *this;
    }
    
    ~shared_memory_object()
    {
        unlink();
    }
    
    int get_fd() const noexcept
    {
        MEFDN_ASSERT(fd_ >= 0);
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

} // namespace mefdn
} // namespace menps

