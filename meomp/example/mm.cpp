
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <sys/time.h>
#ifdef _OPENMP
    #include <omp.h>
#endif

#ifdef MEOMP_ENABLED
    // meomp
    #include <menps/meomp.hpp>
#else
    // normal
    #define meomp_main  main
    static inline int meomp_get_num_procs(void) { return 1; }
    static inline int meomp_get_num_local_threads(void) {
        #ifdef _OPENMP
        return omp_get_num_threads();
        #else
        return 1;
        #endif
    }
    static inline void * meomp_malloc(size_t size) { return malloc(size); }
#endif

#ifdef BENCH_USE_BLAS
    #include <mkl.h>
#endif

using std::size_t;
using real_t = double;

inline double cur_time() {
    struct timeval tv[1];
    gettimeofday(tv, 0);
    return tv->tv_sec + tv->tv_usec * 1.0e-6;
}

class matrix
{
public:
    /*implicit*/ matrix(real_t* const p, const size_t num_rows, const size_t num_cols,
                        const size_t ld) noexcept
        : p_{p}, num_rows_{num_rows}, num_cols_{num_cols}, ld_{ld}
    { }

    void fill(const real_t val) noexcept {
        std::fill(this->p_, this->p_ + this->num_rows_*this->ld_, val);
    }
    
    real_t& operator () (const size_t i, const size_t j) noexcept {
        return this->p_[i * this->ld_ + j];
    }
    const real_t& operator () (const size_t i, const size_t j) const noexcept {
        return this->p_[i * this->ld_ + j];
    }

    real_t* raw() noexcept { return this->p_; }
    const real_t* raw() const noexcept { return this->p_; }
    size_t num_rows() const noexcept { return this->num_rows_; }
    size_t num_cols() const noexcept { return this->num_cols_; }
    size_t ld() const noexcept { return this->ld_; }

private:
    real_t* p_ = nullptr;
    size_t num_rows_ = 0;
    size_t num_cols_ = 0;
    size_t ld_ = 0;
};

void calc_gemm_in_parallel_naive(const matrix& A, const matrix& B, matrix& C)
{
    const auto M = C.num_rows();
    const auto N = C.num_cols();
    const auto K = A.num_cols();

    #pragma omp for nowait
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            for (size_t l = 0; l < K; ++l) {
                C(i, j) += A(i, l) * B(l, j);
            }
        }
    }
}

void calc_gemm_in_parallel(const matrix& A, const matrix& B, matrix& C)
{
    #ifdef BENCH_USE_BLAS
    const auto M = C.num_rows();
    const auto N = C.num_cols();
    const auto K = A.num_cols();

    const double alpha = 1.0;
    const double beta = 1.0;

    #ifdef _OPENMP
    const auto th_num = static_cast<size_t>(omp_get_thread_num());
    const auto nthreads = static_cast<size_t>(omp_get_num_threads());
    #else
    const int th_num = 0;
    const int nthreads = 1;
    #endif
    const auto N_per_th = (M + (nthreads - 1)) / nthreads;
    const auto i_first = N_per_th * th_num;
    if (i_first > M) {
        //std::cout << "Ignore thread=" << th_num << std::endl;
        return;
    }
    const auto i_last = std::min(N_per_th * (th_num+1), M);
    const auto i_len = i_last - i_first;

    //std::cout << "Exec cblas_gemm on thread=" << th_num << ": m=[" << i_first << " " << i_last << ")" << std::endl;

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
        i_len, N, K, alpha,
        &A(i_first, 0), A.ld(), B.raw(), B.ld(),
        beta, &C(i_first, 0), C.ld());

    #else
    calc_gemm_in_parallel_naive(A, B, C);
    #endif
}

