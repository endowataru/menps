
#pragma once

#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/thread/spinlock.hpp>
#include <menps/mefdn/mutex.hpp>

namespace menps {
namespace mefdn {

template <typename P, bool Enabled = P::is_enabled>
class basic_prof;

template <typename P>
class basic_prof<P, true>
{
private:
    using prof_kind_type = typename P::prof_kind_type;
    
    using spinlock_type = mefdn::spinlock;
    using accumulator_type = mefdn::average_accumulator<mefdn::cpu_clock_t>;
    
    struct entry {
        spinlock_type       lock;
        accumulator_type    acc;
    };
    
    static entry& get(const prof_kind_type k) {
        static entry arr[static_cast<mefdn::size_t>(prof_kind_type::end)];
        MEFDN_ASSERT(k < prof_kind_type::end);
        return arr[static_cast<mefdn::size_t>(k)];
    }
    
public:
    static mefdn::cpu_clock_t start()
    {
        return mefdn::get_cpu_clock();
    }
    
    static void finish(const prof_kind_type k, mefdn::cpu_clock_t c)
    {
        const auto now = mefdn::get_cpu_clock();
        auto& e = basic_prof::get(k);
        mefdn::lock_guard<spinlock_type> lk(e.lock);
        e.acc.add(now-c);
    }
    
    static void add(const prof_kind_type k, mefdn::cpu_clock_t c)
    {
        auto& e = basic_prof::get(k);
        mefdn::lock_guard<spinlock_type> lk(e.lock);
        e.acc.add(c);
    }
    
    static std::string to_string(const char* const head)
    {
        fmt::MemoryWriter w;
        
        for (mefdn::size_t i = 0;
             i < static_cast<mefdn::size_t>(prof_kind_type::end); ++i)
        {
            const auto kind = static_cast<prof_kind_type>(i);
            
            w.write("{}{}: {}\n", head, P::get_name(kind),
                basic_prof::get(kind).acc);
        }
        
        return w.str();
    }
};

template <typename P>
class basic_prof<P, false>
{
private:
    using prof_kind_type = typename P::prof_kind_type;
    
public:
    static mefdn::cpu_clock_t start() {
        return 0;
    }
    
    static void finish(const prof_kind_type /*k*/, mefdn::cpu_clock_t /*c*/) {
        // Do nothing.
    }
    
    static std::string to_string(const char* const /*head*/) {
        return "";
    }
};

} // namespace mefdn
} // namespace menps

