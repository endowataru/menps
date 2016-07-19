
#pragma once

#include <mgcom/rma/requester.hpp>

namespace mgcom {
namespace fjmpi {

inline mgbase::uint64_t get_absolute_address(const rma::untyped::local_address& addr) MGBASE_NOEXCEPT
{
    const mgbase::uint64_t laddr = addr.region.info;
    return laddr + addr.offset;
}

inline mgbase::uint64_t get_absolute_address(const rma::untyped::remote_address& addr) MGBASE_NOEXCEPT
{
    const mgbase::uint64_t raddr = addr.region.info;
    return raddr + addr.offset;
}

// Note: Alignments are important to copy the parameters fast

struct MGBASE_ALIGNAS(8) get_params
{
    int                 proc;
    mgbase::uint64_t    laddr;
    mgbase::uint64_t    raddr;
    std::size_t         size_in_bytes;
    int                 flags;
    mgbase::operation   on_complete;
    
    #ifndef MGCOM_FJMPI_DISABLE_COPY
    get_params& operator = (const get_params& params)
    {
        proc          = params.proc;
        laddr         = params.laddr;
        raddr         = params.raddr;
        size_in_bytes = params.size_in_bytes;
        flags         = params.flags;
        on_complete   = params.on_complete;
        
        return *this;
    }
    #endif
};

struct MGBASE_ALIGNAS(8) put_params
{
    int                 proc;
    mgbase::uint64_t    laddr;
    mgbase::uint64_t    raddr;
    std::size_t         size_in_bytes;
    int                 flags;
    mgbase::operation   on_complete;
    
    #ifndef MGCOM_FJMPI_DISABLE_COPY
    put_params& operator = (const put_params& params)
    {
        proc          = params.proc;
        laddr         = params.laddr;
        raddr         = params.raddr;
        size_in_bytes = params.size_in_bytes;
        flags         = params.flags;
        on_complete   = params.on_complete;
        
        return *this;
    }
    #endif
};

struct reg_mem_params
{
    void*           buf;
    std::size_t     length;
};

struct reg_mem_result
{
    int                 memid;
    mgbase::uint64_t    laddr;
};

struct get_remote_addr_params
{
    int pid;
    int memid;
};

struct get_remote_addr_result
{
    mgbase::uint64_t    raddr;
};

struct dereg_mem_params
{
    int memid;
};

class fjmpi_interface
{
protected:
    explicit fjmpi_interface(endpoint& ep) MGBASE_NOEXCEPT
        : ep_(ep) { }

public:
    virtual ~fjmpi_interface() MGBASE_EMPTY_DEFINITION
    
    endpoint& get_endpoint() MGBASE_NOEXCEPT { return ep_; }
    
    fjmpi_interface(const fjmpi_interface&) = delete;
    fjmpi_interface& operator = (const fjmpi_interface&) = delete;
    
    virtual bool try_get_async(const get_params&) = 0;
    
    virtual bool try_put_async(const put_params&) = 0;
    
    virtual reg_mem_result reg_mem(const reg_mem_params&) = 0;
    
    virtual get_remote_addr_result get_remote_addr(const get_remote_addr_params&) = 0;
    
    virtual void dereg_mem(const dereg_mem_params&) = 0;
    
private:
    endpoint& ep_;
};

} // namespace fjmpi
} // namespace mgcom

