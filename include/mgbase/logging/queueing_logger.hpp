
#pragma once

#include <iostream>
#include <deque>

#include <mgbase/profiling/clock.hpp>

namespace mgbase {

//#define MGBASE_ENABLE_QUEUED_LOGGER

class queueing_logger
{
    struct entry {
        cpu_clock_t clock;
        const char* msg;
        mgbase::int64_t arg1;
        mgbase::int64_t arg2;
        mgbase::int64_t arg3;
    };
    
    typedef std::deque<entry> container_type;
    
public:
    #ifdef MGBASE_ENABLE_QUEUED_LOGGER
    static void add_log(const char* msg, mgbase::int64_t arg1 = -1, mgbase::int64_t arg2 = -1, mgbase::int64_t arg3 = -1)
    {
        const entry e = { get_cpu_clock(), msg, arg1, arg2, arg3 };
        get_instance().entries_.push_back(e);
    }
    #else
    static void add_log(const char* /*msg*/, mgbase::int64_t /*arg1*/ = -1, mgbase::int64_t /*arg2*/ = -1, mgbase::int64_t /*arg3*/ = -1)
    {
    }
    #endif
    
    static void show(const char* header)
    {
        const queueing_logger& self = get_instance();
        for (container_type::const_iterator itr = self.entries_.begin(); itr != self.entries_.end(); ++itr) {
            std::cout << header << " " << itr->clock << " " << itr->msg << " " << itr->arg1 << " " << itr->arg2 << " " << itr->arg3 << std::endl;
        }
    }
    
private:
    queueing_logger() { }
    
    static queueing_logger& get_instance() {
        static queueing_logger instance;
        return instance;
    }
    
    std::deque<entry> entries_;
};

} // namespace mgbase

