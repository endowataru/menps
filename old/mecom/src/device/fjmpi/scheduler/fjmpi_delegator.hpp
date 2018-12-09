
#pragma once

#include "device/fjmpi/fjmpi_interface.hpp"
#include "command_producer.hpp"
#include "memid_pool.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class fjmpi_delegator
    : public virtual fjmpi_interface
{
public:
    fjmpi_delegator(endpoint&, command_producer&);
    
    fjmpi_delegator(const fjmpi_delegator&) = delete;
    fjmpi_delegator& operator = (const fjmpi_delegator&) = delete;
    
    MEFDN_NODISCARD
    virtual bool try_get_async(const get_params&) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual bool try_put_async(const put_params&) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual reg_mem_result reg_mem(const reg_mem_params&) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual get_remote_addr_result get_remote_addr(const get_remote_addr_params&) MEFDN_OVERRIDE;
    
    virtual void dereg_mem(const dereg_mem_params&) MEFDN_OVERRIDE;
    
private:
    command_producer&   cp_;
    memid_pool          pool_;
};


} // namespace fjmpi
} // namespace mecom
} // namespace menps

