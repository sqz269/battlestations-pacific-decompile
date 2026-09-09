#include "bsp/texture_load_policy.hpp"
#include <cstring>

namespace bsp {
namespace {
std::uint32_t clamp_signed_greater_than_one(std::uint32_t bits) noexcept {
    // Native CMP/JG tests signed DWORDs after SHR or wrapping SUB.
    return bits > 1 && bits < 0x80000000u ? bits : 1;
}
}

bool select_initial_texture_load_policy_00b2c405(TextureLoadNameView name,
    const TextureLoadImage& image, std::uint32_t stable_setting,
    TextureLoadPolicy& output) noexcept {
    TextureLoadPolicy result{0xffffffffu, 0xffffffffu, image.mip_levels,
        image.width, image.height};
    if (image.mip_levels > 1 && stable_setting != 0) {
        // 00449af0's right string is nonempty "detail.dds". Its length-zero
        // left branch returns unequal without calling _stricmp.
        if (name.length != 0 && !name.data) return false;
        const bool is_detail = name.length != 0 &&
            _stricmp(name.data, "detail.dds") == 0;
        const bool exempt = is_detail || (name.data &&
            (std::strstr(name.data, "noseart") != nullptr ||
             std::strstr(name.data, "interface/textures/gui/units") != nullptr));
        if (!exempt) {
            const std::uint32_t shift = stable_setting & 31u;
            result.requested_width = clamp_signed_greater_than_one(image.width >> shift);
            result.requested_height = clamp_signed_greater_than_one(image.height >> shift);
            result.requested_mip_levels = clamp_signed_greater_than_one(
                image.mip_levels - stable_setting);
            result.saved_width = result.requested_width;
            result.saved_height = result.requested_height;
        }
    }
    output = result;
    return true;
}
}
