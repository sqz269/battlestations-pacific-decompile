#pragma once
#include "bsp/native_shader_system_fields.hpp"

namespace bsp {
// Host continuation storage, never inserted into the actual B0h builder or its
// 0Ch arrays. Keep this address, builder, descriptors, usage buffers, output and
// supplied string owner alive until completion or explicit diagnostic cleanup.
// A failed operation cannot replay or admit builder destruction. Native FH3
// cleanup is not reconstructed: partial acquisitions stay visible here.
struct NativeShaderInterpolatorOperation final {
    enum class Phase { fresh, field_allocation, temporary_name, field_copy,
        field_reserve, field_publication, temporary_cleanup, word_allocation,
        word_copy, word_free, word_publication, complete, failed, diagnostic_retired };
    NativeString temporary_name;
    NativeMaterialProgramBuilderStorage* builder{};
    NativeShaderDescriptorArray* output{};
    NativeStringStorage* strings{};
    const volatile std::uint32_t* texcoord_usage{};
    const volatile std::uint32_t* color_usage{};
    NativeShaderFieldStorage* current_field{};
    const NativeShaderFieldStorage* source_field{};
    NativeShaderDescriptorArray* reserve_rows{};
    void* word_allocation{};
    std::uint32_t function{}, native_site{}, descriptor_offset{}, index{};
    std::uint32_t texcoord_offset{}, color_offset{}, selected_count{}, selected_mask{};
    std::uint32_t component{}, completed_rows{};
    std::int32_t captured_scalar{}, captured_semantic{}, captured_index{}, reserve_capacity{};
    std::uint16_t mapping_word{};
    bool temporary_live{}, field_name_initialized{}, field_published{};
    Phase phase{Phase::fresh};
    NativeShaderInterpolatorOperation() = default;
    ~NativeShaderInterpolatorOperation();
    NativeShaderInterpolatorOperation(const NativeShaderInterpolatorOperation&) = delete;
    NativeShaderInterpolatorOperation& operator=(const NativeShaderInterpolatorOperation&) = delete;
    // Only after external cleanup of retained raw field/name/allocation, with
    // published fields left in their actual output owner for its normal teardown.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B346E0 ECX actual0Ch header, signed stacked capacity, RET4. Reserve TWO-byte
// elements, minimum1, allocate through shared CRT service, copy current rows,
// free current old data, then publish data/capacity. Count remains unchanged.
void reserve_native_shader_interpolator_words_00b346e0(
    NativeShaderDescriptorArray&, std::int32_t, NativeShaderInterpolatorOperation&);

// B36800 ECX actualB0h builder; stacked TEXCOORD usage, COLOR usage, actual0Ch
// output pointer-array header; RET0Ch. Append ScreenSpacePos then selected1Ch
// fields from current descriptor70 and mode_descriptor74 DC/E0 arrays. Null/null
// copies every row and regenerates low component bits. Otherwise each usage
// DWORD has four low-bit flags; separate flattened offsets continue across BOTH
// descriptors. Only a semantic whose usage pointer is present is considered.
// No clear, deduplication, source-name projection, effect78 traversal or text.
void select_native_shader_interpolator_fields_00b36800(
    NativeMaterialProgramBuilderStorage&, const volatile std::uint32_t* texcoord_usage,
    const volatile std::uint32_t* color_usage, NativeShaderDescriptorArray& output,
    NativeStringStorage&, NativeShaderInterpolatorOperation&);

// B34AA0 ECX actualB0h builder; stacked actual output pointer-array; RET4.
// Start at field1, read current masks separately for bits0..3; append field-low-
// byte/component-byte to SAME54/60 two-byte arrays. Ignore width/semantic index.
// Each FOG overwrites WORD6C with low-byte field index and zero component.
void append_native_shader_interpolator_mapping_00b34aa0(
    NativeMaterialProgramBuilderStorage&, NativeShaderDescriptorArray& fields,
    NativeShaderInterpolatorOperation&);
} // namespace bsp
