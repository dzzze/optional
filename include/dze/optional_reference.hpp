#pragma once

#include <optional>
#include <type_traits>
#include <utility>

#include <dze/requires.hpp>

#include "bad_optional_access.hpp"
#include "nullopt.hpp"

namespace dze {

template <typename T>
class optional_reference;

namespace details {

template <typename T, typename U>
constexpr bool convertible_from_optional_reference =
    std::is_constructible_v<T&, const optional_reference<U>&> ||
    std::is_constructible_v<T&, optional_reference<U>&> ||
    std::is_constructible_v<T&, const optional_reference<U>&&> ||
    std::is_constructible_v<T&, optional_reference<U>&&> ||
    std::is_convertible_v<const optional_reference<U>&, T&> ||
    std::is_convertible_v<optional_reference<U>&, T&> ||
    std::is_convertible_v<const optional_reference<U>&&, T&> ||
    std::is_convertible_v<optional_reference<U>&&, T&>;

} // namespace details

// Has similar semantics to std::reference_wrapper ie. assignment rebinds the underlying reference.
template <typename T>
class optional_reference
{
public:
    using value_type = T&;

    optional_reference() = default;

    constexpr optional_reference(nullopt_t) noexcept {}

    template <typename U = T,
        DZE_REQUIRES(std::is_constructible_v<T&, U&>)>
    constexpr optional_reference(U& u) noexcept
        : m_ref{std::addressof(static_cast<T&>(u))} {}

    template <
        typename U,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T&, U&> &&
            std::is_convertible_v<U&, T&> &&
            !details::convertible_from_optional_reference<T, U>)>
    constexpr optional_reference(const optional_reference<U>& other) noexcept
    {
        if (other)
            *this = *other;
    }

    template <typename U,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T&, U&> &&
            !std::is_convertible_v<U&, T&> &&
            !details::convertible_from_optional_reference<T, U>)>
    explicit constexpr optional_reference(const optional_reference<U>& other) noexcept
    {
        if (other)
            *this = *other;
    }

    constexpr optional_reference& operator=(nullopt_t) noexcept
    {
        m_ref = nullptr;
        return *this;
    }

    template <typename U = T,
        DZE_REQUIRES(std::is_constructible_v<T&, U&> && std::is_convertible_v<U&, T&>)>
    constexpr optional_reference& operator=(U& u) noexcept
    {
        m_ref = std::addressof(static_cast<T&>(u));
        return *this;
    }

    template <typename U,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T&, U&> &&
            std::is_convertible_v<U&, T&> &&
            !details::convertible_from_optional_reference<T, U>)>
    constexpr optional_reference& operator=(const optional_reference<U>& other) noexcept
    {
        if (other)
            return *this = *other;
        else
            return *this = nullopt;
    }

    template <typename U = T>
    constexpr optional_reference& emplace(U& u) noexcept
    {
        return *this = u;
    }

    constexpr void reset() noexcept { m_ref = nullptr; }

    constexpr void swap(optional_reference& other) noexcept
    {
        const auto backup = m_ref;
        m_ref = other.m_ref;
        other.mptr = backup;
    }

    constexpr T* operator->() const noexcept { return m_ref; }

    constexpr T& operator*() const noexcept { return *m_ref; }

    [[nodiscard]] constexpr bool has_value() const noexcept { return m_ref != nullptr; }

    explicit constexpr operator bool() const noexcept { return m_ref != nullptr; }

    [[nodiscard]] constexpr T& value() const
    {
        if (!has_value())
            throw bad_optional_access{};

        return *m_ref;
    }

    template <typename U>
    [[nodiscard]] constexpr T value_or(U&& u) const
    {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        return has_value() ? **this : static_cast<T>(std::forward<U>(u));
    }

private:
    T* m_ref = nullptr;
};

template <typename T>
optional_reference(T&) -> optional_reference<T>;

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) == static_cast<bool>(rhs) && (!lhs || *lhs == *rhs);
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) != static_cast<bool>(rhs) ||
        (static_cast<bool>(lhs) && *lhs != *rhs);
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
constexpr bool operator>(const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs > *rhs);
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs <= *rhs);
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(
    const optional_reference<T>& lhs, const optional_reference<U>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs >= *rhs);
}

// Comparisons with nullopt.

template <typename T>
[[nodiscard]] constexpr bool operator==(const optional_reference<T>& lhs, nullopt_t) noexcept
{
    return !lhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return !rhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(const optional_reference<T>& lhs, nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator<(const optional_reference<T>&, nullopt_t) noexcept
{
    return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator<(nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator>(const optional_reference<T>& lhs, nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator>(nullopt_t, const optional_reference<T>&) noexcept
{
    return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator<=(const optional_reference<T>& lhs, nullopt_t) noexcept
{
    return !lhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator<=(nullopt_t, const optional_reference<T>&) noexcept
{
    return true;
}

template <typename T>
[[nodiscard]] constexpr bool operator>=(const optional_reference<T>&, nullopt_t) noexcept
{
    return true;
}

template <typename T>
[[nodiscard]] constexpr bool operator>=(nullopt_t, const optional_reference<T>& rhs) noexcept
{
    return !rhs;
}

// Comparisons with value type.

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const optional_reference<T>& lhs, const U& rhs)
{
    return lhs && *lhs == rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const U& lhs, const optional_reference<T>& rhs)
{
    return rhs && lhs == *rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const optional_reference<T>& lhs, const U& rhs)
{
    return !lhs || *lhs != rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const U& lhs, const optional_reference<T>& rhs)
{
    return !rhs || lhs != *rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const optional_reference<T>& lhs, const U& rhs)
{
    return !lhs || *lhs < rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const U& lhs, const optional_reference<T>& rhs)
{
    return rhs && lhs < *rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const optional_reference<T>& lhs, const U& rhs)
{
    return lhs && *lhs > rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const U& lhs, const optional_reference<T>& rhs)
{
    return !rhs || lhs > *rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const optional_reference<T>& lhs, const U& rhs)
{
    return !lhs || *lhs <= rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const U& lhs, const optional_reference<T>& rhs)
{
    return rhs && lhs <= *rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const optional_reference<T>& lhs, const U& rhs)
{
    return lhs && *lhs >= rhs;
}

template <typename T, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const U& lhs, const optional_reference<T>& rhs)
{
    return !rhs || lhs >= *rhs;
}

template <typename T>
constexpr void swap(optional_reference<T>& lhs, optional_reference<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace dze
