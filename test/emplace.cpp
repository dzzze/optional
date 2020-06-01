#include "optional.hpp"
#include "optional_reference.hpp"

#include <tuple>
#include <utility>

#include <catch2/catch.hpp>

TEST_CASE("Emplace value", "[emplace.val]")
{
    dze::optional<std::pair<std::pair<int, int>, std::pair<double, double>>> i;
    i.emplace(std::piecewise_construct, std::make_tuple(0, 1), std::make_tuple(2, 3));
    REQUIRE(i.has_value());
    CHECK(i->first.first == 0);
    CHECK(i->first.second == 1);
    CHECK(i->second.first == 2);
    CHECK(i->second.second == 3);
}

TEST_CASE("Emplace reference", "[emplace.ref]")
{
    SECTION("Same type")
    {
        constexpr int i = 42;
        constexpr int j = 84;

        dze::optional_reference o1 = i;
        o1.emplace(j);
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Different type")
    {
        constexpr base b{42};
        constexpr derived d{84};

        dze::optional_reference o1 = b;
        o1.emplace(d);
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }
}
