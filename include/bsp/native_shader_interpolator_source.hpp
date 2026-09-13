#pragma once
#include "bsp/native_shader_struct_declarations.hpp"

namespace bsp {
// Stable source call frame; actual B0h builder and pooled 8h strings retain
// their native layout. Borrowed owners/context/scratch remain alive and caller
// excludes their retirement while running/failed. No native FH3 unwind or
// implicit rollback. Existing helper children preserve their own failure rules.
struct NativeShaderInterpolatorSourceOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, row{}, completed_rows{}, mapping_offset{};
    NativeMaterialProgramBuilderStorage* builder{};
    const NativeShaderDescriptorArray* fields{};
    NativeShaderConstantHeaderContext* context{};
    NativeString temporary;
    bool temporary_live{}, temporary_captured{};
    char* captured_data{};
    std::uint32_t captured_length{}, group_count{}, last_width{};
    std::uint8_t field_index{}, component{}, position{}, fog{}, vpos{};
    const char* field_name{};
    std::unique_ptr<NativeShaderStructDeclarationOperation> line_child;
    std::unique_ptr<NativeShaderConstantHeaderOperation> format_child;
    NativeShaderInterpolatorSourceOperation() = default;
    ~NativeShaderInterpolatorSourceOperation();
    NativeShaderInterpolatorSourceOperation(const NativeShaderInterpolatorSourceOperation&) = delete;
    NativeShaderInterpolatorSourceOperation& operator=(const NativeShaderInterpolatorSourceOperation&) = delete;
    // After explicit native acquisition/child cleanup; never resumes emission.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// B35540 ECX actualB0h builder, RET. Appends into SAME builder4C, reads
// current field-list28 and two-byte mapping arrays54/60; no clearing/filtering.
void append_native_shader_interpolator_pack_00b35540(NativeMaterialProgramBuilderStorage&,
    NativeShaderConstantHeaderContext&, NativeShaderInterpolatorSourceOperation&);
// B36E30 ECX builder, stacked position/fog/vPos LOW BYTE flags, RET0C.
// Writes actual7C/80/84/88 via wrapped signed division/remainder; reloads
// emitted group count/last width each iteration. Reads descriptor70/74+30.
void append_native_shader_interpolator_struct_00b36e30(NativeMaterialProgramBuilderStorage&,
    std::uint8_t position, std::uint8_t fog, std::uint8_t vpos,
    NativeShaderConstantHeaderContext&, NativeShaderInterpolatorSourceOperation&);
// B37000 ECX builder, stacked actual0Ch field pointer-list header, RET4.
// Current supplied list, not builder28, names mapped components. Fog is copied
// or zeroed according to current builder98; either descriptor lowbyte30 adds
// vPos. Unmapped values remain uninitialized. Valid mapped field pointers and
// producer-generated component bytes0..3 required; no source bounds clamp.
void append_native_shader_interpolator_unpack_00b37000(NativeMaterialProgramBuilderStorage&,
    const NativeShaderDescriptorArray&, NativeShaderConstantHeaderContext&,
    NativeShaderInterpolatorSourceOperation&);
} // namespace bsp
