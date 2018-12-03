
#include <menps/meult/qd/qdlock_delegator.hpp>
#include <menps/meult/backend/mth.hpp>
#include <menps/meult/qd/qdlock_mutex.hpp>

struct del_exec_result {
    bool is_executed;
    bool is_active;
};

struct progress_result {
    bool is_active;
};

int main()
{
    using lock_t = menps::meult::qdlock_delegator<int, menps::meult::backend::mth::ult_policy>;
    lock_t::qdlock_pool_type p;
    lock_t l(p);
    
    l.start_consumer(
        [] (lock_t::qdlock_node_type& n) {
            fmt::print("del: {}\n", n.func);
            return del_exec_result{ true, false };
        },
        [] () {
            fmt::print("prog\n");
            return progress_result{ false };
        }
    );
    
    l.lock();
    if (l.lock_or_delegate(
        [] (lock_t::qdlock_node_type& n) {
            fmt::print("del: {}\n", n.func);
        }))
    {
        fmt::print("locked\n");
        
        l.unlock();
    }
    l.unlock();
    
    using lock2_t = menps::meult::qdlock_mutex<menps::meult::backend::mth::ult_policy>;
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

