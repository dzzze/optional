#pragma once

#include <initializer_list>
#include <type_traits>
#include <utility>

#include <dze/type_traits.hpp>

#include "bad_optional_access.hpp"
#include "details/payload.hpp"
#include "nullopt.hpp"

namespace dze {

namespace details::optional_ns {

// Common base class for base<T, Policy> to avoid repeating these
// member functions in each specialization.
template <typename T, typename Policy, typename Base>
class base_impl
{
    using stored_type = std::remove_const_t<T>;

protected:
    // construct has !is_engaged() as a precondition.
    template <typename... Args>
    void construct(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<stored_type, Args...>)
    {
        auto& payload = static_cast<Base*>(this)->get_payload();
        payload.construct(std::forward<Args>(args)...);
        if constexpr (is_default_policy_v<Policy>)
            payload.set();
    }

    // destruct has is_engaged() as a precondition.
    void destruct() noexcept { static_cast<Base*>(this)->get_payload().destruct(); }

    [[nodiscard]] constexpr bool is_engaged() const noexcept
    {
        return static_cast<const Base*>(this)->get_payload().is_engaged();
    }

    // unchecked_reset has is_engaged() as a precondition.
    constexpr void unchecked_reset() noexcept
    {
        static_cast<Base*>(this)->get_payload().unchecked_reset();
    }

    // reset is a 'safe' operation with no precondition.
    constexpr void reset_impl() noexcept
    {
        static_cast<Base*>(this)->get_payload().reset();
    }

    // The get() operations have is_engaged as a precondition.
    [[nodiscard]] constexpr const T& get() const noexcept
    {
        return static_cast<const Base*>(this)->get_payload().get();
    }

