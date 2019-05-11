
#include <mefdn/optional.hpp>
#include "unittest.hpp"

TEST(Optional, Basic)
{
    mefdn::optional<int> o = mefdn::make_optional(123);
    ASSERT_TRUE(o);
    
    o = mefdn::nullopt;
    ASSERT_FALSE(o);
}

