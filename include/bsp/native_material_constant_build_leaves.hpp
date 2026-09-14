#pragma once
#include <cstdint>

namespace bsp {
// Raw native storage; no companion, copy, retain or bounds check.
void* __fastcall native_material_parameter_table_00b17390(const void*) noexcept;
std::uint32_t __fastcall native_shader_constant_register_count_00b5b880(const void*) noexcept;
std::int32_t __fastcall native_model_bone_count_00b8ff00(const void*) noexcept;
void* __fastcall native_model_bone_node_00b90620(const void*, void*, std::uint32_t) noexcept;
std::uint32_t __fastcall native_vertex_declaration_element_count_00b47900(const void*) noexcept;
void* __fastcall native_logical_vertex_decode_record_00b61e10(const void*, void*, std::uint32_t) noexcept;
// Original B75E50 has no arguments. Source ECX borrows its actual publication.
std::uint32_t __fastcall native_optimized_animator_type_00b75e50(const volatile std::uint32_t*) noexcept;
// Original ECX owner is unused, stack type, RET4, AL result. Source EDX
// supplies the SAME current three DWORD cells [10900FC,1090108).
std::uint8_t __fastcall native_optimized_animator_has_type_00b782d0(
    const void*, const volatile std::uint32_t*, std::uint32_t) noexcept;
} // namespace bsp
