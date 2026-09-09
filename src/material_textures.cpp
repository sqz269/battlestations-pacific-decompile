#include "bsp/material_textures.hpp"

namespace bsp {
bool set_material_texture_00b189f0(MaterialTextureSlots& material, std::uint32_t slot,
    const std::shared_ptr<LogicalTexture>& texture) {
    if (slot >= MaterialTextureSlots::capacity) return false;
    // Snapshot before growth: texture can refer to material.textures()[another].
    // This temporary host retain is not a native intrusive-reference operation.
    auto incoming = texture;
    if (slot >= material.textures_.size()) material.textures_.resize(slot + 1);
    auto& current = material.textures_[slot];
    // Native high-water update precedes this pointer-identity short circuit.
    if (current.get() == incoming.get()) return true;
    // New ownership is already retained. Publish it before releasing the old
    // owner, including when its deleter inspects the material during destruction.
    current.swap(incoming);
    return true;
}
}
