#pragma once

#include "bsp/environment_fog_apply.hpp"
#include <cstddef>

namespace bsp {

// Mutable references to the SAME four regions consumed by EnvironmentFogFields.
// They belong to the actual environment; this view owns no storage or defaults.
struct MutableEnvironmentFogFields {
    std::array<float, 11>& scalars_08;
    std::array<float, 4>& color_34;
    std::array<std::array<float, 4>, 4>& directional_colors_44;
    std::array<float, 4>& underwater_color_84;

    EnvironmentFogFields read_only() const noexcept {
        return {scalars_08, color_34, directional_colors_44, underwater_color_84};
    }
};

inline constexpr std::size_t kAuthoredFogMinimumReadableBytes = 0x128;

// Interior [0078CAA4,0078CCE0) of 0078C9B0: ESI=actual authored block,
// EDI=environment. New host ABI, not the enclosing native thiscall/RET4.
// Readable source bytes include [80h,E8h) with holes and [104h,128h).
// The source, four regions and actual environment+B4 projection slot stay live
// throughout the call. Every nonnull projection must belong to an existing
// SystemFogOwner; no allocation, retained reference or alternate fog state.
//
// Copies raw forward color/underwater/directional words, then eleven FLD/FSTP
// scalars. Captures B4 after D0->08 and before D4->0C; that captured owner gets
// the first color. Reloads B4 for each of four directionals and eleven scalars.
// Underwater is copied into the environment but NOT into its private owner.
// All reads remain live, including overlapping source/destination bindings.
//
// Null/short source is a checked host error before writes. A null owner fails
// only at its native consumption point, retaining prior writes/x87 effects.
// These checks do not reproduce native invalid-pointer exception behavior.
bool copy_authored_fog_0078caa4(const void* actual_authored_block,
    std::size_t readable_bytes, const MutableEnvironmentFogFields& environment,
    const SystemFogState* const& actual_environment_private_b4, std::string& error);

} // namespace bsp
