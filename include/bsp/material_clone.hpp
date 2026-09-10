#pragma once
#include "bsp/effect_cache.hpp"
#include "bsp/material_lighting.hpp"
#include "bsp/material_textures.hpp"
#include <cstdint>
#include <memory>

namespace bsp {
// Semantic projection of every field transferred by00b18b60, plus its
// explicitly reset+100h word. Offsets retain their names where meaning is
// unresolved. Unwritten native+80h..+FCh bytes are not synthesized here.
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
    std::uint32_t word100{};
    std::uint32_t word104{};
    std::uint32_t word108{};
    std::uint8_t byte10d{};
};

// Original00b18b60: ECX fresh110h-byte destination, stack source material;
// EAX destination; RET4. Returns a distinct typed clone here, not a native
// layout/ABI replacement. Copies all proved fields, sparse/null texture slots
// and their high-water count; forces lighting flag+10Ch on and word+100h zero.
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
