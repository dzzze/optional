#pragma once

#include <array>
#include <cstddef>
#include <cstring>

#include <dze/optional.hpp>
#include <dze/sentinel.hpp>

namespace dze::test {

template <size_t Bytes>
class ff_policy
{
public:
    [[nodiscard]] static bool is_engaged(const std::byte* storage) noexcept
    {
        return std::memcmp(storage, &sentinel, Bytes) != 0;
    }

    static void null_initialize(std::byte* const storage) noexcept
    {
        std::memcpy(storage, &sentinel, Bytes);
    }

private:
    static auto init_sentinel()
    {
        std::array<std::byte, Bytes> sentinel;
        sentinel.fill(std::byte{0xFF});
        return sentinel;
    }

    inline static const std::array<std::byte, Bytes> sentinel = init_sentinel();
};

template <typename T>
using ff_sentinel = ::dze::optional<T, ff_policy<sizeof(T)>>;

} // namespace dze::test
