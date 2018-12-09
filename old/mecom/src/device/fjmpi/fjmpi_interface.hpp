
#pragma once

#include <menps/mecom/rma/requester.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {

inline mefdn::uint64_t get_absolute_address(const rma::untyped::local_address& addr) noexcept
{
    const mefdn::uint64_t laddr = addr.region.info;
    return laddr + addr.offset;
}

inline mefdn::uint64_t get_absolute_address(const rma::untyped::remote_address& addr) noexcept
{
    const mefdn::uint64_t raddr = addr.region.info;
    return raddr + addr.offset;
}

// Note: Alignments are important to copy the parameters fast

struct MEFDN_ALIGNAS(8) get_params
{
    int                 proc;
    mefdn::uint64_t    laddr;
    mefdn::uint64_t    raddr;
    std::size_t         size_in_bytes;
    int                 flags;
    mefdn::callback<void ()> on_complete;
    
    #ifndef MECOM_FJMPI_DISABLE_COPY
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

struct MEFDN_ALIGNAS(8) put_params
{
    int                 proc;
    mefdn::uint64_t    laddr;
    mefdn::uint64_t    raddr;
    std::size_t         size_in_bytes;
    int                 flags;
    mefdn::callback<void ()> on_complete;
    
    #ifndef MECOM_FJMPI_DISABLE_COPY
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
    mefdn::uint64_t    laddr;
};

struct get_remote_addr_params
{
    int pid;
    int memid;
};

struct get_remote_addr_result
{
    mefdn::uint64_t    raddr;
};

struct dereg_mem_params
{
    int memid;
};

class fjmpi_interface
{
protected:
    explicit fjmpi_interface(endpoint& ep) noexcept
        : ep_(ep) { }

public:
    virtual ~fjmpi_interface() = default;
    
    endpoint& get_endpoint() noexcept { return ep_; }
    
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
} // namespace mecom
} // namespace menps

