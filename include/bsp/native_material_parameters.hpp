#pragma once
#include "bsp/native_material_pools.hpp"
#include <array>

namespace bsp {
// Actual84h payload inside the parameter pool's88h slot. Name/source/registers
// are the material's only parameter state; slot+84 remains owned by the pool.
// Inline construction in B44D60 leaves word_count_0c and padding_11 unwritten
// until the registration writes the count. The exact matrix BYTE is preserved.
struct NativeMaterialParameterStorage {
    NativeString name_00;
    const void* source_08;
    std::uint32_t word_count_0c;
    std::uint8_t matrix_10;
    std::array<std::byte, 3> padding_11;
    std::array<std::int32_t, 14> vertex_registers_14;
    std::array<std::int32_t, 14> pixel_registers_4c;
};

using NativeMaterialParameterNameCompare = int (*)(const char*, const char*);
struct NativeMaterialParameterAccess {
    NativeMaterialParameterPool& parameter_slots; // same actual F8D3E4
    NativeStringStorage& parameter_names;         // actual00419CC0 string service
    // Original CRT __stricmp/00BF7FBF contract and locale. Required only when
    // equal nonzero lengths reach that call; no copied/lowercased name index.
    NativeMaterialParameterNameCompare compare_names_00bf7fbf;
    // Same actual D61A00 table view; wrapper validates current slot+10.
    // Direct B44D60 does not dispatch virtually and does not use this view.
    const volatile std::uint32_t* effect_table_00d61a00;
};

// Exact leaf getters over the same20h shader-constant entry. Name is the actual
// eight-byte header at+14; register is the signed DWORD at+00. No name copy.
const void* native_shader_constant_name_00b5b820(const void* actual_constant) noexcept;
std::int32_t native_shader_constant_register_00b5b870(const void* actual_constant) noexcept;
// Incoming native ECX=84h is discarded; B18790 routes to actual F8D3E4/B185A0.
void* allocate_native_material_parameter_slot_00b18790(NativeMaterialParameterPool&);

// Full B44D60: ECX actual effect, stack material/existing/name/source/count/flag,
// RET18h. Effect+C8..FC has fourteen current pass pointers; each nonnull pass
// has actual vertex/pixel owners+70/+74 whose ordinary constants are pointer
// +78/count+7C,20h stride. Existing records and skipped selectors are preserved
// when no stage matches. A first qualifying selector initializes/updates once.
// Borrows actual metadata, source memory and lifetime; adds no resource retain.
NativeMaterialParameterStorage* bind_native_material_parameter_00b44d60(
    void* actual_effect, NativeMaterialStorage& material,
    NativeMaterialParameterStorage* existing, const void* actual_name_header,
    const void* source, std::uint32_t word_count, std::uint8_t matrix,
    NativeMaterialParameterAccess&);

// Full B17E10: ECX material, stack name/source/count/flag, RET10h. Null effect
// returns before reading name/table/source or consulting the dispatch view.
// Searches the current actual+80/+100 table, then reloads effect7C and dispatches
// its supported current D61A00/+10=B44D60 profile. Unknown profiles fail.
NativeMaterialParameterStorage* register_native_material_parameter_00b17e10(
    NativeMaterialStorage&, const void* actual_name_header, const void* source,
    std::uint32_t word_count, std::uint8_t matrix, NativeMaterialParameterAccess&);

// Calls must be serialized over live, valid native metadata and storage. Counted
// material slots must be nonnull and in0..32. Registration does not dereference
// borrowed source words; they must survive until their actual consumers finish.
// Native corrupt extents, null backing allocation and invalid pointer behavior
// are outside this C++ interface. No rollback, secondary table, effect projection,
// widget lifetime token or native binary ABI replacement is supplied.
} // namespace bsp
