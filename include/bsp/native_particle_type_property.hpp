#pragma once
#include "bsp/native_particle_type_base.hpp"

namespace bsp {
struct NativeParticleTypePropertyBindings {
    NativeParticleTypeBaseBindings& base; // SAME strings, parameter pool and native array domain
    void* actual_atlas_manager_00f8c26c; // actual +4 pointer array/+8 signed count
    void* actual_renderer_00f8d394;
    void* context;
    // Required real services: execute the captured CURRENT target on the same
    // actual receiver. No fabricated resource, owner or success fallback.
    void* (*renderer_virtual64)(void*, void* actual_renderer, std::uint32_t target,
        const void* actual_name8h, std::uint32_t flags);
    void (*shader_virtual10)(void*, void* actual_definition, std::uint32_t target,
        const char* shader);
    void (*model_virtual20)(void*, void* actual_definition, std::uint32_t target,
        const char* model); // Object parser's independently reviewed service
    // B01350 writes neither native record+18 stack DWORD. They are distinct
    // stack slots: first record at ESP+24h, later records at ESP+40h.
    std::uint32_t first_record_stack_word18;
    std::uint32_t later_record_stack_word18;
    const char* null_pattern_00e17bf0; // SAME current writable native fallback bytes
    const char* empty_frame_name_00f8d390; // Tracer texture-list null name
    const char* empty_stem_00f8c2c1; // filename helper null data
    const char* empty_texture_name_00f8d37c; // B01350 frame-name null data
};

// Complete native bodies through their real service boundaries; new C++ APIs,
// not binary replacements. Original FH3 ABI, invalid storage/stack overflow
// faults and resource-service internals remain outside these interfaces.
// B015C0 ECX definition, stack actual4h pooled suffix, RET4/AL recognized.
bool load_native_particle_type_property_00b015c0(void* actual_definition,
    const void* actual_suffix, NativeParticleTypePropertyBindings&);
// B01350 ECX definition, stack nullable filename, RET4/AL. Empty names leave
// records untouched; renderer miss path still dereferences its actual result.
bool load_native_particle_type_texture_00b01350(void*, const char*,
    NativeParticleTypePropertyBindings&);
// AF4360 ECX actual layer owner; stack length/data BY VALUE, RET8/EAX index.
// Consumes/relinquishes the supplied string bytes; no match also returns zero.
std::int32_t find_native_particle_layer_00af4360(void* actual_layer_owner,
    std::uint32_t name_length, char* consumed_name, NativeStringStorage&);
// AF3A20 ECX filename, RET/EAX signed count. Counts contiguous atlas hits from
// 000 only when the extension-stripped name ends in three zeroes after index0.
std::int32_t count_native_particle_texture_frames_00af3a20(const char*,
    NativeParticleTypePropertyBindings&);
// AF3B50 ECX actual8h output, EDX filename, stack signed index, RET4/EAX output.
void* construct_native_particle_texture_frame_name_00af3b50(void* actual_output,
    const char*, std::int32_t index, NativeParticleTypePropertyBindings&);
bool has_native_particle_frame_suffix_00af3750(const char*) noexcept; // EDI/AL/RET
void* construct_native_particle_texture_stem_00af37d0(void*, const char*,
    NativeStringStorage&); // ECX output, EDX filename, RET/EAX output
void* construct_native_particle_texture_extension_00af38b0(void*, const char*,
    NativeStringStorage&); // ECX output, EDX filename, RET/EAX output
void* construct_native_particle_texture_prefix_00af3960(void*, const char*,
    NativeStringStorage&); // ESI output, EAX stem, RET/EAX output
// Actual1Ch record producer: four ordered x87 loads/stores (including sNaN
// quieting), actual imported half conversion into +10..+17; +18 is untouched.
void* construct_native_particle_uv_record_00b00880(void*, const void* uv_min,
    const void* uv_max, const NativeD3dx9Float32To16Import&);
void append_native_particle_type_record_00b00ee0(void* actual_descriptor,
    const void* actual_record, NativeParticleTypeBaseBindings&); // ECX,stack,RET4
void clear_native_particle_type_records_00b00f30(void* actual_descriptor,
    NativeParticleTypeBaseBindings&); // ECX,RET; retains allocation

// Actual atlas owner and raw30h item identity. No TextureAtlasItem conversion,
// independent collection or alternate string allocator is introduced.
void* find_native_particle_atlas_item_00aefb20(void* actual_manager,
    const char* name, NativeStringStorage&, const char* null_pattern_00e17bf0);
bool matches_native_particle_atlas_item_00aee0f0(const void* actual_item,
    const void* actual_query8h, const char* null_pattern_00e17bf0);
// Raw-header counterpart to the existing typed MatchesAtOffset interface.
// The null-pattern bytes are borrowed from the original writable 00E17BF0.
bool matches_native_particle_string_at_0043e9a0(const void* candidate,
    const void* pattern, std::uint32_t start, const char* null_pattern_00e17bf0) noexcept;
void resize_native_particle_string_fill_0043bbf0(void* actual_header,
    std::uint32_t length, std::int8_t fill, NativeStringStorage&);
void replace_native_particle_string_substrings_004cad40(void* actual_header,
    const void* search, const void* replacement, std::uint32_t count,
    NativeStringStorage&);
} // namespace bsp
