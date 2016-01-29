
#include <mgbase/optional.hpp>
#include "unittest.hpp"

TEST(Optional, Basic)
{
    mgbase::optional<int> o = mgbase::make_optional(123);
    bool b = o;
    ASSERT_TRUE(b);
    
    o = mgbase::nullopt;
    ASSERT_FALSE(b);
}

