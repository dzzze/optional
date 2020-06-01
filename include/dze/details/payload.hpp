#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

#include <dze/type_traits.hpp>

namespace dze::details::optional_ns {

// This policy provides equivalent semantics with std::optional.
struct default_policy {};

template <typename Policy>
constexpr auto is_default_policy_v = std::is_same_v<Policy, default_policy>;

// This class template manages construction/destruction of
// the contained value for a dze::optional.
template <typename T, typename Policy>
class payload_base
{
    using stored_type = std::remove_const_t<T>;

public:
    struct ctor_tag_t {}; // To prevent assignment operators from being implicitly deleted.

    static constexpr ctor_tag_t ctor_tag{};

    payload_base() = default;

    template <typename... Args>
    constexpr payload_base(std::in_place_t, Args&&... args)
        : m_pack{std::in_place, std::forward<Args>(args)...}
    {
        assert(is_engaged());
    }

    template <typename U, typename... Args>
    constexpr payload_base(std::initializer_list<U> ilist, Args&&... args)
        : m_pack{ilist, std::forward<Args>(args)...}
    {
        assert(is_engaged());
    }

    // Constructor used by base copy constructor when the contained value is not trivially
    // copy constructible.
    payload_base(const payload_base& other, ctor_tag_t)
        noexcept(std::is_nothrow_copy_constructible_v<stored_type>)
        : m_pack{other.is_engaged()}
    {
        if (other.is_engaged())
            construct(other.get());
    }

    // Constructor used by base move constructor when the contained value is not trivially
    // move constructible.
    payload_base(payload_base&& other, ctor_tag_t)
        noexcept(std::is_nothrow_move_constructible_v<stored_type>)
        : m_pack{other.is_engaged()}
    {
        if (other.is_engaged())
            construct(std::move(other.get()));
    }

    template <typename... Args>
    void construct(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<stored_type, Args...>)
    {
        ::new
            (static_cast<void*>(std::addressof(m_pack.storage.value)))
            stored_type(std::forward<Args>(args)...);
    }

    template <typename P = Policy,
        DZE_REQUIRES(is_default_policy_v<P>)>
    constexpr void set() noexcept
    {
        m_pack.is_engaged = true;
    }

    void copy_assign(const payload_base& other)
        noexcept(
            std::is_nothrow_copy_constructible_v<stored_type> &&
            std::is_nothrow_copy_assignable_v<stored_type>)
    {
        if (is_engaged() && other.is_engaged())
            get() = other.get();
        else if (other.is_engaged())
        {
            construct(other.get());
            if constexpr (is_default_policy_v<Policy>)
                m_pack.is_engaged = true;
        }
        else
            reset();
    }

    void move_assign(payload_base&& other)
        noexcept(
            std::is_nothrow_move_constructible_v<stored_type> &&
            std::is_nothrow_move_assignable_v<stored_type>)
    {
        if (is_engaged() && other.is_engaged())
            get() = std::move(other.get());
        else if (other.is_engaged())
        {
            construct(std::move(other.get()));
            if constexpr (is_default_policy_v<Policy>)
                m_pack.is_engaged = true;
        }
        else
            reset();
    }

    // While reset puts the managed object to null state, destruct actually destroys it.
    constexpr void destruct() noexcept
    {
        if (is_engaged())
            get().~stored_type();
    }

    [[nodiscard]] constexpr bool is_engaged() const noexcept
    {
        if constexpr (is_default_policy_v<Policy>)
            return m_pack.is_engaged;
        else
            return Policy::is_engaged(m_pack.storage_address());
    }

    constexpr void unchecked_reset() noexcept
    {
        assert(is_engaged());

        get().~stored_type();
        if constexpr (is_default_policy_v<Policy>)
            m_pack.is_engaged = false;
        else
            Policy::null_initialize(m_pack.storage_address());
    }

    constexpr void reset() noexcept
    {
        if (is_engaged())
            unchecked_reset();
    }

    // Apply the erased const qualifier.
    [[nodiscard]] constexpr const T& get() const noexcept
    {
        assert(is_engaged());

        return m_pack.storage.value;
    }

    [[nodiscard]] constexpr T& get() noexcept
    {
        assert(is_engaged());

        return m_pack.storage.value;
    }

private:
    struct empty_byte {};

    template <typename U, bool = std::is_trivially_destructible_v<U>>
    union storage_t
    {
        empty_byte empty;
        stored_type value;

        constexpr storage_t() noexcept
            : empty{} {}

        // Not using brace initializers here to allow potential narrowing conversions.
        template <typename... Args>
        constexpr storage_t(std::in_place_t, Args&&... args)
            : value(std::forward<Args>(args)...) {}

        // Not using brace initializers here to allow potential narrowing conversions.
        template <typename V, typename... Args>
        constexpr storage_t(std::initializer_list<V> ilist, Args&&... args)
            : value(ilist, std::forward<Args>(args)...) {}
    };

    template <typename U>
    union storage_t<U, false>
    {
        empty_byte empty;
        stored_type value;

        constexpr storage_t() noexcept
            : empty{} {}

