
#include "bench.hpp"
#include <unordered_map>
#include <numeric>
#include <vector>

template <typename P>
class bench_mtx
{
private:
    using ult_itf_type = typename P::ult_itf_type;

    using thread = typename ult_itf_type::thread;
    using mutex = typename ult_itf_type::mutex;

public:
    explicit bench_mtx(const int argc, char** const argv) {
        this->nthreads_ = (argc > 1 ? atol(argv[1]) : 50);
        this->n_per_th_ = (argc > 2 ? atol(argv[2]) : 200000);
        this->num_mtxs_ = (argc > 3 ? atol(argv[3]) : 1);
        this->entries_ = cmpth::fdn::make_unique<entry []>(this->num_mtxs_);
    }

    struct result_type {
        long r;
        std::vector<long> counts;
    };
    result_type operator() () {
        long r = 0;
        rec_params p{ *this, 0, this->nthreads_, &r };
        rec(&p);
        std::vector<long> counts;
        for (long i = 0; i < this->num_mtxs_; ++i) {
            counts.push_back(this->entries_[i].count);
        }
        return { r, std::move(counts) };
    }
    bool is_correct(const result_type& ret) const {
        const auto sum = std::accumulate(ret.counts.begin(), ret.counts.end(), 0);
        return (ret.r == (this->nthreads_ - 1) * this->nthreads_ / 2)
            && (sum == this->nthreads_ * this->n_per_th_);
    }
    static const char* name() { return "bench_mtx"; }

    using config_type = std::unordered_map<std::string, std::string>;
    config_type get_config() const {
        config_type c;
        c["nthreads"] = std::to_string(this->nthreads_);
        c["n_per_th"] = std::to_string(this->n_per_th_);
        c["num_mtxs"] = std::to_string(this->num_mtxs_);
        return c;
    }
    long num_total_ops() const {
        return this->nthreads_ * this->n_per_th_;
    }

private:
    struct rec_params {
        bench_mtx& self;
        long a;
        long b;
        long* r;
    };
    static void rec(void* const p_void) {
        const auto& p = *static_cast<rec_params*>(p_void);
        auto& self = p.self;
        const auto a = p.a;
        const auto b = p.b;
        const auto r = p.r;
        if (b - a == 1) {
            const auto n_per_th = self.n_per_th_;
            const auto idx = a % self.num_mtxs_;
            auto& mtx = self.entries_[idx].mtx;
            auto& count = self.entries_[idx].count;
            for (long i = 0; i < n_per_th; ++i) {
                mtx.lock();
                ++count;
                mtx.unlock();
            }
            *r = a;
        }
        else {
            long c = (a + b) / 2;
            long r1 = 0, r2 = 0;
            rec_params p1{ self, a, c, &r1 };
            rec_params p2{ self, c, b, &r2 };
            auto t = thread::ptr_fork(&rec, &p1);
            rec(&p2);
            t.join();
            *r = r1 + r2;
        }
    }

    long    nthreads_ = 0;
    long    n_per_th_ = 0;
    long    num_mtxs_ = 0;
    struct entry {
        mutex   mtx;
        long    count = 0;
        char    pad[CMPTH_CACHE_LINE_SIZE - sizeof(long)];
    };
    std::unique_ptr<entry []> entries_;
};

int main(const int argc, char** const argv) {
    return exec_bench<bench_mtx>(argc, argv);
}

