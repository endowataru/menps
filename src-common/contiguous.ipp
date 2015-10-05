
#pragma once

#include <mgcom.hpp>

#include "impl.hpp"

#include "rma.hpp"

namespace mgcom {

namespace rma {

namespace detail {

namespace {

class remote_read_handlers {
public:
    typedef remote_read_cb      cb_type;
    
    static void start(cb_type& cb) {
        mgbase::control::enter<cb_type, try_>(cb);
    }

private:
    static void try_(cb_type& cb)
    {
        if (try_remote_read(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.size_in_bytes
        ,   make_notifier_finished(cb)
        )) {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class remote_write_handlers {
public:
    typedef remote_write_cb     cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }

private:
    static void try_(cb_type& cb)
    {
        if (try_remote_write(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.size_in_bytes
        ,   make_notifier_finished(cb)
        )) {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class remote_atomic_write_default_handlers {
public:
    typedef remote_atomic_write_default_cb  cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }
    
private:
    static void try_(cb_type& cb)
    {
        if (try_remote_atomic_write_default(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.buf_addr
        ,   make_notifier_finished(cb)
        )) {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class remote_atomic_read_default_handlers {
public:
    typedef remote_atomic_read_default_cb   cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }
    
private:
    static void try_(cb_type& cb)
    {
        if (try_remote_atomic_read_default(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.buf_addr
        ,   make_notifier_finished(cb)
        )) {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class remote_compare_and_swap_default_handlers {
public:
    typedef remote_compare_and_swap_default_cb  cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }
    
private:
    static void try_(cb_type& cb)
    {
        if (try_remote_compare_and_swap_default(
            cb.target_proc
        ,   cb.target_addr
        ,   cb.expected_addr
        ,   cb.desired_addr
        ,   cb.result_addr
        ,   make_notifier_finished(cb)
        )) {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class remote_fetch_and_add_default_handlers {
public:
    typedef remote_fetch_and_add_default_cb     cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }
    
private:
    static void try_(cb_type& cb)
    {
        if (try_remote_fetch_and_add_default(
            cb.target_proc
        ,   cb.target_addr
        ,   cb.value_addr
        ,   cb.result_addr
        ,   make_notifier_finished(cb)
        )) {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/)
    {
        poll();
    }
};

}

}

}

}

