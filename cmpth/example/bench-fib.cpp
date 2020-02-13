
#include "bench.hpp"
#include <unordered_map>
#include <vector>

template <typename P>
class bench_fib
{
private:
    using ult_itf_type = typename P::ult_itf_type;

    using thread = typename ult_itf_type::thread;
    using mutex = typename ult_itf_type::mutex;
    
    using fib_int_t = std::uint64_t;

public:
    explicit bench_fib(const int argc, char** const argv) {
        this->n_ = (argc > 1 ? atol(argv[1]) : 37);
    }

    struct result_type {
        fib_int_t r;
    };
    result_type operator() () {
        fib_int_t r = 0;
        rec_params p{ *this, this->n_, &r };
        rec(&p);
        return { r };
    }
    bool is_correct(const result_type& ret) const {
        static std::vector<fib_int_t> ans{
            0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393, 196418, 317811, 514229, 832040, 1346269, 2178309, 3524578, 5702887, 9227465, 14930352, 24157817, 39088169, 63245986
        };
        return this->n_ < ans.size() && ret.r == ans[this->n_];
    }
    static const char* name() { return "bench_fib"; }

    using config_type = std::unordered_map<std::string, std::string>;
    config_type get_config() const {
        config_type c;
        c["n"] = std::to_string(this->n_);
        return c;
    }
    long num_total_ops() const {
        return 1; // TODO
    }

private:
    struct rec_params {
        bench_fib& self;
        fib_int_t n;
        fib_int_t* r;
    };
    static void rec(void* const p_void) {
        const auto& p = *static_cast<rec_params*>(p_void);
        auto& self = p.self;
        const auto n = p.n;
        const auto r = p.r;
        if (n == 0 || n == 1) {
            *r = n;
        }
        else {
            fib_int_t r1 = 0, r2 = 0;
            rec_params p1{ self, n-1, &r1 };
            rec_params p2{ self, n-2, &r2 };
            auto t = thread::ptr_fork(&rec, &p1);
            rec(&p2);
            t.join();
            *r = r1 + r2;
        }
    }

    fib_int_t    n_ = 0;
};

int main(const int argc, char** const argv) {
    return exec_bench<bench_fib>(argc, argv);
}