        // Not using brace initializers here to allow potential narrowing conversions.
        template <typename... Args>
        constexpr storage_t(std::in_place_t, Args&&... args)
            : value(std::forward<Args>(args)...) {}

        // Not using brace initializers here to allow potential narrowing conversions.
        template <typename V, typename... Args>
        constexpr storage_t(std::initializer_list<V> ilist, Args&&... args)
            : value(ilist, std::forward<Args>(args)...) {}

        // User-provided destructor is needed when U has a non-trivial dtor.
        // Clang Tidy does not understand non-default destuctors of unions.
        // NOLINTNEXTLINE(modernize-use-equals-default)
        ~storage_t() {}
    };

    template <typename P, bool = is_default_policy_v<P>>
    struct pack
    {
        storage_t<stored_type> storage;
        bool is_engaged = false;

        pack() = default;

        constexpr pack(const bool engaged) noexcept
            : is_engaged{engaged} {}

        template <typename... Args>
        constexpr pack(std::in_place_t, Args&&... args)
            : storage{std::in_place, std::forward<Args>(args)...}
            , is_engaged{true} {}

        template <typename U, typename... Args>
        constexpr pack(std::initializer_list<U> ilist, Args&&... args)
            : storage{ilist, std::forward<Args>(args)...}
            , is_engaged{true} {}
    };

    template <typename P>
    struct pack<P, false>
    {
        storage_t<stored_type> storage;

        constexpr pack() noexcept
        {
            Policy::null_initialize(storage_address());
        }

        constexpr pack(bool) noexcept {}

        template <typename... Args>
        constexpr pack(std::in_place_t, Args&&... args)
            : storage{std::in_place, std::forward<Args>(args)...} {}

        template <typename U, typename... Args>
        constexpr pack(std::initializer_list<U> ilist, Args&&... args)
            : storage{ilist, std::forward<Args>(args)...} {}

        [[nodiscard]] const std::byte* storage_address() const noexcept
        {
            return std::addressof(reinterpret_cast<const std::byte&>(storage.value));
        }

        [[nodiscard]] std::byte* storage_address() noexcept
        {
            return std::addressof(reinterpret_cast<std::byte&>(storage.value));
        }
    };

    pack<Policy> m_pack;
};

template <
    typename T,
    typename Policy,
    bool = std::is_trivially_destructible_v<T>,
    bool = std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>,
    bool = std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>>
class payload;

// Payload for potentially-constexpr optionals.
template <typename T, typename Policy>
class payload<T, Policy, true, true, true> : public payload_base<T, Policy>
{
public:
    using payload_base<T, Policy>::payload_base;

    payload() = default;
};

// Clang-Tidy generates incorrect warnings for defaulted move constructors/assignment
// operators.

// Payload for optionals with non-trivial copy construction/assignment.
template <typename T, typename Policy>
class payload<T, Policy, true, false, true> : public payload_base<T, Policy>
{
public:
    using payload_base<T, Policy>::payload_base;

    payload() = default;

    payload(const payload&) = default;

    constexpr payload& operator=(const payload& other)
        noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
    {
        this->copy_assign(other);
        return *this;
    }

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    payload(payload&&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    payload& operator=(payload&&) = default;
};

// Payload for optionals with non-trivial move construction/assignment.
template <typename T, typename Policy>
class payload<T, Policy, true, true, false> : public payload_base<T, Policy>
{
public:
    using payload_base<T, Policy>::payload_base;

    payload() = default;

    payload(const payload&) = default;

    payload& operator=(const payload&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    payload(payload&&) = default;

    constexpr payload& operator=(payload&& other)
        // This class assumes the movability of the contained type.
        // Therefore, noexcept specifiers should be applied to the
        // contained type.
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
    {
        this->move_assign(std::move(other));
        return *this;
    }
};

// Payload for optionals with non-trivial copy and move assignment.
template <typename T, typename Policy>
class payload<T, Policy, true, false, false> : public payload_base<T, Policy>
{
public:
    using payload_base<T, Policy>::payload_base;

    payload() = default;

    payload(const payload&) = default;

    constexpr payload& operator=(const payload& other)
        noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
    {
        this->copy_assign(other);
        return *this;
    }

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    payload(payload&&) = default;

    constexpr payload& operator=(payload&& other)
        // This class assumes the movability of the contained type.
        // Therefore, noexcept specifiers should be applied to the
        // contained type.
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
    {
        this->move_assign(std::move(other));
        return *this;
    }
};

// Payload for optionals with non-trivial destructors.
template <typename T, typename Policy, bool Copy, bool Move>
class payload<T, Policy, false, Copy, Move> : public payload<T, Policy, true, Copy, Move>
{
public:
    // Base class implements all the constructors and assignment operators:
    using payload<T, Policy, true, Copy, Move>::payload;

    payload() = default;

    payload(const payload&) = default;

    payload& operator=(const payload&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    payload(payload&&) = default;

    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    payload& operator=(payload&&) = default;

    ~payload() { this->destruct(); }
};

} // namespace dze::details::optional_ns
