
#pragma once

#include <menps/medev2/mpi/mpi_funcs.hpp>
#include <menps/mefdn/logger.hpp>
#include <exception>
#ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
#include <menps/mefdn/thread/spinlock.hpp>
#endif

namespace menps {
namespace medev2 {
namespace mpi {

struct mpi_error : std::exception
{
    static void check(const int err_code) {
        if (err_code != MPI_SUCCESS) {
            throw mpi_error();
        }
    }
};

#ifdef MEDEV2_SERIALIZE_MPI_CALLS
    #define MPI_CRITICAL    unique_lock_type lk(this->mtx_);
#else
    #define MPI_CRITICAL
#endif

template <typename P>
class direct_mpi_facade
{
    using ult_itf_type = typename P::ult_itf_type;
    #ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
    using mutex_type = typename ult_itf_type::spinlock;
    using unique_lock_type = mefdn::unique_lock<mutex_type>;
    #else
    using mutex_type = typename ult_itf_type::mutex;
    using unique_lock_type = typename ult_itf_type::unique_mutex_lock;
    #endif
    
public:
    explicit direct_mpi_facade(
        int* const      argc
    ,   char*** const   argv
    ,   const int       required
    ,   int* const      provided
    ) {
        mpi_error::check(
            MPI_Init_thread(argc, argv, required, provided)
        );
    }
    
    direct_mpi_facade(const direct_mpi_facade&) = delete;
    direct_mpi_facade& operator = (const direct_mpi_facade&) = delete;
    
    ~direct_mpi_facade()
    {
        MPI_Finalize();
    }
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const name##_params& p) { \
            MEFDN_LOG_DEBUG( \
                "msg:Entering MPI_" #Name ".\t" \
                MEDEV2_EXPAND_PARAMS_TO_LOG_FMT(num, __VA_ARGS__) \
            ,   MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS(num, __VA_ARGS__) \
            ); \
            { \
                MPI_CRITICAL \
                mpi_error::check( \
                    MPI_##Name( \
                        MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__) \
                    ) \
                ); \
            } \
            MEFDN_LOG_DEBUG( \
                "msg:Exiting MPI_" #Name ".\t" \
                MEDEV2_EXPAND_PARAMS_TO_LOG_FMT(num, __VA_ARGS__) \
            ,   MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS(num, __VA_ARGS__) \
            ); \
        }
    
    MEDEV2_MPI_FUNCS_ALL(D, /*dummy*/)
    
    #undef D
    
    void progress()
    {
        MEFDN_LOG_VERBOSE("msg:Entering MPI progress.");
        
        // Issue probing to forward the progress.
        int flag = 0;
        this->iprobe({
            MPI_ANY_SOURCE
        ,   MPI_ANY_TAG
        ,   MPI_COMM_WORLD
        ,   &flag
        ,   MPI_STATUS_IGNORE
        });
        
        MEFDN_LOG_VERBOSE("msg:Exiting MPI progress.");
    }
    
    #ifdef MEDEV2_SERIALIZE_MPI_CALLS
private:
    mutex_type mtx_;
    #endif
};

template <typename UltItf>
struct direct_mpi_facade_policy
{
    using ult_itf_type = UltItf;
};

template <typename UltItf>
struct direct_mpi_itf
{
    using mpi_facade_type = direct_mpi_facade<direct_mpi_facade_policy<UltItf>>;
    using ult_itf_type = UltItf;
};

} // namespace mpi
} // namespace medev2
} // namespace menps

