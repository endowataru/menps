
#include "bench.hpp"
#include <unordered_map>

template <typename UltItf>
class bench_mtx
{
private:
    using thread = typename UltItf::thread;
    using mutex = typename UltItf::mutex;

public:
    explicit bench_mtx(const int argc, char** const argv) {
        this->nthreads_ = (argc > 1 ? atol(argv[1]) : 50);
        this->n_per_th_ = (argc > 1 ? atol(argv[2]) : 200000);
    }

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
            && ret.count == this->nthreads_ * this->n_per_th_;
    }
    static const char* name() { return "bench_mtx"; }

    using config_type = std::unordered_map<std::string, std::string>;
    config_type get_config() const {
        config_type c;
        c["nthreads"] = std::to_string(this->nthreads_);
        c["n_per_th"] = std::to_string(this->n_per_th_);
        return c;
    }

private:
    struct rec_params {
        bench_mtx& self;
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
            const auto n_per_th = self.n_per_th_;
            for (long i = 0; i < n_per_th; ++i) {
                self.mtx_.lock();
                ++*count;
                self.mtx_.unlock();
            }
            *r = a;
        }
        else {
            long c = (a + b) / 2;
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
    mutex   mtx_;
};

int main(const int argc, char** const argv) {
    return exec_bench<bench_mtx>(argc, argv);
}

