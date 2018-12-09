
#include "rma.hpp"

namespace menps {
namespace mecom {

namespace rma {
namespace /*unnamed*/ {

using namespace fjmpi;

class fjmpi_registrator
    : public registrator
{
public:
    explicit fjmpi_registrator(fjmpi_interface& fi)
        : fi_(fi) { }
    
    virtual ~fjmpi_registrator() = default;
    
    virtual untyped::local_region register_region(const untyped::register_region_params& params) MEFDN_OVERRIDE
    {
        const auto res = fi_.reg_mem({
            params.local_ptr
        ,   params.size_in_bytes
        });
        
        return untyped::make_local_region(
            untyped::make_region_key(
                params.local_ptr
            ,   static_cast<mefdn::uint64_t>(res.memid)
            )
        ,   res.laddr
        );
    }
    
    virtual untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) MEFDN_OVERRIDE
    {
        const auto res
            = fi_.get_remote_addr({
                static_cast<int>(params.proc_id)
            ,   static_cast<int>(params.key.info)
            });
        
        return untyped::make_remote_region(params.key, res.raddr);
    }
    
    virtual void deregister_region(const untyped::deregister_region_params& params) MEFDN_OVERRIDE
    {
        fi_.dereg_mem({
            static_cast<int>(params.region.key.info)
        });
    }
    
private:
    fjmpi_interface& fi_;
};

} // unnamed namespace
} // namespace rma

namespace fjmpi {

mefdn::unique_ptr<rma::registrator> make_rma_registrator(fjmpi_interface& fi)
{
    return mefdn::make_unique<rma::fjmpi_registrator>(fi);
}

} // namespace fjmpi

} // namespace mecom
} // namespace menps

