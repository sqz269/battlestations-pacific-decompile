#pragma once
#include <cstdint>

namespace bsp {
struct ShaderLuaOptions;

// Explicit state for native construction order, not a resource-name/COM hash.
// Native globals00f8d3a8 and0108d6e8 both start at zero in the PE's zero-fill
// data. Supply the actual next values when joining an existing domain. Calls
// must be serialized; native increments are not interlocked. Never create a
// separate domain for each asset, clone, category or texture subtype.
struct MaterialSortMetadataCounters {
    MaterialSortMetadataCounters(std::uint32_t effect_next,
        std::uint32_t texture_next) noexcept
        : next_effect_serial_c0(effect_next), next_texture_serial20(texture_next) {}
    std::uint32_t next_effect_serial_c0;
    std::uint32_t next_texture_serial20;
};

struct MaterialEffectSortMetadata {
    explicit MaterialEffectSortMetadata(std::uint32_t serial) noexcept
        : construction_serial_c0(serial) {}
    // Native constructor stores a DWORD;00b51df0 reads only its low BYTE.
    std::uint32_t construction_serial_c0;
    // These placeholders are unreadable until descriptor_assigned: native
    //00b18d60/00b407a0 do NOT initialize effect+B0 or+AC to zero.
    std::int32_t priority_b0{}, pipe_id_ac{};
    bool descriptor_assigned{};
};
struct LogicalTextureSortMetadata {
    explicit LogicalTextureSortMetadata(std::uint32_t serial) noexcept
        : construction_serial20(serial) {}
    // Native wrapper+20 DWORD. The batch key masks this to12 bits.
    std::uint32_t construction_serial20;
};

// Numeric metadata step at00b18e60..00b18e7b, within base ctor00b18d60
// (ECX object, RET, EAX object). Consume exactly once when construction reaches
// this step, AFTER error.tga acquisition/temporary cleanup. A subsequent effect
// load failure does not undo it. Does not run the remaining native constructor.
MaterialEffectSortMetadata construct_material_effect_sort_metadata_00b18d60(
    MaterialSortMetadataCounters&) noexcept;

// Numeric metadata step at00b341af..00b341b7 within base ctor00b34120
// (ECX object; stack native-name, native texture, policy; RET0C; EAX object).
// The same counter step is in00b33fc0/00b34020/00b340a0. Count EVERY covered
// native construction, including later failed initialization, across all such
// texture types. Existing-wrapper recreation/retention does not allocate a new
// serial. No native string/COM/base-lifetime implementation is implied.
LogicalTextureSortMetadata construct_logical_texture_sort_metadata_00b34120(
    MaterialSortMetadataCounters&) noexcept;

// Assignment fragment00b45f63..00b45f7b in loader00b45ee0 (ECX effect; stack
// name, byte policy, override flag; RET0C). Read parsed descriptor+8 -> B0,
// then descriptor+4 -> AC. This is the ROOT descriptor at effect+C4, shared by
// all mode passes; per-mode descriptors do not supply these fields. The
// existing ShaderLuaOptions represent Priority
// and PipeID at those offsets, including native parser defaults. Call when the
// descriptor read completes, BEFORE later shader/pass creation, so metadata
// remains assigned even if a subsequent loader stage fails. Repeated calls
// update only descriptor metadata and preserve the construction serial.
void apply_material_effect_sort_descriptor_00b45ee0(MaterialEffectSortMetadata&,
    const ShaderLuaOptions&) noexcept;
}
