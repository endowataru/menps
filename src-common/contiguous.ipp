
#pragma once

#include <mgcom.hpp>

#include "impl.hpp"

#include "rma.hpp"

namespace mgcom {

namespace rma {

namespace detail {

namespace {

class read_handlers {
public:
    static void start(read_cb& cb) {
        mgbase::control::enter<read_cb, try_>(cb);
    }

private:
    static void try_(read_cb& cb)
    {
        if (try_read(cb.local_addr, cb.remote_addr, cb.size_in_bytes, cb.dest_proc,
            make_notifier_finished(cb)))
        {
            mgbase::control::enter<read_cb, test>(cb);
        }
    }
    
    static void test(read_cb& /*cb*/) {
        poll();
    }
};

class write_handlers {
public:
    static void start(write_cb& cb)
    {
        mgbase::control::enter<write_cb, try_>(cb);
    }

private:
    static void try_(write_cb& cb)
    {
        if (try_write(cb.local_addr, cb.remote_addr, cb.size_in_bytes,
            cb.dest_proc, make_notifier_finished(cb)))
        {
            mgbase::control::enter<write_cb, test>(cb);
        }
    }
    
    static void test(write_cb& /*cb*/) {
        poll();
    }
};

class atomic_write_64_handlers {
public:
    typedef atomic_write_64_cb  cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }
    
private:
    static void try_(cb_type& cb)
    {
        if (try_atomic_write_64(cb.local_addr, cb.buf_addr, cb.remote_addr, cb.dest_proc,
            make_notifier_finished(cb)))
        {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class atomic_read_64_handlers {
public:
    typedef atomic_read_64_cb  cb_type;
    
    static void start(cb_type& cb)
    {
        mgbase::control::enter<cb_type, try_>(cb);
    }
    
private:
    static void try_(cb_type& cb)
    {
        if (try_atomic_read_64(cb.local_addr, cb.buf_addr, cb.remote_addr, cb.src_proc,
            make_notifier_finished(cb)))
        {
            mgbase::control::enter<cb_type, test>(cb);
        }
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class compare_and_swap_64_handlers {
public:
    static void start(compare_and_swap_64_cb& cb)
    {
        mgbase::control::enter<compare_and_swap_64_cb, try_>(cb);
    }
    
private:
    static void try_(compare_and_swap_64_cb& cb)
    {
        if (try_compare_and_swap_64(cb.remote_addr, &cb.expected, &cb.desired,
            cb.result, cb.dest_proc, make_notifier_finished(cb)))
        {
            mgbase::control::enter<compare_and_swap_64_cb, test>(cb);
        }
    }
    
    static void test(compare_and_swap_64_cb& /*cb*/) {
        poll();
    }
};

class fetch_and_add_64_handlers {
public:
    static void start(fetch_and_op_64_cb& cb)
    {
        mgbase::control::enter<fetch_and_op_64_cb, try_>(cb);
    }
    
private:
    static void try_(fetch_and_op_64_cb& cb)
    {
        if (try_fetch_and_add_64(cb.remote_addr, &cb.value, cb.result,
            cb.dest_proc, make_notifier_finished(cb)))
        {
            mgbase::control::enter<fetch_and_op_64_cb, test>(cb);
        }
    }
    
    static void test(fetch_and_op_64_cb& /*cb*/)
    {
        poll();
    }
};

}

}

}

}

