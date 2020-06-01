#pragma once

#include <cstddef>
#include <cstring>

#include "optional.hpp"

namespace dze {

namespace details::optional_ns {

template <typename T, auto sentinel_value>
class sentinel_value_policy
{
public:
    [[nodiscard]] static bool is_engaged(const std::byte* const storage) noexcept
    {
        return std::memcmp(storage, &sentinel, sizeof(T)) != 0;
    }

    static void null_initialize(std::byte* const storage) noexcept
    {
        std::memcpy(storage, &sentinel, sizeof(T));
    }

private:
    inline static const T sentinel{sentinel_value};
};

} // details::optional_ns

template <typename T, auto sentinel_value>
using sentinel = optional<T, details::optional_ns::sentinel_value_policy<T, sentinel_value>>;

} // namespace dze
