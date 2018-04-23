
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/tuple.hpp>
#include <menps/mefdn/type_traits/integer_sequence.hpp>

namespace menps {
namespace mefdn {

template <typename P>
class basic_coro_frame
    : private P::ret_cont_type
    , public P::user_frame_type
{
    MEFDN_DEFINE_DERIVED(P)
    
public:
    using user_frame_type = typename P::user_frame_type;
    using ret_cont_type = typename P::ret_cont_type;
    using worker_type = typename P::worker_type;
    
    using return_type =
        typename worker_type::template return_t<typename user_frame_type::result_type>;
    
    using typename user_frame_type::result_type; // Requirement
    using typename user_frame_type::children; // Optional
    
    template <typename... FAs, typename... RAs>
    explicit basic_coro_frame(mefdn::tuple<FAs...> fas, mefdn::tuple<RAs...> ras)
        : basic_coro_frame(
            // TODO: Are these moves necessary?
            mefdn::move(fas), mefdn::index_sequence_for<FAs...>(),
            mefdn::move(ras), mefdn::index_sequence_for<RAs...>()
        )
    { }
    
private:
    // helper constructor
    template <typename... FAs, mefdn::size_t... FNs,
              typename... RAs, mefdn::size_t... RNs>
    explicit basic_coro_frame(
        mefdn::tuple<FAs...> fas MEFDN_MAYBE_UNUSED
    ,   mefdn::index_sequence<FNs...> /*ignored*/
    ,   mefdn::tuple<RAs...> ras MEFDN_MAYBE_UNUSED
    ,   mefdn::index_sequence<RNs...> /*ignored*/
    )
        // The order is reversed
        : ret_cont_type(mefdn::get<RNs>(ras)...)
        , user_frame_type(mefdn::get<FNs>(fas)...)
    { }
    
public:
    template <typename... Args>
    return_type set_return(Args&&... args) {
        // Implicit cast to the base class.
        ret_cont_type& rc = *this;
        return rc(mefdn::forward<Args>(args)...);
    }
    
    template <typename... Args>
    return_type operator() (worker_type& wk, Args&&... args) {
        auto& self = this->derived();
        // Create the "start" label.
        typename P::start_label_type lb(self, wk);
        return lb( mefdn::forward<Args>(args)... );
    }
};

} // namespace mefdn
} // namespace menps

