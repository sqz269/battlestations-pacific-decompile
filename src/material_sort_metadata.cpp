#include "bsp/material_sort_metadata.hpp"
#include "bsp/shader_lua.hpp"

namespace bsp {
MaterialEffectSortMetadata construct_material_effect_sort_metadata_00b18d60(
    MaterialSortMetadataCounters& counters) noexcept {
    const auto serial = counters.next_effect_serial_c0;
    // Native MOV old DWORD to+C0, ADD EAX,1, MOV global: modulo2^32.
    ++counters.next_effect_serial_c0;
    return MaterialEffectSortMetadata(serial);
}

LogicalTextureSortMetadata construct_logical_texture_sort_metadata_00b34120(
    MaterialSortMetadataCounters& counters) noexcept {
    const auto serial = counters.next_texture_serial20;
    // All four established texture base paths copy old DWORD before ADD1.
    ++counters.next_texture_serial20;
    return LogicalTextureSortMetadata(serial);
}

void apply_material_effect_sort_descriptor_00b45ee0(MaterialEffectSortMetadata& effect,
    const ShaderLuaOptions& descriptor) noexcept {
    effect.priority_b0 = descriptor.priority;
    effect.pipe_id_ac = descriptor.pipe_id;
    effect.descriptor_assigned = true;
}
}
