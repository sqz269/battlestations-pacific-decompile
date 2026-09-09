#pragma once
#include <cstdint>

namespace bsp {
// Native string projection: length gates equality; non-null data must be a
// readable NUL-terminated string. Bytes after NUL are not compared/searched.
struct TextureLoadNameView {
    std::uint32_t length{};
    const char* data{};
};

struct TextureLoadImage {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mip_levels{};
};

struct TextureLoadPolicy {
    std::uint32_t requested_width{};
    std::uint32_t requested_height{};
    std::uint32_t requested_mip_levels{};
    std::uint32_t saved_width{};
    std::uint32_t saved_height{};
};

// Fragment of 00b2c2d0 starting at 00b2c405; not the disk-reload policy.
// Caller supplies a stable setting for both native +1D84h reads.
// False only for nonzero name length with null data when name checks execute;
// output is unchanged on that explicit malformed-input boundary.
bool select_initial_texture_load_policy_00b2c405(TextureLoadNameView name,
    const TextureLoadImage& image, std::uint32_t stable_setting,
    TextureLoadPolicy& output) noexcept;
}
