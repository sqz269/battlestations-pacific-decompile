#include "bsp/native_material_constant_build_leaves.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4);
__declspec(naked) void* __fastcall native_material_parameter_table_00b17390(const void*) noexcept {
    __asm { lea eax, [ecx+80h] }
    __asm { ret }
}
__declspec(naked) std::uint32_t __fastcall native_shader_constant_register_count_00b5b880(const void*) noexcept {
    __asm { mov eax, [ecx+4] }
    __asm { ret }
}
__declspec(naked) std::int32_t __fastcall native_model_bone_count_00b8ff00(const void*) noexcept {
    __asm { mov eax, [ecx+188h] }
    __asm { ret }
}
__declspec(naked) void* __fastcall native_model_bone_node_00b90620(const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, [ecx+184h]
        mov ecx, [esp+4]
        mov eax, [eax+ecx*4]
        ret 4
    }
}
__declspec(naked) std::uint32_t __fastcall native_vertex_declaration_element_count_00b47900(const void*) noexcept {
    __asm { mov eax, [ecx+10h] }
    __asm { ret }
}
__declspec(naked) void* __fastcall native_logical_vertex_decode_record_00b61e10(const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, [esp+4]
        shl eax, 5
        add eax, [ecx+50h]
        ret 4
    }
}
__declspec(naked) std::uint32_t __fastcall native_optimized_animator_type_00b75e50(const volatile std::uint32_t*) noexcept {
    __asm { mov eax, [ecx] }
    __asm { ret }
}
__declspec(naked) std::uint8_t __fastcall native_optimized_animator_has_type_00b782d0(
    const void*, const volatile std::uint32_t*, std::uint32_t) noexcept {
    __asm {
        mov ecx, [esp+4]
        mov eax, edx
        add edx, 0ch
    again:
        cmp [eax], ecx
        je found
        add eax, 4
        cmp eax, edx
        jne again
        xor al, al
        ret 4
    found:
        mov al, 1
        ret 4
    }
}
} // namespace bsp
