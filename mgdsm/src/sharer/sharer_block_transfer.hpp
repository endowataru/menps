
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rma/to_raw.hpp>
#include <mgcom/rpc.hpp>

namespace mgdsm {

template <typename Policy>
class sharer_block_transfer
{
    typedef mgcom::rma::paired_remote_ptr<void>     prptr_type;
    typedef mgcom::rma::remote_ptr<void>            rptr_type;
    typedef mgcom::rma::local_ptr<void>             lptr_type;
    
    typedef mgcom::rma::paired_remote_ptr<mgbase::uint8_t> prptr_byte_type;
    typedef mgcom::rma::remote_ptr<mgbase::uint8_t> rptr_byte_type;
    typedef mgcom::rma::local_ptr<mgbase::uint8_t>  lptr_byte_type;
    
    typedef typename Policy::process_id_type        process_id_type;
    
    typedef typename Policy::handler_id_type        handler_id_type;
    
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
    
    static void register_handlers()
    {
        Policy::register_handler(unpack_diff_handler{});
    }
    
    static void write_diffs(
        void* const             src_ptr
    ,   const lptr_type&        twin_lptr
    ,   const prptr_type&       dest_prptr
    ,   const mgbase::size_t    block_size
    ) {
        #if 1
        const auto max_diff_pack_size =
            sizeof(typename unpack_diff_handler::request_type) +
            (sizeof(diff_index_t)*2 + diff_align) / 2 * block_size;
        
        // TODO: very ugly. needs better interface
        auto msg =
            client_request_message_type::convert_from(
                Policy::allocate_request(8 /*TODO*/, max_diff_pack_size)
            );
        
        mgbase::size_t dest_offset = 0;
        mgbase::size_t pos = 0;
        
        copy_diffs(src_ptr, twin_lptr, dest_prptr, block_size,
            pack_diff{ dest_prptr, msg, block_size, &dest_offset, &pos });
        
        auto& header = *msg;
        header.dest_ptr = mgcom::rma::to_raw_pointer(dest_prptr.ptr);
        header.pos = pos;
        header.block_size = block_size;
        
        Policy::template call<unpack_diff_handler>(
            dest_prptr.proc
        ,   mgbase::move(msg)
        );
        #else
        copy_diffs(src_ptr, twin_lptr, dest_prptr, block_size, rma_write{});
        #endif
    }
    
private:
    // RPC-based implementation
    
    typedef mgbase::uint32_t    diff_index_t;
    static const mgbase::size_t diff_align = 8; // TODO
    
    static void unpack_diff(
        void* const                 dest_void_ptr
    ,   const void*                 msg_void_ptr
    ,   const mgbase::size_t        size
    ,   const mgbase::size_t        block_size
    ) {
        auto dest_ptr =
            static_cast<mgbase::uint8_t*>(dest_void_ptr);
        
        const auto msg_ptr =
            static_cast<const mgbase::uint8_t*>(msg_void_ptr);
        
        mgbase::size_t i = 0;
        while (i < size)
        {
            const auto skipped = *reinterpret_cast<const diff_index_t*>(msg_ptr + i);
            i += sizeof(diff_index_t);
            
            const auto diff_size = *reinterpret_cast<const diff_index_t*>(msg_ptr + i);
            i += sizeof(diff_index_t);
            
            MGBASE_ASSERT(skipped < block_size);
            MGBASE_ASSERT(diff_size < block_size);
            
            dest_ptr += skipped;
            
            std::memcpy(dest_ptr, msg_ptr + i, diff_size);
            
            i += diff_size;
            dest_ptr += diff_size;
            
            while (i % diff_align != 0) {
                ++i;
            }
        }
        
        MGBASE_ASSERT(i == size);
    }
    
    struct unpack_diff_handler
    {
        static const handler_id_type handler_id = Policy::write_diff_handler_id;
        
        struct request_type {
            // Header
            void*           dest_ptr;
            mgbase::size_t  pos;
            mgbase::size_t  block_size;
        };
        typedef void    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            unpack_diff(
                rqst.dest_ptr
            ,   &rqst + 1
            ,   rqst.pos
            ,   rqst.block_size
            );
            
            return sc.make_reply();
        }
    };
    
    typedef typename Policy::template client_request_message_type<typename unpack_diff_handler::request_type>::type
        client_request_message_type;
    
    struct pack_diff
    {
        prptr_type                      dest_prptr;
        client_request_message_type&    rqst;
        mgbase::size_t                  block_size;
        mgbase::size_t*                 dest_offset;
        mgbase::size_t*                 buf_pos;
        
        void operator() (
            const process_id_type       dest_proc
        ,   const rptr_byte_type&       dest_rptr
        ,   const lptr_byte_type&       src_lptr
        ,   const mgbase::size_t        size
        ) {
            MGBASE_ASSERT(dest_prptr.proc == dest_proc);
            
            MGBASE_ASSERT(
                sizeof(typename unpack_diff_handler::request_type)
                    + *buf_pos + sizeof(diff_index_t)*2 + size
                <= rqst.size_in_bytes()
            );
            
            const auto orig_msg_ptr =
                reinterpret_cast<mgbase::uint8_t*>(
                    // Skip the header.
                    rqst.get() + 1
                );
            
            auto msg_ptr = orig_msg_ptr;
            
            // Skip the area that is already written.
            msg_ptr += *buf_pos;
            
            // TODO : provide better interface
            const auto dest_pos =
                mgcom::rma::to_raw_pointer(dest_rptr) -
                static_cast<mgbase::uint8_t*>(mgcom::rma::to_raw_pointer(dest_prptr.ptr));
            
            const auto skipped = dest_pos - *dest_offset;
            MGBASE_ASSERT(skipped < block_size);
            
            *reinterpret_cast<diff_index_t*>(msg_ptr)
                = static_cast<diff_index_t>(skipped);
            
            msg_ptr += sizeof(diff_index_t);
            
            *reinterpret_cast<diff_index_t*>(msg_ptr)
                = static_cast<diff_index_t>(size);
            
            msg_ptr += sizeof(diff_index_t);
            
            std::memcpy(msg_ptr, src_lptr, size);
            
            msg_ptr += size;
            
            while (reinterpret_cast<mgbase::uintptr_t>(msg_ptr) % diff_align != 0) {
                *msg_ptr = 0;
                ++msg_ptr;
            }
            
            MGBASE_UNUSED const auto orig_pos = *buf_pos;
            
            *dest_offset += skipped + size;
            *buf_pos = msg_ptr - orig_msg_ptr;
            
            MGBASE_ASSERT(*buf_pos ==
                diff_align *
                    mgbase::roundup_divide(
                        orig_pos + sizeof(diff_index_t)*2 + size
                    ,   diff_align
                    )
            );
        }
    };
    
    // RMA-based implementation
    
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
            write_func(
                dest_prptr.proc
            ,   dest_rptr + static_cast<mgbase::ptrdiff_t>(from)
            ,   twin_lptr_byte + from
            ,   len
            );
        }
    }
};

} // namespace mgdsm

