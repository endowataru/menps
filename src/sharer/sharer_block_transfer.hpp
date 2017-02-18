
#pragma once

#include <mgcom/rma.hpp>

namespace mgdsm {

class sharer_block_transfer
{
    typedef mgcom::rma::paired_remote_ptr<void>     prptr_type;
    typedef mgcom::rma::remote_ptr<void>            rptr_type;
    typedef mgcom::rma::local_ptr<void>             lptr_type;
    
    typedef mgcom::rma::remote_ptr<mgbase::uint8_t> rptr_byte_type;
    typedef mgcom::rma::local_ptr<mgbase::uint8_t>  lptr_byte_type;
    
public:
    static void read(
        const prptr_type&       src_prptr
    ,   const lptr_type&        twin_lptr
    ,   void* const             dest_ptr
    ,   const mgbase::size_t    block_size
    )
    {
        // Copy from the source to the twin.
        mgcom::rma::read(
            src_prptr.proc
        ,   mgbase::reinterpret_pointer_cast<mgbase::uint8_t>(
                src_prptr.ptr
            )
        ,   mgbase::reinterpret_pointer_cast<mgbase::uint8_t>(
                twin_lptr
            )
        ,   block_size
        );
        
        const void* const twin_ptr = twin_lptr;
        
        // Copy from the twin to the destination.
        std::memcpy(dest_ptr, twin_ptr, block_size);
    }
    
    static void write_whole(
        void* const             src_ptr
    ,   const lptr_type&        twin_lptr
    ,   const prptr_type&       dest_prptr
    ,   const mgbase::size_t    block_size
    )
    {
        void* const twin_ptr = twin_lptr;
        
        // Copy from the source to the twin.
        std::memcpy(twin_ptr, src_ptr, block_size);
        
        // Copy from the twin to the destination.
        mgcom::rma::write(
            dest_prptr.proc
        ,   mgbase::reinterpret_pointer_cast<mgbase::uint8_t>(
                dest_prptr.ptr
            )
        ,   mgbase::reinterpret_pointer_cast<mgbase::uint8_t>(
                twin_lptr
            )
        ,   block_size
        );
    }
    
    static void write_diffs(
        void* const             src_ptr
    ,   const lptr_type&        twin_lptr
    ,   const prptr_type&       dest_prptr
    ,   const mgbase::size_t    block_size
    ) {
        copy_diffs(src_ptr, twin_lptr, dest_prptr, block_size, rma_write{});
    }
    
private:
    struct rma_write
    {
        void operator() (
            const mgcom::process_id_t   dest_proc
        ,   const rptr_byte_type&       dest_rptr
        ,   const lptr_byte_type&       src_lptr
        ,   const mgbase::size_t        size
        ) {
            mgcom::rma::write(dest_proc, dest_rptr, src_lptr, size);
        }
    };
    
    template <typename WriteFunc>
    static void copy_diffs(
        void* const             src_ptr
    ,   const lptr_type&        twin_lptr
    ,   const prptr_type&       dest_prptr
    ,   const mgbase::size_t    block_size
    ,   WriteFunc               write_func
    )
    {
        const auto data_ptr =
            static_cast<mgbase::uint8_t*>(src_ptr);
        
        void* const twin_ptr_void = twin_lptr;
        
        const auto twin_ptr =
            static_cast<mgbase::uint8_t*>(twin_ptr_void);
        
        const auto dest_rptr =
            mgbase::reinterpret_pointer_cast<mgbase::uint8_t>(
                dest_prptr.ptr
            );
        
        const auto twin_lptr_byte =
            mgbase::reinterpret_pointer_cast<mgbase::uint8_t>(
                twin_lptr
            );
        
        mgbase::size_t from = 0;
        mgbase::size_t index = 0;
        
        while (index < block_size)
        {
            // Scan clean bytes.
            
            while (data_ptr[index] == twin_ptr[index])
            {
                ++index;
                
                if (!(index < block_size)) {
                    return;
                }
            }
            
            // Scan dirty bytes.
            
            from = index;
            
            do {
                ++index;
                
                if (!(index < block_size)) {
                    break;
                }
            }
            while (data_ptr[index] != twin_ptr[index]);
            
            const auto len = index - from;
            
            // Copy the data to the twin.
            std::memcpy(twin_ptr + from, data_ptr + from, len);
            
            // Copy the twin to the destination.
            mgcom::rma::write(
                dest_prptr.proc
            ,   dest_rptr + static_cast<mgbase::ptrdiff_t>(from)
            ,   twin_lptr_byte + from
            ,   len
            );
        }
    }
};

} // namespace mgdsm