    [[nodiscard]] constexpr T& get() noexcept
    {
        return static_cast<Base*>(this)->get_payload().get();
    }
};

/*
Class template that takes care of copy/move constructors of optional.

Such a separate base class template is necessary in order to
conditionally make copy/move constructors trivial.

When the contained value is trivally copy/move constructible,
the copy/move constructors of base will invoke the
trivial copy/move constructor of payload. Otherwise,
they will invoke payload(bool, const payload&)
or payload(bool, payload&&) to initialize
the contained value, if copying/moving an engaged optional.

Whether the other special members are trivial is determined by the
payload<T> specialization used for the m_payload member.
*/
template <
    typename T,
    typename Policy,
    bool = std::is_trivially_copy_constructible_v<T>,
    bool = std::is_trivially_move_constructible_v<T>>
class base : public base_impl<T, Policy, base<T, Policy>>
{
public:
    // Constructor for disengaged optionals.
    base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr base(
        std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    constexpr base(const base& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        : m_payload{other.m_payload, decltype(m_payload)::ctor_tag} {}

    base& operator=(const base&) = default;

    constexpr base(base&& other)
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_payload{std::move(other.m_payload), decltype(m_payload)::ctor_tag} {}

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    base& operator=(base&&) = default;

    [[nodiscard]] constexpr auto& get_payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& get_payload() noexcept { return m_payload; }

private:
    payload<T, Policy> m_payload;
};

template <typename T, typename Policy>
class base<T, Policy, false, true> : public base_impl<T, Policy, base<T, Policy>>
{
public:
    // Constructor for disengaged optionals.
    base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr base(
        std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    constexpr base(const base& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        : m_payload{other.m_payload, decltype(m_payload)::ctor_tag} {}

    base& operator=(const base&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    constexpr base(base&&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    base& operator=(base&&) = default;

    [[nodiscard]] constexpr auto& get_payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& get_payload() noexcept { return m_payload; }

private:
    payload<T, Policy> m_payload;
};

template <typename T, typename Policy>
class base<T, Policy, true, false> : public base_impl<T, Policy, base<T, Policy>>
{
public:
    // Constructor for disengaged optionals.
    base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr base(std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    constexpr base(const base&) = default;

    base& operator=(const base&) = default;

    constexpr base(base&& other)
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_payload{std::move(other.m_payload), decltype(m_payload)::ctor_tag} {}

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    base& operator=(base&&) = default;

    [[nodiscard]] constexpr auto& get_payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& get_payload() noexcept { return m_payload; }

private:
    payload<T, Policy> m_payload;
};

template <typename T, typename Policy>
class base<T, Policy, true, true> : public base_impl<T, Policy, base<T, Policy>>
{
public:
    // Constructors for disengaged optionals.
    base() = default;

    // Constructors for engaged optionals.
    template <typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr base(std::in_place_t, Args&&... args)
        : m_payload{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr base(std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : m_payload{std::in_place, ilist, std::forward<Args>(args)...} {}

    [[nodiscard]] constexpr auto& get_payload() const noexcept { return m_payload; }

    [[nodiscard]] constexpr auto& get_payload() noexcept { return m_payload; }

private:
    payload<T, Policy> m_payload;
};

template <typename T>
using enable_copy_move = enable_copy_move<
    std::is_copy_constructible_v<T>,
    std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>,
    std::is_move_constructible_v<T>,
    std::is_move_constructible_v<T> && std::is_move_assignable_v<T>,
    T>;

} // namespace details::optional_ns

template <typename T, typename Policy>
class optional;

namespace details::optional_ns {

template <typename T, typename U, typename Policy>
constexpr bool convertible_from_optional =
    std::is_constructible_v<T, const optional<U, Policy>&> ||
    std::is_constructible_v<T, optional<U, Policy>&> ||
    std::is_constructible_v<T, const optional<U, Policy>&&> ||
    std::is_constructible_v<T, optional<U, Policy>&&> ||
    std::is_convertible_v<const optional<U, Policy>&, T> ||
    std::is_convertible_v<optional<U, Policy>&, T> ||
    std::is_convertible_v<const optional<U, Policy>&&, T> ||
    std::is_convertible_v<optional<U, Policy>&&, T>;

template <typename T, typename U, typename Policy>
constexpr bool assignable_from_optional =
    std::is_assignable_v<T&, const optional<U, Policy>&> ||
    std::is_assignable_v<T&, optional<U, Policy>&> ||
    std::is_assignable_v<T&, const optional<U, Policy>&&> ||
    std::is_assignable_v<T&, optional<U, Policy>&&>;

} // namespace details::optional_ns

template <typename T, typename Policy = details::optional_ns::default_policy>
class optional
    : private details::optional_ns::base<T, Policy>
    , private details::optional_ns::enable_copy_move<T>
{
    static_assert(!std::is_same_v<std::remove_cv_t<T>, nullopt_t>);
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>);
    static_assert(!std::is_reference_v<T>);
    static_assert(std::is_trivially_copy_constructible_v<Policy>);
    static_assert(std::is_trivially_move_constructible_v<Policy>);
    static_assert(std::is_trivially_copy_assignable_v<Policy>);
    static_assert(std::is_trivially_move_assignable_v<Policy>);
    static_assert(std::is_trivially_destructible_v<Policy>);

    using base = details::optional_ns::base<T, Policy>;

public:
    using value_type = T;

    constexpr optional() = default;

    constexpr optional(nullopt_t) noexcept {}

    // Converting constructors for engaged optionals.
    template <typename U = T,
        DZE_REQUIRES(
            !std::is_same_v<optional<T, Policy>, std::decay_t<U>> &&
            !std::is_same_v<std::in_place_t, std::decay_t<U>> &&
            std::is_constructible_v<T, U&&> &&
            std::is_convertible_v<U&&, T>)>
    constexpr optional(U&& t)
        : base{std::in_place, std::forward<U>(t)} {}

    template <typename U = T,
        DZE_REQUIRES(
            !std::is_same_v<optional<T, Policy>, std::decay_t<U>> &&
            !std::is_same_v<std::in_place_t, std::decay_t<U>> &&
            std::is_constructible_v<T, U&&> &&
            !std::is_convertible_v<U&&, T>)>
    explicit constexpr optional(U&& t)
        : base{std::in_place, std::forward<U>(t)} {}

    template <typename U, typename Policy2,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, const U&> &&
            std::is_convertible_v<const U&, T> &&
            !details::optional_ns::convertible_from_optional<T, U, Policy2>)>
    constexpr optional(const optional<U, Policy2>& other)
    {
        if (other)
            emplace(*other);
    }

    template <typename U, typename Policy2,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, const U&> &&
            !std::is_convertible_v<const U&, T> &&
            !details::optional_ns::convertible_from_optional<T, U, Policy2>)>
    explicit constexpr optional(const optional<U, Policy2>& other)
    {
        if (other)
            emplace(*other);
    }

    template <typename U, typename Policy2,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, U&&> &&
            std::is_convertible_v<U&&, T> &&
            !details::optional_ns::convertible_from_optional<T, U, Policy2>)>
    constexpr optional(optional<U, Policy2>&& other)
    {
        if (other)
            emplace(std::move(*other));
    }

    template <typename U, typename Policy2,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, U&&> &&
            !std::is_convertible_v<U&&, T> &&
            !details::optional_ns::convertible_from_optional<T, U, Policy2>)>
    explicit constexpr optional(optional<U, Policy2>&& other)
    {
        if (other)
            emplace(std::move(*other));
    }

    template <typename... Args,
         DZE_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    explicit constexpr optional(std::in_place_t, Args&&... args)
        : base{std::in_place, std::forward<Args>(args)...} {}

    template <typename U, typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>&, Args&&...>)>
    explicit constexpr optional(
        std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        : base{std::in_place, ilist, std::forward<Args>(args)...} {}

    optional& operator=(nullopt_t) noexcept
    {
        this->reset();
        return *this;
    }

    template <typename U = T,
        DZE_REQUIRES(
            !std::is_same_v<optional<T, Policy>, std::decay_t<U>> &&
            std::is_constructible_v<T, U> &&
            !(std::is_scalar_v<T> &&
            std::is_same_v<T, std::decay_t<U>>) &&
            std::is_assignable_v<T&, U>)>
    optional& operator=(U&& value)
    {
        if (this->is_engaged())
            this->get() = std::forward<U>(value);
        else
            this->construct(std::forward<U>(value));

        return *this;
    }

    template <
        typename U,
        typename Policy2,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, const U&> &&
            std::is_assignable_v<T&, const U&> &&
            !details::optional_ns::convertible_from_optional<T, U, Policy2> &&
            !details::optional_ns::assignable_from_optional<T, U, Policy2>)>
    optional& operator=(const optional<U, Policy2>& other)
    {
        if (other)
        {
            if (this->is_engaged())
                this->get() = *other;
            else
                this->construct(*other);
        }
        else
            this->reset();

        return *this;
    }

    template <typename U, typename Policy2,
        DZE_REQUIRES(
            !std::is_same_v<T, U> &&
            std::is_constructible_v<T, U&&> &&
            std::is_assignable_v<T&, U&&> &&
            !details::optional_ns::convertible_from_optional<T, U, Policy2> &&
            !details::optional_ns::assignable_from_optional<T, U, Policy2>)>
    optional& operator=(optional<U, Policy2>&& other)
    {
        if (other)
        {
            if (this->is_engaged())
                this->get() = std::move(*other);
            else
                this->construct(std::move(*other));
        }
        else
            this->reset();

        return *this;
    }

    template <typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, Args&&...>)>
    T& emplace(Args&&... args)
    {
        this->destruct();
        this->construct(std::forward<Args>(args)...);
        return this->get();
    }

    template <typename U, typename... Args,
        DZE_REQUIRES(std::is_constructible_v<T, std::initializer_list<U>, Args&&...>)>
    T& emplace(std::initializer_list<U> ilist, Args&&... args)
    {
        this->destruct();
        this->construct(ilist, std::forward<Args>(args)...);
        return this->get();
    }

    // Destructor is implicit, implemented in optional_ns::base.

    void swap(optional& other)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>)
    {
        using std::swap;

        if (this->is_engaged() && other.is_engaged())
            swap(this->get() && other.get());
        else if (this->is_engaged())
        {
            other.construct(std::move(this->get()));
            this->unchecked_reset();
        }
        else if (other.is_engaged())
        {
            this->construct(std::move(other.get()));
            other.unchecked_reset();
        }
    }

