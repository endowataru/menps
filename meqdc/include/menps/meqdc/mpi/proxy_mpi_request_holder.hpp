
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/mpi/mpi.hpp>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_mpi_request_holder
{
    using orig_mpi_facade_type = typename P::orig_mpi_facade_type;
    using proxy_request_type = typename P::proxy_request_type;
    using size_type = typename P::size_type;
    
public:
    proxy_mpi_request_holder() = default;
    
    proxy_mpi_request_holder(const proxy_mpi_request_holder&) = delete;
    proxy_mpi_request_holder& operator = (const proxy_mpi_request_holder&) = delete;
    
    void add(
        proxy_request_type* const   proxy_req
    ,   const MPI_Request           orig_req
    ) {
        this->proxy_reqs_.push_back(proxy_req);
        this->orig_reqs_.push_back(orig_req);
        this->indices_.push_back(0);
        this->statuses_.push_back(MPI_Status()); // TODO
        
        MEFDN_LOG_VERBOSE(
            "msg:Added new request in proxy MPI.\t"
            "num_reqs:{}"
        ,   this->proxy_reqs_.size()
        );
    }
    
    template <typename ProgFunc>
    size_type progress(orig_mpi_facade_type& orig_mf, ProgFunc prog_func)
    {
        const auto num_reqs = this->proxy_reqs_.size();
        
        if (num_reqs == 0) {
            MEFDN_LOG_VERBOSE(
                "msg:No request for progress in proxy MPI"
            );
            return 0;
        }
        
        const auto incount = static_cast<int>(num_reqs);
        int outcount = 0;
        
        // Call the original test function.
        orig_mf.testsome({ incount, &this->orig_reqs_[0],
            &outcount, &this->indices_[0], &this->statuses_[0] });
        
        const auto num_completed = static_cast<size_type>(outcount);
        
        if (num_completed == 0) {
            MEFDN_LOG_VERBOSE(
                "msg:Found no completed request in proxy MPI.\t"
                "num_reqs:{}"
            ,   num_reqs
            );
            return 0;
        }
        
        for (size_type i = 0; i < num_completed; ++i) {
            const auto index = static_cast<size_type>(this->indices_[i]);
            MEFDN_LOG_VERBOSE("msg:Found completion.\tindex:{}", index);
            MEFDN_ASSERT(0 <= index && index < num_reqs);
            
            prog_func(this->proxy_reqs_[index], this->statuses_[index]);
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
        
        #if 0
        auto num_ongoing = num_reqs;
        
        for (size_type i = 0; i < num_completed; ++i) {
            const auto index = static_cast<size_type>(this->indices_[i]);
            
            // Move the last request element to the completed position.
            this->proxy_reqs_[index] = this->proxy_reqs_[num_ongoing - 1];
            this->orig_reqs_[index]  = this->orig_reqs_[num_ongoing - 1];
            
            MEFDN_ASSERT(num_ongoing > 0);
            --num_ongoing;
        }
        #endif
        
        // Remove completed requests from the arrays.
        this->proxy_reqs_.resize(num_ongoing);
        this->orig_reqs_.resize(num_ongoing);
        this->indices_.resize(num_ongoing);
        this->statuses_.resize(num_ongoing);
        
        MEFDN_LOG_VERBOSE(
            "msg:Completed requests in proxy MPI.\t"
            "num_reqs:{}\t"
            "num_completed:{}"
        ,   num_reqs
        ,   num_completed
        );
        
        return num_completed;
    }
    
    size_type get_num_ongoing() const noexcept
    {
        return this->proxy_reqs_.size();
    }
    
private:
    std::vector<proxy_request_type*>    proxy_reqs_;
    std::vector<MPI_Request>            orig_reqs_;
    std::vector<int>                    indices_;
    std::vector<MPI_Status>             statuses_;
};

} // namespace meqdc
} // namespace menps

