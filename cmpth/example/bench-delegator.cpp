
// TODO: suspended_thread is not supported in ABT
#define BENCH_AVOID_ABT
#define BENCH_AVOID_DUMMY
#define BENCH_AVOID_KTH
#include "bench.hpp"

namespace fdn = cmpth::fdn;

template <typename P>
class my_consumer {
    using ult_itf_type = typename P::ult_itf_type;
    using suspended_thread = typename ult_itf_type::suspended_thread;

public:
    struct delegated_func_type {
        fdn::size_t i;
    };
    fdn::tuple<bool, suspended_thread> execute(delegated_func_type& f) {
        this->increase(f.i);
        //std::cout << this->x_ << std::endl;
        return fdn::make_tuple(true, suspended_thread{});
    }
    bool is_active() const noexcept {
        return false;
    }
    suspended_thread progress() {
        return suspended_thread{};
    }

    void increase(long i) {
        this->x_ += i;
    }
    long get_result() { return this->x_; }

private:
    long x_ = 0;
};

template <typename P>
struct my_delegator_policy {
    using consumer_type = my_consumer<P>;
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& /*pool*/) {
        return 128; // TODO
    }
};

template <typename P>
class bench_delegator
{
    using ult_itf_type = typename P::ult_itf_type;
    using delegator_type =
        typename ult_itf_type::template delegator_t<my_delegator_policy<P>>;
    using consumer_type = typename delegator_type::consumer_type;
    using delegated_func_type = typename consumer_type::delegated_func_type;

    using thread = typename ult_itf_type::thread;
    using suspended_thread = typename ult_itf_type::suspended_thread;

public:
    explicit bench_delegator(int argc, char** argv) {
        this->nthreads_ = (argc > 1 ? atol(argv[1]) : 50);
        this->n_per_th_ = (argc > 2 ? atol(argv[2]) : 200000);
        del_.start_consumer(c_);
    }
    ~bench_delegator() {
        del_.stop_consumer();
    }

    static const char* name() { return "bench_delegator"; }

    struct result_type {
        long r;
    };
    result_type operator() () {
        long r = 0;
        rec_params p{ *this, 0, this->nthreads_, &r };
        rec(&p);
        while (c_.get_result() < this->nthreads_ * this->n_per_th_) {
            ult_itf_type::this_thread::yield();
            //std::cout << c_.get_result() << std::endl;
        }
        return { c_.get_result() };
    }
    bool is_correct(const result_type& ret) const {
        return ret.r == this->nthreads_ * this->n_per_th_;
    }
    using config_type = std::unordered_map<std::string, std::string>;
    config_type get_config() const {
        config_type c;
        c["nthreads"] = std::to_string(this->nthreads_);
        c["n_per_th"] = std::to_string(this->n_per_th_);
        return c;
    }
    long num_total_ops() const {
        return this->nthreads_ * this->n_per_th_;
    }

private:
    struct rec_params {
        bench_delegator& self;
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
            auto& del = self.del_;
            for (long i = 0; i < n_per_th; ++i) {
                del.execute_or_delegate(
                    [&self] () {
                        self.c_.increase(1);
                        return fdn::make_tuple(true, nullptr);
                    },
                    [] (delegated_func_type& f) -> suspended_thread* {
                        f = { 1 };
                        return nullptr;
                    }
                );
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

    long nthreads_ = 0;
    long n_per_th_ = 0;
    consumer_type c_;
    fdn::byte pad_[CMPTH_CACHE_LINE_SIZE - sizeof(consumer_type)];
    delegator_type del_;
};

int main(const int argc, char** const argv) {
    return exec_bench<bench_delegator>(argc, argv);
}

