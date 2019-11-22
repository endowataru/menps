
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

class dir_sig_buffer
{
public:
    using serialized_buffer_type = fdn::unique_ptr<fdn::byte []>;
    
    serialized_buffer_type serialize(const fdn::size_t /*size*/) const
    {
        return fdn::make_unique<fdn::byte []>(0);
    }
    
    static dir_sig_buffer deserialize_from(const void* /*src*/, const fdn::size_t /*src_size*/)
    {
        return dir_sig_buffer();
    }
};

} // namespace medsm3
} // namespace menps

