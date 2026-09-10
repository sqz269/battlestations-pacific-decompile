#pragma once
#include "bsp/effect_cache.hpp"
#include "bsp/material_lighting.hpp"
#include "bsp/material_parameter_bindings.hpp"
#include "bsp/material_textures.hpp"
#include <cstdint>
#include <memory>

namespace bsp {
// Semantic projection of every field transferred by00b18b60, plus its
// explicitly reset parameter count+100h. Offsets retain their names where
// meaning is unresolved. Native unwritten table slots become an empty table.
// Default aggregate values are not the ordinary constructor00b18900: that
// constructor sets+104h/+108h toFFFFFFFF. Supply the actual source values.
struct MaterialCloneState {
    std::uint32_t word08{};
    const void* pointer0c{};
    // Native+0Ch is retained only if copied byte+10Dh is nonzero. The pointer
    // remains borrowed when that byte is zero, even if source.owner0c exists.
    // For a retained non-null pointer, owner0c.get() must equal pointer0c.
    std::shared_ptr<const void> owner0c;
    MaterialTextureSlots textures; // Nine slots+10h..+30h, signed count+34h.
    MaterialLighting lighting; // Exact17DWORDs+38h..+78h, flag+10Ch.
    EffectOwner effect; // Retained native effect+7Ch; identity is preserved.
    MaterialParameterBindings parameters; // inline table+80..FC, count+100
    std::uint32_t word104{};
    std::uint32_t word108{};
    std::uint8_t byte10d{};
};

// Original00b18b60: ECX fresh110h-byte destination, stack source material;
// EAX destination; RET4. Returns a distinct typed clone here, not a native
// layout/ABI replacement. Copies all proved fields, sparse/null texture slots
// and their high-water count; forces lighting flag+10Ch on and table count zero.
// Shared effect metadata is retained so the clone can register new parameters.
// Source byte+10Dh is copied exactly, not normalized to bool. Effect and
// texture owners retain identity; LogicalTexture's COM pointer stays borrowed.
//
// Source conditional borrowed pointers and underlying COM resources must
// outlive consumers. This function does not perform a deep resource clone.
// Owner validation, HRESULT failures and unchanged output on failure are new
// host policy; native malformed count/OOM/SEH behavior is not reproduced.
HRESULT clone_material_00b18b60(const MaterialCloneState& source,
    std::shared_ptr<MaterialCloneState>& output);
}
