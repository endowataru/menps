
#include <menps/meult/qd/qdlock_delegator.hpp>
#include <menps/meult/backend/mth.hpp>
#include <menps/meult/qd/qdlock_mutex.hpp>

using ult_itf_t = menps::meult::backend::mth::ult_policy;

struct del_exec_result {
    bool is_executed;
    bool is_active;
    bool needs_awake;
    ult_itf_t::uncond_variable* awake_uv;
};

struct progress_result {
    bool is_active;
    bool needs_awake;
    ult_itf_t::uncond_variable* awake_uv;
};

struct del_result {
    bool needs_wait;
    ult_itf_t::uncond_variable* wait_uv;
};

int main()
{
    using lock_t = menps::meult::qdlock_delegator<int, ult_itf_t>;
    lock_t::qdlock_pool_type p;
    lock_t l(p);
    
    l.start_consumer(
        [] (lock_t::qdlock_node_type& n) {
            fmt::print("del: {}\n", n.func);
            return del_exec_result{ true, false, false, nullptr };
        },
        [] () {
            fmt::print("prog\n");
            return progress_result{ false, false, nullptr };
        }
    );
    
    l.lock();
    if (l.lock_or_delegate(
        [] (lock_t::qdlock_node_type& n) {
            fmt::print("del: {}\n", n.func);
            return del_result{ false, nullptr };
        }))
    {
        fmt::print("locked\n");
        
        l.unlock();
    }
    l.unlock();
    
    using lock2_t = menps::meult::qdlock_mutex<ult_itf_t>;
    lock2_t l2;
    l2.lock();
    l2.unlock();
    
    #if 0
    myqdlock_pool p;
    myqdlock l;/*(p, 
        [] (myqdlock_node& n) {
            fmt::print("del: {}\n", n.i);
            return false;
        },
        [] () {
            fmt::print("prog\n");
            return false;
        }
    );*/
    
    l.lock();
    l.unlock();
    #endif
    
    #if 0
    l.produce(
        [] () {
            fmt::print("imm: {}\n", 123);
            return false;
        },
        [] (myqdlock_node& n) {
            fmt::print("del");
            n.i = 123;
        }
    );
    #endif
}

