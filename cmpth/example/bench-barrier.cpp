
#include "bench.hpp"
#include <unordered_map>

template <typename P>
class bench_barrier
{
private:
    using ult_itf_type = typename P::ult_itf_type;

    using thread = typename ult_itf_type::thread;
    using barrier = typename ult_itf_type::barrier;

public:
    explicit bench_barrier(const int argc, char** const argv)
        : nthreads_{(argc > 1 ? atol(argv[1]) : 50)}
        , n_per_th_{(argc > 2 ? atol(argv[2]) : 300000)}
        , bar_{this->nthreads_}
    { }

    struct result_type {
        long r;
        long count;
    };
    result_type operator() () {
        long r = 0;
        long count = 0;
        rec_params p{ *this, 0, this->nthreads_, &r, &count };
        rec(&p);
        return { r, count };
    }
    bool is_correct(const result_type& ret) const {
        return ret.r == (this->nthreads_ - 1) * this->nthreads_ / 2
            && ret.count == this->n_per_th_;
    }
    static const char* name() { return "bench_barrier"; }

    using config_type = std::unordered_map<std::string, std::string>;
    config_type get_config() const {
        config_type c;
        c["nthreads"] = std::to_string(this->nthreads_);
        c["n_per_th"] = std::to_string(this->n_per_th_);
        return c;
    }
    long num_total_ops() const {
        return this->n_per_th_;
    }

private:
    struct rec_params {
        bench_barrier& self;
        long a;
        long b;
        long* r;
        long* count;
    };
    static void rec(void* const p_void) {
        const auto& p = *static_cast<rec_params*>(p_void);
        auto& self = p.self;
        const auto a = p.a;
        const auto b = p.b;
        const auto r = p.r;
        const auto count = p.count;
        if (b - a == 1) {
            const auto nthreads = self.nthreads_;
            const auto n_per_th = self.n_per_th_;
            for (long i = 0; i < n_per_th; ++i) {
                CMPTH_P_PROF_SCOPE(P, bench_event);
                if (i % nthreads == a) {
                    ++*count;
                }
                self.bar_.arrive_and_wait();
            }
            *r = a;
        }
        else {
            const long c = (a + b) / 2;
            long r1 = 0, r2 = 0;
            rec_params p1{ self, a, c, &r1, count };
            rec_params p2{ self, c, b, &r2, count };
            auto t = thread::ptr_fork(&rec, &p1);
            rec(&p2);
            t.join();
            *r = r1 + r2;
        }
    }

    long    nthreads_ = 0;
    long    n_per_th_ = 0;
    barrier bar_;
    unsigned char pad_[CMPTH_CACHE_LINE_SIZE];
};

int main(const int argc, char** const argv) {
    return exec_bench<bench_barrier>(argc, argv);
}

