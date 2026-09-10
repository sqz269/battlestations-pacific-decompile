#pragma once
#include <array>
#include <cstdint>

namespace bsp {
using MaterialLightingValues = std::array<float, 17>;

//00b17840: ECX68-byte record; EAX record; plain RET. First four floats1,
// next twelve0, final float10. Beyond the diffuse quartet, field names are
// not established by this packet. Host value return has a new ABI.
MaterialLightingValues initialize_lighting_record_00b17840() noexcept;

// Only the lighting record/flag fragment of material constructor00b18900.
// Native material+38h is the record and byte+10Ch starts0. Texture slots,
// effect retention, other material fields and destruction remain separate.
// Evidence: docs/MATERIAL_LIGHTING.md; reports/material_lighting_audit.json.
class MaterialLighting {
public:
    MaterialLighting() noexcept;
    //00b179d0 ECX material; stack ignored slot and source record; RET8.
    // Set flag+10Ch then copy all17DWORDs exactly. Last setter wins.
    void set_lighting_record_00b179d0(std::uint32_t ignored_slot,
        const MaterialLightingValues& values) noexcept;
    //00b179f0 ECX material; unused stack slot; EAX material+38h; RET4.
    // Alias the first four floats (diffuse RGBA); writing through this pointer
    // does not itself set flag+10Ch, because the native getter only returns it.
    float* diffuse_color_00b179f0(std::uint32_t ignored_slot) noexcept;
    const float* diffuse_color_00b179f0(std::uint32_t ignored_slot) const noexcept;
    const MaterialLightingValues& values() const noexcept { return values_; }
    bool flag_10c() const noexcept { return flag_10c_; }
private:
    MaterialLightingValues values_;
    bool flag_10c_{};
};
}
