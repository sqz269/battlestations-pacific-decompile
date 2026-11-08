#include "bsp/native_enum_map_lifetime.hpp"
#include "bsp/native_enum_registry_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum map lifetime requires MSVC Win32.
#endif
namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall
initialize_native_cenum_table_map_storage_004ba130(void*) noexcept {
    __asm {
        mov edx, ecx
        push edi
        xor eax, eax
        lea edi, [edx + 8]
        mov ecx, 40h
        mov dword ptr [edx], 00ce7514h
        mov dword ptr [edx + 4], 0
        rep stosd
        mov eax, edx
        pop edi
        ret
    }
}

__declspec(naked) void* __fastcall
initialize_native_scene_enum_registry_storage_004d25f0(void*) noexcept {
    // 004D25F3..004D2611 writes the outer phase word, then exactly the
    // 004BA130 map layout at receiver+4: CE7514, zero count, 64 zero heads.
    // Use that existing body on the actual nested map instead of repeating
    // its stores. The child preserves EDI and consumes the same REP STOSD
    // direction flag; restore the outer receiver in both native result regs.
    __asm {
        mov dword ptr [ecx], 00ce78bch
        push ecx
        lea ecx, [ecx + 4]
        call initialize_native_cenum_table_map_storage_004ba130
        pop eax
        mov edx, eax
        ret
    }
}

void destroy_native_cenum_table_map_004d0ea0(void* receiver, void* table_pool,
    void* symbol_pool, NativeStringRawPoolContext& strings) {
    __asm {
        mov eax, receiver
        mov dword ptr [eax], 00ce7514h
    }
    clear_native_cenum_table_map_004d0760(receiver, table_pool, symbol_pool, strings);
}
} // namespace bsp
