
#pragma once

#include "am_base.ipp"

namespace mgcom {
namespace am {

namespace /*unnamed*/ {

class impl
    : public am_base
{
public:
    static void try_send(
        process_id_t    proc
    ,   const message&  msg
    ) {
        mgcom::rma::local_pointer<const am_message>    src_ptr;
        mgcom::rma::remote_pointer<am_message>         dest_buf;
        
        if (am_base::try_send(msg, &src_ptr, &dest_ptr))
        {
            mgcom::rma::try_remote_write_extra(
                proc
            ,   dest_ptr.to_address()
            ,   src_ptr.to_address()
            ,   
            );
        }
    }
    
    
};

} // unnamed namespace

} // namespace am
} // namespace mgcom

