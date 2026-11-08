#pragma once
#include <cstdint>

namespace bsp {
struct NativeMaterialParameterAccess;
namespace cc13_terrain {

// 00B18AC0, __thiscall(material, name, source, vector_count), RET 0Ch.
// Borrows the live material, NativeString header and source; B17E10 retains
// the source address. Access must be the material's actual pool/name services.
// The LEA multiplies the complete DWORD count by four with x86 wraparound.
// New source interface; the original binary ABI and gameplay are not claimed.
void* material_register_float4_array_00b18ac0(void* material, const void* name,
    const void* source, std::uint32_t vector_count, NativeMaterialParameterAccess& access);

} // namespace cc13_terrain
} // namespace bsp
