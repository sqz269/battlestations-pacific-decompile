#pragma once
#include "bsp/d3d9_states.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace bsp {
class MaterialTextureSlots {
public:
    static constexpr std::uint32_t capacity = 9; // Constructor00b18900: +10..+30.
    std::int16_t count() const noexcept { return static_cast<std::int16_t>(textures_.size()); }
    const std::vector<std::shared_ptr<LogicalTexture>>& textures() const noexcept { return textures_; }
private:
    std::vector<std::shared_ptr<LogicalTexture>> textures_;
    friend bool set_material_texture_00b189f0(MaterialTextureSlots&, std::uint32_t,
        const std::shared_ptr<LogicalTexture>&);
};
// ECX native material, unsigned slot/resource stack arguments, RET8.
// Typed valid-owner projection: null sets still raise the high-water count.
// Rejects slots>=9 unchanged. Source may alias an existing owner's slot.
// Retains LogicalTexture ownership; its COM texture pointer remains borrowed.
bool set_material_texture_00b189f0(MaterialTextureSlots&, std::uint32_t slot,
    const std::shared_ptr<LogicalTexture>& texture);
}
