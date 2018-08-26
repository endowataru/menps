
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/thread/spinlock.hpp>

namespace menps {
namespace medsm2 {

#define MEDSM2_PROF_KINDS(x) \
    x(fence) \
    x(release) \
    x(lock_global) \
    x(begin_tx) \
    x(tx_merge) \
    x(end_tx) \
    x(unlock_global) \
    x(barrier_allgather) \
    x(barrier_acq) \
    x(push_wn) \
    x(start_write_twin) \
    x(tx_read) \
    x(merge_read) \
    x(wn_read)

#define DEFINE_PROF_KIND(name) name,

enum class prof_kind {
    MEDSM2_PROF_KINDS(DEFINE_PROF_KIND)
    end
};

#undef DEFINE_PROF_KIND

class prof
{
#ifdef MEDSM2_ENABLE_PROF
private:
    using spinlock_type = mefdn::spinlock;
    using accumulator_type = mefdn::average_accumulator<mefdn::cpu_clock_t>;
    
    struct entry {
        spinlock_type       lock;
        accumulator_type    acc;
    };
    
    static entry& get(const prof_kind k) {
        static entry arr[static_cast<mefdn::size_t>(prof_kind::end)];
        MEFDN_ASSERT(k < prof_kind::end);
        return arr[static_cast<mefdn::size_t>(k)];
    }
    
public:
    static mefdn::cpu_clock_t start()
    {
        return mefdn::get_cpu_clock();
    }
    
    static void finish(const prof_kind k, mefdn::cpu_clock_t c)
    {
        const auto now = mefdn::get_cpu_clock();
        auto& e = prof::get(k);
        mefdn::lock_guard<spinlock_type> lk(e.lock);
        e.acc.add(now-c);
    }
    
    static std::string to_string(const char* const head)
    {
        fmt::MemoryWriter w;
        
        #define WRITE(name) \
            w.write("{}" #name ": {}\n", head, prof::get(prof_kind::name).acc);
        
        MEDSM2_PROF_KINDS(WRITE)
        
        #undef WRITE
        
        return w.str();
    }
#else
public:
    static mefdn::cpu_clock_t start() {
        return 0;
    }
    
    static void finish(const prof_kind /*k*/, mefdn::cpu_clock_t /*c*/) {
        // Do nothing.
    }
    
    static std::string to_string(const char* const /*head*/) {
        return "";
    }
#endif
};

} // namespace medsm2
} // namespace menps

