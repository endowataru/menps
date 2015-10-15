
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/tls.hpp>

namespace mgbase {
namespace urcu {

typedef mgbase::int64_t counter_t;

namespace detail {

struct thread_entry
{
    counter_t       counter;
    thread_entry*   next;
};

struct global
{
    counter_t           counter;
    mgbase::spinlock    registration_lock;
    thread_entry*       first;
};


inline global& get_global() {
    static global g;
    return g;
}

inline thread_entry& get_thread_entry() {
    static MGBASE_THREAD_LOCAL thread_entry e = { 0, 0 };
    return e;
}

inline void update()
{
    
}

struct constants {
    
};

}

namespace {

inline void read_lock()
{
    counter_t& thread_counter = get_thread_entry().counter;
    
    const counter_t current = thread_counter;
    
    if ((current & rcu_nest_mask) != 0) {
        // FIXME: atomic load
        const counter_t proc_clock = get_global().counter;
        
        // FIXME: atomic store
        thread_counter = proc_clock;
        
        // FIXME: memory barrier (or memory_order_release ?)
        // barrier();
    }
    else {
        // FIXME: atomic store
        thread_counter = current + constants::rcu_nest_count;
    }
}

inline void read_unlock()
{
    counter_t& counter = get_thread_entry().counter;
    
    // FIXME: atomic store
    counter = counter - constants::rcu_nest_count;
}

inline void synchronize_nb(synchronize_cb& cb) {
    
}

}

}
}


