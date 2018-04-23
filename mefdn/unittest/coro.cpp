
#include "unittest.hpp"
#include <menps/mefdn/coro/slc.hpp>
#include <menps/mefdn/coro/sfc.hpp>
#include <menps/mefdn/coro/klt_worker.hpp>
#include <iostream>

/*
    struct frame_base
    {
        template <template <typename> class... ChildFrames>
        using define_children = ???;
        
        using children = define_children<>;
        
        using fiber = ???;
    };
    
    struct label_base
    {
        using return_type = ???;
        
        template <template <typename> class NextLabel, typename... Args>
        return_type jump(Args&&... args);
        
        template <template <typename> class NextLabel, template <typename> class ChildFrame, typename... Args>
        return_type call(Args&&... args)
        
        template <template <typename> class NextLabel, template <typename> class ChildFrame, typename... Args>
        return_type call_rec(Args&&... args)
        
        template <typename... Args>
        return_type set_return(Args&&... args);
        
        template <template <typename> class NextLabel, typename Func, typename... Args>
        return_type suspend(Func&& func, Args&&... args);
        
        //template <template <typename> class NextLabel, template <typename> class ChildFrame, typename... Args>
        //return_type fork(fiber*, Args&&... args);
            // always no argument for the continuation
        
        template <template <typename> class NextLabel, template <typename> class ChildFrame, typename... Args>
        return_type fork_rec(fiber*, Args&&... args);
        
        template <template <typename> class NextLabel>
        return_type join(fiber&);
    };
*/


namespace /*unnamed*/ {

template <typename F>
struct identity_coro : F {
    using result_type = int;
    
    template <typename L>
    struct start : L {
        typename L::return_type operator() () {
            return this->set_return(123);
        }
    };
};

struct identity_retcont
{
    template <typename T>
    int operator() (T&&, int x) {
        return x;
    }
};

struct empty_task { };

struct empty_worker {
    template <typename... Results>
    using task = empty_task;
    
    template <typename T>
    using return_t = T;
};

} // unnamed namespace

TEST(Coro, Identity)
{
    namespace fdn = menps::mefdn;
    
    empty_worker wk;
    {
        fdn::slc_frame<identity_coro, identity_retcont, empty_worker> fr(
            fdn::make_tuple(), fdn::make_tuple()
        );
        const auto x = fr(wk);
        ASSERT_EQ(123, x);
    }
    
    {
        fdn::sfc_frame<identity_coro, identity_retcont, empty_worker> fr(
            fdn::make_tuple(), fdn::make_tuple()
        );
        const auto x = fr(wk);
        ASSERT_EQ(123, x);
    }
}

namespace /*unnamed*/ {

template <typename F>
struct call_coro_a : F
{
    using result_type = int;
    
    explicit call_coro_a(int v) : v_(v) { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            f.v_ *= 2;
            return this->template jump<finish>();
        }
    };
    
private:
    template <typename C>
    struct finish : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            return this->template set_return(f.v_);
        }
    };
    
    int v_;
};

template <typename F>
struct call_coro_b : F
{
    using children = typename F::template define_children<call_coro_a>;
    using result_type = int;
    
    explicit call_coro_b(int v) : v_(v) { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            return this->template call<finish, call_coro_a>(f.v_ * 100);
        }
    };
    
private:
    template <typename C>
    struct finish : C {
        typename C::return_type operator() (int y) {
            auto& f = this->frame();
            return this->set_return(f.v_ * 3 + y);
        }
    };
    
    int v_;
};

} // unnamed namespace

TEST(Coro, Call)
{
    namespace fdn = menps::mefdn;
    
    empty_worker wk;
    {
        fdn::slc_frame<call_coro_b, identity_retcont, empty_worker> fr(
            fdn::make_tuple(1), fdn::make_tuple()
        );
        const auto x = fr(wk);
        ASSERT_EQ(203, x);
    }
    
    {
        fdn::sfc_frame<call_coro_b, identity_retcont, empty_worker> fr(
            fdn::make_tuple(1), fdn::make_tuple()
        );
        const auto x = fr(wk);
        ASSERT_EQ(203, x);
    }
}


namespace /*unnamed*/ {

template <typename F>
struct fib_coro : F
{
    using result_type = int;
    
    explicit fib_coro(int n)
        : n_(n) { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            if (f.n_ == 0 || f.n_ == 1) {
                return this->set_return(1);
            }
            else {
                return this->template fork_rec<after_fork, ::fib_coro>(
                    f.t_, f.n_-1);
            }
        }
    };
    
private:
    template <typename C>
    struct after_fork : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            return this->template call_rec<after_call, ::fib_coro>(
                f.n_-2);
        }
    };
    
    template <typename C>
    struct after_call : C {
        typename C::return_type operator() (int r1) {
            auto& f = this->frame();
            f.r1_ = r1;
            return this->template join<finish>(f.t_);
        }
    };
    
    template <typename C>
    struct finish : C {
        typename C::return_type operator() (int r2) {
            auto& f = this->frame();
            return this->set_return(f.r1_ + r2);
        }
    };
    
    typename F::template task<int> t_;
    int n_;
    int r1_;
};

} // unnamed namespace


TEST(Coro, Fib)
{
    namespace fdn = menps::mefdn;
    
    fdn::klt_worker wk;
    
    {
        fdn::sfc_frame<fib_coro, identity_retcont, fdn::klt_worker> fr(
            fdn::make_tuple(10), fdn::make_tuple()
        );
        int r = fr(wk);
        ASSERT_EQ(89, r);
    }
    
    /*{
        fdn::slc_frame<fib_coro, identity_retcont, fdn::klt_worker> fr(
            fdn::make_tuple(10), fdn::make_tuple()
        );
        int r = fr(wk);
        ASSERT_EQ(89, r);
    }*/
}



#if 0

std::function<int ()> g_f;

struct assign_cont {
    template <typename F>
    int operator() (F&& f) {
        g_f = f;
        return -1;
    }
};


template <typename F>
struct coroA : F {
    using result_type = int;
    
    explicit coroA(int x)
        : x_(x)
    { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            return this->template suspend<finish>(assign_cont());
        }
    };
    
private:
    template <typename C>
    struct finish : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            return this->set_return(f.x_ + 100);
        }
    };
    
    int x_;
};

template <typename F>
struct coroB : F {
    using children = typename F::template define_children<coroA>;
    using result_type = int;
    
    explicit coroB(int x)
        : x_(x)
    { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& f = this->frame();
            f.x_ += 10000;
            return this->template call<finish, coroA>(f.x_);
        }
    };

private:
    template <typename C>
    struct finish : C {
        typename C::return_type operator() (int y) {
            auto& f = this->frame();
            return this->set_return(f.x_ + y);
        }
    };
    
    int x_;
};

struct show_x
{
    template <typename T>
    int operator() (T&&, int x) {
        std::cout << x;
        return x;
    }
};




int main_()
{
    //menps::mefdn::seq_call<coroB>( [] (int x) { std::cout << x; }, 3);
    //menps::mefdn::seq_call<coroB>(show_x(), 3);
    //show_x ret_cont;
    menps::mefdn::seq_coro_frame<coroB, show_x> f(menps::mefdn::make_tuple(3), menps::mefdn::make_tuple());
    f();
    
    std::cout << "a";
    g_f();
}

#endif