    constexpr const T* operator->() const { return std::addressof(this->get()); }

    constexpr T* operator->() { return std::addressof(this->get()); }

    constexpr const T& operator*() const& { return this->get(); }

    constexpr T& operator*() & { return this->get(); }

    constexpr const T&& operator*() const&& { return std::move(this->get()); }

    constexpr T&& operator*() && { return std::move(this->get()); }

    explicit constexpr operator bool() const noexcept { return this->is_engaged(); }

    [[nodiscard]] constexpr bool has_value() const noexcept { return this->is_engaged(); }

    [[nodiscard]] constexpr T& value() &
    {
        if (!this->is_engaged())
            throw bad_optional_access{};

        return this->get();
    }

    [[nodiscard]] constexpr const T& value() const&
    {
        if (!this->is_engaged())
            throw bad_optional_access{};

        return this->get();
    }

    [[nodiscard]] constexpr T&& value() &&
    {
        if (!this->is_engaged())
            throw bad_optional_access{};

        return std::move(this->get());
    }

    [[nodiscard]] constexpr const T&& value() const&&
    {
        if (!this->is_engaged())
            throw bad_optional_access{};

        return std::move(this->get());
    }

    template <typename U>
    [[nodiscard]] constexpr T value_or(U&& default_value) const&
    {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        return this->is_engaged()
            ? this->get()
            : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
    [[nodiscard]] constexpr T value_or(U&& default_value) &&
    {
        static_assert(std::is_move_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        return this->is_engaged()
            ? std::move(this->get())
            : static_cast<T>(std::forward<U>(default_value));
    }

    void reset() noexcept { this->reset_impl(); }
};

template <typename T>
optional(T) -> optional<T>;

template <typename T, typename Policy1, typename U, typename Policy2,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) == static_cast<bool>(rhs) && (!lhs || *lhs == *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return
        static_cast<bool>(lhs) != static_cast<bool>(rhs) ||
        (static_cast<bool>(lhs) && *lhs != *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs > *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(rhs) && (!lhs || *lhs <= *rhs);
}

template <typename T, typename Policy1, typename U, typename Policy2,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(
    const optional<T, Policy1>& lhs, const optional<U, Policy2>& rhs)
{
    return static_cast<bool>(lhs) && (!lhs || *lhs >= *rhs);
}

// Comparisons with nullopt.
template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator==(const optional<T, Policy>& lhs, nullopt_t) noexcept
{
    return !lhs;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator==(nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return !rhs;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator!=(const optional<T, Policy>& lhs, nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator!=(nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<(const optional<T, Policy>&, nullopt_t) noexcept
{
    return false;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<(nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return static_cast<bool>(rhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>(const optional<T, Policy>& lhs, nullopt_t) noexcept
{
    return static_cast<bool>(lhs);
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>(nullopt_t, const optional<T, Policy>&) noexcept
{
    return false;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<=(const optional<T, Policy>& lhs, nullopt_t) noexcept
{
    return !lhs;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator<=(nullopt_t, const optional<T, Policy>&) noexcept
{
    return true;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>=(const optional<T, Policy>&, nullopt_t) noexcept
{
    return true;
}

template <typename T, typename Policy>
[[nodiscard]] constexpr bool operator>=(nullopt_t, const optional<T, Policy>& rhs) noexcept
{
    return !rhs;
}

// Comparisons with value type.

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const optional<T, Policy>& lhs, const U& rhs)
{
    return lhs && *lhs == rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() == std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator==(const U& lhs, const optional<T, Policy>& rhs)
{
    return rhs && lhs == *rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const optional<T, Policy>& lhs, const U& rhs)
{
    return !lhs || *lhs != rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() != std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator!=(const U& lhs, const optional<T, Policy>& rhs)
{
    return !rhs || lhs != *rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const optional<T, Policy>& lhs, const U& rhs)
{
    return !lhs || *lhs < rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() < std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<(const U& lhs, const optional<T, Policy>& rhs)
{
    return rhs && lhs < *rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const optional<T, Policy>& lhs, const U& rhs)
{
    return lhs && *lhs > rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() > std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>(const U& lhs, const optional<T, Policy>& rhs)
{
    return !rhs || lhs > *rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const optional<T, Policy>& lhs, const U& rhs)
{
    return !lhs || *lhs <= rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() <= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator<=(const U& lhs, const optional<T, Policy>& rhs)
{
    return rhs && lhs <= *rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const optional<T, Policy>& lhs, const U& rhs)
{
    return lhs && *lhs >= rhs;
}

template <typename T, typename Policy, typename U,
    DZE_REQUIRES(std::is_convertible_v<decltype(std::declval<T>() >= std::declval<U>()), bool>)>
[[nodiscard]] constexpr bool operator>=(const U& lhs, const optional<T, Policy>& rhs)
{
    return !rhs || lhs >= *rhs;
}

template <typename T, typename Policy,
    DZE_REQUIRES(std::is_move_constructible_v<T> && std::is_swappable_v<T>)>
void swap(optional<T, Policy>& lhs, optional<T, Policy>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

template <typename T, typename Policy,
    DZE_REQUIRES(!(std::is_move_constructible_v<T> && std::is_swappable_v<T>))>
void swap(optional<T, Policy>&, optional<T, Policy>&) = delete;

template <typename T>
[[nodiscard]] constexpr optional<std::decay_t<T>> make_optional(T&& value)
{
    return optional<std::decay_t<T>>{std::forward<T>(value)};
}

template <typename Policy, typename T>
[[nodiscard]] constexpr optional<std::decay_t<T>, Policy> make_optional(T&& value)
{
    return optional<std::decay_t<T>, Policy>{std::forward<T>(value)};
}

template <typename T, typename... Args>
[[nodiscard]] constexpr optional<T> make_optional(Args&&... args)
{
    return optional<T>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename Policy, typename... Args>
[[nodiscard]] constexpr optional<T, Policy> make_optional(Args&&... args)
{
    return optional<T, Policy>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename U, typename... Args>
[[nodiscard]] constexpr optional<T>
    make_optional(std::initializer_list<U> ilist, Args&&... args)
{
    return optional<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

template <typename T, typename Policy, typename U, typename... Args>
[[nodiscard]] constexpr optional<T, Policy>
    make_optional(std::initializer_list<U> ilist, Args&&... args)
{
    return optional<T, Policy>{std::in_place, ilist, std::forward<Args>(args)...};
}

template <typename T>
struct hash;

template <typename T, typename Policy>
struct hash<optional<T, Policy>>
{
    size_t operator()(const optional<T, Policy>& opt) const
    {
        using std::hash;

        constexpr auto magic_disengaged_hash = static_cast<size_t>(-3333);
        return opt ? hash<std::remove_const_t<T>>{}(*opt) : magic_disengaged_hash;
    }
};

} // namespace dze
