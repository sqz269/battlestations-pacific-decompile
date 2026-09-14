#pragma once

#include "bsp/native_material_owner.hpp"

#include <cstdint>

namespace bsp {

// Original ECX is the actual material, EAX is its parameter-table ADDRESS
// (+80h), and RET leaves the caller's stack untouched. Borrowed storage.
void* const* __fastcall get_native_material_parameter_table_00b17390(
    const NativeMaterialStorage* actual_material) noexcept;

// Original ECX is the actual vertex declaration. The DWORD at +10h is
// returned without checking its value or the object's address.
std::uint32_t __fastcall get_native_vertex_declaration_element_count_00b47900(
    const void* actual_declaration) noexcept;

// Original ECX is the actual shader-constant metadata entry. RegisterCount is
// the DWORD at +04h; Rows at +08h is a separate field.
std::uint32_t __fastcall get_native_shader_constant_register_count_00b5b880(
    const void* actual_constant) noexcept;

// Original 00B75E50 has no input and reads absolute global cell 010900FC.
// This source interface receives a reference to that SAME live cell in ECX,
// then reads its CURRENT DWORD. It must not receive a copied type ID or a
// pointer-value snapshot. The added ECX input changes the binary ABI.
std::uint32_t __fastcall get_native_optimized_animator_type_id_00b75e50(
    const volatile std::uint32_t& actual_type_id_cell_010900fc) noexcept;

// Original ECX is the actual light, EAX its borrowed shadow-map owner at
// +174h. No retain, null substitution, or pointer validation is performed.
void* __fastcall get_native_light_shadow_map_owner_00b7aab0(
    const void* actual_light) noexcept;

} // namespace bsp
