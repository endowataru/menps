
#include "bench.hpp"
#include <unordered_map>

template <typename P>
class bench_create
{
private:
    using ult_itf_type = typename P::ult_itf_type;

    using thread = typename ult_itf_type::thread;
    using mutex = typename ult_itf_type::mutex;

public:
    explicit bench_create(const int argc, char** const argv) {
        this->nthreads_ = (argc > 1 ? atol(argv[1]) : 100000);
    }

    struct result_type {
        long r;
    };
    result_type operator() () {
        long r = 0;
        rec_params p{ *this, 0, this->nthreads_, &r };
        rec(&p);
        return { r };
    }
    bool is_correct(const result_type& ret) const {
        return ret.r == (this->nthreads_ - 1) * this->nthreads_ / 2;
    }
    static const char* name() { return "bench_create"; }

    using config_type = std::unordered_map<std::string, std::string>;
    config_type get_config() const {
        config_type c;
        c["nthreads"] = std::to_string(this->nthreads_);
        return c;
    }

private:
    struct rec_params {
        bench_create& self;
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
};

int main(const int argc, char** const argv) {
    return exec_bench<bench_create>(argc, argv);
}

