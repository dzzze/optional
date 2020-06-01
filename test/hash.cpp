#include "optional.hpp"

#include <utility>
#include <vector>

#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE(
    "Hash", "[hash.val]", dze::optional<int>, (dze::sentinel<int, -1>))
{
    for (auto i : {0, 1, std::numeric_limits<int>::max()})
        CHECK(dze::hash<TestType>{}(i) == std::hash<int>{}(i));
}

TEMPLATE_TEST_CASE(
    "Hash", "[hash.val]", dze::optional<std::string>, (dze::test::ff_sentinel<std::string>))
{
    for (auto str : {"a", "abcd", "abcdefgh"})
        CHECK(dze::hash<TestType>{}(str) == std::hash<std::string>{}(str));
}
