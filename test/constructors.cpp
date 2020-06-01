#include "optional.hpp"
#include "optional_reference.hpp"

#include <utility>
#include <vector>

#include <catch2/catch.hpp>

constexpr int i = 42;
constexpr base b{42};
constexpr derived d{84};

TEMPLATE_TEST_CASE(
    "Constructors", "[constructors.val]", dze::optional<int>, (dze::sentinel<int, -1>))
{
    SECTION("Empty construct")
    {
        TestType o1;
        TestType o2{};
        TestType o3 = {};
        TestType o4 = std::nullopt;
        TestType o5 = {std::nullopt};
        TestType o6(std::nullopt);

        CHECK(!o1);
        CHECK(!o2);
        CHECK(!o3);
        CHECK(!o4);
        CHECK(!o5);
        CHECK(!o6);
    }

    SECTION("Value construct")
    {
        TestType o1 = 42;
        TestType o2{42};
        TestType o3(42);
        TestType o4 = {42};

        TestType o5 = i;
        TestType o6{i};
        TestType o7(i);
        TestType o8 = {i};

        CHECK(*o1 == 42);
        CHECK(*o2 == 42);
        CHECK(*o3 == 42);
        CHECK(*o4 == 42);
        CHECK(*o5 == 42);
        CHECK(*o6 == 42);
        CHECK(*o7 == 42);
        CHECK(*o8 == 42);
    }

    SECTION("Construct from another")
    {
        TestType o1 = 42;
        TestType o2 = o1;
        TestType o3{o1};
        TestType o4(o1);
        TestType o5 = {o1};

        CHECK(*o1 == 42);
        CHECK(*o2 == 42);
        CHECK(*o3 == 42);
        CHECK(*o4 == 42);
        CHECK(*o5 == 42);
    }

    SECTION("Construct from another optional type")
    {
        dze::optional<short> o1 = i;
        TestType o2 = o1;
        TestType o3{o1};
        TestType o4(o1);
        TestType o5 = {o1};

        CHECK(*o1 == 42);
        CHECK(*o2 == 42);
        CHECK(*o3 == 42);
        CHECK(*o4 == 42);
        CHECK(*o5 == 42);
    }
}

struct foo
{
    foo() = default;
    foo(const foo&) = delete;
    foo(foo&&) noexcept {}; // NOLINT(modernize-use-equals-default)
};

TEMPLATE_TEST_CASE(
    "Constructors",
    "[constructors.val]",
    dze::optional<std::vector<foo>>,
    dze::test::ff_sentinel<std::vector<foo>>)
{
    std::vector<foo> v;
    v.emplace_back();
    TestType ov1 = std::move(v);
    REQUIRE(v.empty());
    REQUIRE(ov1.has_value());
    REQUIRE(ov1->size() == 1);

    TestType ov2 = std::move(ov1);
    REQUIRE(ov2.has_value());
    REQUIRE(ov2->size() == 1);
    REQUIRE(ov1.has_value());
    CHECK(ov1->empty());
}

TEST_CASE("Constructors", "[constructors.ref]")
{
    SECTION("Empty construct")
    {
        constexpr dze::optional_reference<int>
            o1,
            o2{},
            o3 = {},
            o4 = std::nullopt,
            o5 = {std::nullopt},
            o6{std::nullopt},
            o7(std::nullopt);

        STATIC_REQUIRE(!o1);
        STATIC_REQUIRE(!o2);
        STATIC_REQUIRE(!o3);
        STATIC_REQUIRE(!o4);
        STATIC_REQUIRE(!o5);
        STATIC_REQUIRE(!o6);
        STATIC_REQUIRE(!o7);
    }

    SECTION("Value construct")
    {

        constexpr dze::optional_reference
            o1 = i,
            o2{i},
            o3(i),
            o4 = {i};

        STATIC_REQUIRE(*o1 == 42);
        STATIC_REQUIRE(*o2 == 42);
        STATIC_REQUIRE(*o3 == 42);
        STATIC_REQUIRE(*o4 == 42);
    }

    SECTION("Construct from another")
    {
        constexpr dze::optional_reference
            o1 = i,
            o2 = o1,
            o3{o1},
            o4(o1),
            o5 = {o1};

        STATIC_REQUIRE(*o1 == 42);
        STATIC_REQUIRE(*o2 == 42);
        STATIC_REQUIRE(*o3 == 42);
        STATIC_REQUIRE(*o4 == 42);
        STATIC_REQUIRE(*o5 == 42);
    }

    SECTION("Implicit construct from another optional type")
    {
        constexpr dze::optional_reference o1 = d;
        constexpr dze::optional_reference<const base>
            o2 = o1,
            o3{o1},
            o4(o1),
            o5 = {o1};

        STATIC_REQUIRE(*o1 == 84);
        STATIC_REQUIRE(*o2 == 84);
        STATIC_REQUIRE(*o3 == 84);
        STATIC_REQUIRE(*o4 == 84);
        STATIC_REQUIRE(*o5 == 84);
    }

    SECTION("Explicit construct from another optional type")
    {
        constexpr dze::optional_reference o1 = b;
        // Downcasting is not constexpr.
        const dze::optional_reference<const derived>
            o2{o1},
            o3(o1);

        REQUIRE(*o1 == 42);
        REQUIRE(*o2 == 42);
        REQUIRE(*o3 == 42);
    }
}
