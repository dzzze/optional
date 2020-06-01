#include "optional.hpp"
#include "optional_reference.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <catch2/catch.hpp>

TEMPLATE_TEST_CASE("Assignment", "[assignment.val]", dze::optional<int>, (dze::sentinel<int, -1>))
{
    TestType o1 = 42;

    SECTION("Assign value")
    {
        o1 = 84;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Assign value of different type")
    {
        o1 = short{84};
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Self assign")
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#endif
        o1 = o1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        REQUIRE(o1.has_value());
        CHECK(*o1 == 42);
    }

    SECTION("Assign another optional")
    {
        TestType o2 = 84;
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional")
    {
        TestType o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Assign nullopt")
    {
        o1 = std::nullopt;
        REQUIRE(!o1.has_value());
    }

    SECTION("Assign another optional of different type")
    {
        dze::optional<short> o2 = short{84};
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional of different type")
    {
        dze::optional<short> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }
}

TEMPLATE_TEST_CASE(
    "Assignment",
    "[assignment.val]",
    dze::optional<std::string>,
    dze::test::ff_sentinel<std::string>)
{
    TestType o1 = std::string{"42"};

    SECTION("Assign value")
    {
        o1 = std::string{"84"};
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
    }

    SECTION("Assign value of different type")
    {
        o1 = "84";
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
    }

    SECTION("Self assign")
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#endif
        o1 = o1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        REQUIRE(o1.has_value());
        CHECK(*o1 == "42");
    }

    SECTION("Assign another optional")
    {
        TestType o2 = std::string{"84"};
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
        REQUIRE(o2.has_value());
        CHECK(*o2 == "84");
    }

    SECTION("Move assign another optional")
    {
        TestType o2 = std::string{"84"};
        o1 = std::move(o2);
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
        REQUIRE(o2.has_value());
        CHECK(o2->empty());
    }

    SECTION("Assign empty optional")
    {
        TestType o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Move assign empty optional")
    {
        TestType o2;
        o1 = std::move(o2);
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Assign nullopt")
    {
        o1 = std::nullopt;
        REQUIRE(!o1.has_value());
    }

    SECTION("Assign another optional of different type")
    {
        dze::optional<const char*> o2 = "84";
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == "84");
        REQUIRE(o2.has_value());
        CHECK(*o2 == std::string_view{"84"});
    }

    SECTION("Assign empty optional of different type")
    {
        dze::optional<const char*> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }
}

TEST_CASE("Assignment", "[assignment.ref]")
{
    constexpr int i = 42;
    constexpr int j = 84;
    constexpr base b{42};
    constexpr derived d{84};

    SECTION("Assign value")
    {
        dze::optional_reference o1 = i;
        o1 = j;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Assign value of different type")
    {
        dze::optional_reference o1 = b;
        o1 = d;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
    }

    SECTION("Self assign")
    {
        dze::optional_reference o1 = i;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#endif
        o1 = o1;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        REQUIRE(o1.has_value());
        CHECK(*o1 == 42);
    }

    SECTION("Assign another optional")
    {
        dze::optional_reference o1 = i;
        dze::optional_reference o2 = j;
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional")
    {
        dze::optional_reference o1 = i;
        dze::optional_reference<const int> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }

    SECTION("Assign nullopt")
    {
        dze::optional_reference o1 = i;
        o1 = std::nullopt;
        REQUIRE(!o1.has_value());
    }

    SECTION("Assign another optional of different type")
    {
        dze::optional_reference o1 = b;
        dze::optional_reference o2 = d;
        o1 = o2;
        REQUIRE(o1.has_value());
        CHECK(*o1 == 84);
        REQUIRE(o2.has_value());
        CHECK(*o2 == 84);
    }

    SECTION("Assign empty optional of different type")
    {
        dze::optional_reference o1 = b;
        dze::optional_reference<const derived> o2;
        o1 = o2;
        CHECK(!o1.has_value());
        CHECK(!o2.has_value());
    }
}
