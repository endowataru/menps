
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/mpi/direct_mpi_facade.hpp>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_mpi_request_holder
{
    using orig_mpi_facade_type = typename P::orig_mpi_facade_type;
    using proxy_request_type = typename P::proxy_request_type;
    using size_type = typename P::size_type;
    using ult_itf_type = typename P::ult_itf_type;
    using suspended_thread_type = typename ult_itf_type::suspended_thread;
    
public:
    proxy_mpi_request_holder()
        : num_reqs_(0)
        , proxy_reqs_(mefdn::make_unique<proxy_request_type* []>(P::max_num_ongoing))
        , orig_reqs_(mefdn::make_unique<MPI_Request []>(P::max_num_ongoing))
        , indices_(mefdn::make_unique<int []>(P::max_num_ongoing))
        , statuses_(mefdn::make_unique<MPI_Status []>(P::max_num_ongoing))
    { }
    
    proxy_mpi_request_holder(const proxy_mpi_request_holder&) = delete;
    proxy_mpi_request_holder& operator = (const proxy_mpi_request_holder&) = delete;
    
    void add(
        proxy_request_type* const   proxy_req
    ,   const MPI_Request           orig_req
    ) {
        const auto num_reqs = this->num_reqs_;
        MEFDN_ASSERT(num_reqs < P::max_num_ongoing);
        
        this->proxy_reqs_[num_reqs] = proxy_req;
        this->orig_reqs_[num_reqs] = orig_req;
        
        this->num_reqs_ = num_reqs+1;
        
        MEFDN_LOG_VERBOSE(
            "msg:Added new request in proxy MPI.\t"
            "num_reqs:{}"
        ,   this->num_reqs_
        );
    }
    
    struct progress_result {
        size_type               num_ongoing;
        size_type               num_completed;
        suspended_thread_type   awake_sth;
    };
    
    template <typename CompFunc>
    progress_result progress(orig_mpi_facade_type& orig_mf, CompFunc comp_func)
    {
        const auto num_reqs = this->num_reqs_;
        
        if (num_reqs == 0) {
            MEFDN_LOG_VERBOSE(
                "msg:No request for progress in proxy MPI"
            );
            return { 0, 0, suspended_thread_type() };
        }
        
        const auto incount = static_cast<int>(num_reqs);
        int outcount = 0;
        
        // Call the original test function.
        orig_mf.testsome({ incount, &this->orig_reqs_[0],
            &outcount, &this->indices_[0], &this->statuses_[0] });
        
        if (MEFDN_UNLIKELY(outcount < 0)) {
            throw medev2::mpi::mpi_error();
        }
        
        const auto num_completed = static_cast<size_type>(outcount);
        MEFDN_ASSERT(num_completed <= num_reqs);
        
        if (num_completed == 0) {
            MEFDN_LOG_VERBOSE(
                "msg:Found no completed request in proxy MPI.\t"
                "num_reqs:{}"
            ,   num_reqs
            );
            return { num_reqs, 0, suspended_thread_type() };
        }
        
        suspended_thread_type awake_sth;
        
        for (size_type i = 0; i < num_completed; ++i) {
            const auto index = static_cast<size_type>(this->indices_[i]);
            MEFDN_LOG_VERBOSE("msg:Found completion.\tindex:{}", index);
            //MEFDN_ASSERT(0 <= index); // TODO
            MEFDN_ASSERT(index < num_reqs);
            
            auto& proxy_req = this->proxy_reqs_[index];
            const bool waiting = comp_func(proxy_req, this->statuses_[index]);
            
            if (waiting) {
                if (awake_sth) {
                    awake_sth.notify();
                }
                awake_sth = mefdn::move(proxy_req->sth);
            }
        }
        
        size_type num_ongoing = 0;
        for (size_type i = 0; i < num_reqs; ++i) {
            if (this->orig_reqs_[i] != MPI_REQUEST_NULL) {
                if (i != num_ongoing) {
                    this->proxy_reqs_[num_ongoing] = this->proxy_reqs_[i];
                    this->orig_reqs_[num_ongoing]  = this->orig_reqs_[i];
                }
                ++num_ongoing;
            }
        }
        
        MEFDN_ASSERT(num_ongoing + num_completed == num_reqs);
        
        // Remove completed requests from the arrays.
        this->num_reqs_ = num_ongoing;
        
        MEFDN_LOG_VERBOSE(
            "msg:Completed requests in proxy MPI.\t"
            "num_reqs:{}\t"
            "num_completed:{}"
        ,   num_reqs
        ,   num_completed
        );
        
        return { num_reqs, num_completed, mefdn::move(awake_sth) };
    }
    
    size_type get_num_ongoing() const noexcept
    {
        return this->num_reqs_;
    }
    
    bool is_available() const noexcept {
        return this->get_num_ongoing() < P::max_num_ongoing;
    }
    
private:
    size_type                                   num_reqs_ = 0;
    mefdn::unique_ptr<proxy_request_type* []>   proxy_reqs_;
    mefdn::unique_ptr<MPI_Request []>           orig_reqs_;
    mefdn::unique_ptr<int []>                   indices_;
    mefdn::unique_ptr<MPI_Status []>            statuses_;
};

} // namespace meqdc
} // namespace menps

