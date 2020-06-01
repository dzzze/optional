#pragma once

#include <dze/optional_reference.hpp>

class derived;

class base
{
public:
    explicit constexpr base(const int val) noexcept
        : m_val{val} {}

    constexpr operator int() const noexcept { return m_val; }

    explicit constexpr operator const derived&() const noexcept;

    explicit constexpr operator derived&() noexcept;

private:
    int m_val;
};

class derived : public base
{
    using base::base;
};

constexpr base::operator const derived&() const noexcept
{
    return *static_cast<const derived*>(this);
}

constexpr base::operator derived&() noexcept { return *static_cast<derived*>(this); }
