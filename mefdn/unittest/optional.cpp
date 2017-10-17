
#include <mgbase/optional.hpp>
#include "unittest.hpp"

TEST(Optional, Basic)
{
    mgbase::optional<int> o = mgbase::make_optional(123);
    ASSERT_TRUE(o);
    
    o = mgbase::nullopt;
    ASSERT_FALSE(o);
}