extern "C"
int meomp_main(int argc, char** argv)
{
    if (argc != 6) {
        std::cerr << argv[0] << " [M] [ld] [num_warmups] [num_trials] [verify]" << std::endl;
        return EXIT_FAILURE;
    }

    const auto M = static_cast<size_t>(std::atoi(argv[1]));
    const auto ld = static_cast<size_t>(std::atoi(argv[2]));
    const auto num_warmups = static_cast<size_t>(std::atoi(argv[3]));
    const auto num_trials = static_cast<size_t>(std::atoi(argv[4]));
    const auto do_verify = std::atoi(argv[5]);
    const auto N = M;
    const auto K = M;

    auto buf_A = static_cast<real_t*>(meomp_malloc(M*ld*sizeof(real_t)));
    auto buf_B = static_cast<real_t*>(meomp_malloc(K*ld*sizeof(real_t)));
    auto buf_C = static_cast<real_t*>(meomp_malloc(M*ld*sizeof(real_t)));

    matrix A{buf_A, M, K, ld};
    matrix B{buf_B, K, N, ld};
    matrix C{buf_C, M, N, ld};
    const double C_init = 1.0;

    std::cout << "Initializing..." << std::endl;
    #pragma omp parallel for
    for (size_t i = 0; i < M; ++i) {
        for (size_t l = 0; l < K; ++l) {
            A(i, l) = i+l;
        }
    }
    #pragma omp parallel for
    for (size_t l = 0; l < K; ++l) {
        for (size_t j = 0; j < N; ++j) {
            B(l, j) = j*l;
        }
    }
    #pragma omp parallel for
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            C(i, j) = C_init;
        }
    }

    std::cout << "Warmup..." << std::endl;
    const auto warmup_t0 = cur_time();
    #pragma omp parallel
    {
        for (size_t warmup = 0; warmup < num_warmups; ++warmup) {
            calc_gemm_in_parallel(A, B, C);
        }
    }
    const auto warmup_t1 = cur_time();
    
    std::cout << "Calculating..." << std::endl;
    const auto calc_t0 = cur_time();
    #pragma omp parallel
    {
        for (size_t trial = 0; trial < num_trials; ++trial) {
            calc_gemm_in_parallel(A, B, C);
        }
    }
    const auto calc_t1 = cur_time();

    #pragma omp single
    {
        bool correct = true;
        size_t num_wrong = 0;

        if (do_verify) {
            std::cout << "Verifying..." << std::endl;
            
            auto buf_ansC = new real_t[M*ld];
            matrix ansC{buf_ansC, M, N, ld};
            ansC.fill(C_init);
            
            for (size_t i = 0; i < M; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    for (size_t l = 0; l < K; ++l) {
                        ansC(i, j) += A(i, l) * B(l, j) * (num_warmups+num_trials);
                    }
                }
            }
            for (size_t i = 0; i < M; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    if (!(std::abs(ansC(i, j) - C(i, j)) < 1e-4)) {
                        ++num_wrong;
                    }
                }
            }
            correct = num_wrong == 0;
        }
        
        std::stringstream ss;
        ss << "- bench_name: mm" << std::endl;
        ss << "  verified: " << do_verify << std::endl;
        if (do_verify) {
            ss << "  correct: " << correct << std::endl;
            ss << "  num_wrong: " << num_wrong << std::endl;
        }
        ss << "  M: " << M << std::endl;
        ss << "  ld: " << ld << std::endl;
        ss << "  num_warmups: " << num_warmups << std::endl;
        ss << "  num_trials: " << num_trials << std::endl;
        #ifdef _OPENMP
        const int num_threads = omp_get_max_threads();
        #else
        const int num_threads = 1;
        #endif
        ss << "  num_threads: " << num_threads << std::endl;
        ss << "  num_procs: " << meomp_get_num_procs() << std::endl;
        ss << "  num_local_threads: " << meomp_get_num_local_threads() << std::endl;
        ss << "  warmup_time: " << (warmup_t1 - warmup_t0) << " # [sec]"<< std::endl;
        if (correct) {
            const auto total_time = calc_t1-calc_t0;
            const auto time = total_time/num_trials;
            const auto flop = 2.0*M*N*K*num_trials;
            const auto flops = flop / time;
            ss << "  time: " << time << " # [sec]" << std::endl;
            ss << "  total_time: " << total_time << " # [sec]" << std::endl;
            ss << "  flop: " << flop << " # [FLOP]" << std::endl;
            ss << "  flops: " << flops << " # [FLOP/sec]" << std::endl;
            ss << "  flops_per_core: " << flops/num_threads << " # [FLOP/sec/core]" << std::endl;
        }
        
        const auto str = ss.str();
        std::cout << str;

        const char * file_path = getenv("BENCH_OUTPUT_PATH");
        if (file_path == NULL) { file_path = "output.yaml"; }
        std::ofstream ofs{file_path, std::ios::out | std::ios::app};   
        ofs << str;
    }

    #pragma omp barrier

    return EXIT_SUCCESS;
}

