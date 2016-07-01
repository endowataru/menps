
#include "rma_window.hpp"
#include "common/command/delegate.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include <mgbase/logger.hpp>

namespace mgcom {
namespace mpi3 {

rma_window::rma_window(/*delegator& del*/)
    //: del_(del)
{
    /*struct closure
    {
        rma_window& self;
        
        MGBASE_WARN_UNUSED_RESULT
        bool operator() () const
        {*/
            auto& self = *this;
            
            mpi3_error::check(
                MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, &self.win_)
            );
            
            int* model;
            int flag;
            
            MPI_Win_get_attr(self.win_, MPI_WIN_MODEL, &model, &flag);
            
            // Cast the type of "model".
            const int model_i = static_cast<int>(reinterpret_cast<mgbase::intptr_t>(model));
            
            MGBASE_LOG_DEBUG(
                "msg:Called MPI_Win_create_dynamic.\t"
                "model:{}"
            ,   (flag ? (model_i == MPI_WIN_UNIFIED ? "MPI_WIN_UNIFIED" : "MPI_WIN_SEPARATE") : "no model")
            );
            
            mpi3_error::check( 
                MPI_Win_lock_all(0, self.win_) // TODO : Assertion
            );
            
            /*return true;
        }
    };
    
    execute(del_, closure{ *this });*/
}

rma_window::~rma_window()
{
    /*struct closure
    {
        rma_window& self;
        
        MGBASE_WARN_UNUSED_RESULT
        bool operator() () const
        {*/
            auto& self = *this;
            
            mpi3_error::check(
                MPI_Win_unlock_all(self.win_)
            );
            
            mpi3_error::check(
                MPI_Win_free(&self.win_)
            );
            
            /*return true;
        }
    };
    
    execute(del_, closure{ *this });*/
}

} // namespace mpi3
} // namespace mgcom

