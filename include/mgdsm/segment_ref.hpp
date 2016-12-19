
#pragma once

#include <mgdsm/segment.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class segment_ref
{
public:
    explicit segment_ref(segment* const seg)
        : seg_()
    { }
    
    /*virtual void fetch(mgbase::size_t, mgbase::size_t) = 0;
    
    virtual void touch(mgbase::size_t, mgbase::size_t) = 0;
    
    virtual void reconcile_all() = ();
    
    virtual void flush_all() = ();*/
    
private:
    mgbase::unique_ptr<segment> seg_;
};

} // namespace mgdsm

