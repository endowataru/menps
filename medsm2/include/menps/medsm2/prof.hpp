
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace medsm2 {

using clock_accumulator = mefdn::average_accumulator<mefdn::cpu_clock_t>;

#define MEDSM2_PROF_KINDS(x) \
    x(fence) \
    x(release) \
    x(rel_merge) \
    x(barrier_acq) \
    x(barrier_allgather)

#define DEFINE_PROF_KIND(name) name,

enum class prof_kind {
    MEDSM2_PROF_KINDS(DEFINE_PROF_KIND)
    end
};

#undef DEFINE_PROF_KIND

class prof
{
public:
    #ifdef MEDSM2_ENABLE_PROF
    static mefdn::cpu_clock_t start()
    {
        return mefdn::get_cpu_clock();
    }
    
    static void finish(const prof_kind k, mefdn::cpu_clock_t c)
    {
        const auto now = mefdn::get_cpu_clock();
        prof::get(k).add(now-c);
    }
    #else
    static mefdn::cpu_clock_t start()
    {
        return 0;
    }
    
    static void finish(const prof_kind /*k*/, mefdn::cpu_clock_t /*c*/)
    {
        // Do nothing.
    }
    #endif
    
    static clock_accumulator& get(const prof_kind k) {
        static clock_accumulator arr[static_cast<mefdn::size_t>(prof_kind::end)];
        MEFDN_ASSERT(k < prof_kind::end);
        return arr[static_cast<mefdn::size_t>(k)];
    }
    
    static std::string to_string(const char* const head)
    {
        fmt::MemoryWriter w;
        
        #define WRITE(name) \
            w.write("{}" #name ": {}\n", head, prof::get(prof_kind::name));
        
        MEDSM2_PROF_KINDS(WRITE)
        
        #undef WRITE
        
        return w.str();
    }
};

} // namespace medsm2
} // namespace menps

