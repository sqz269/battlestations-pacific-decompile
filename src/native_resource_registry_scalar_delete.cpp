#include "bsp/native_resource_registry_scalar_delete.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource registry scalar deletion requires MSVC Win32.
#endif

namespace bsp {
namespace {
void __fastcall destroy_bound_registry(
    void* registry, const NativeResourceRegistryDeleteBindings* bindings) {
    destroy_native_resource_registry_00b1b5f0(
        registry, bindings->actual_publication_00f8d41c,
        bindings->strings, bindings->invalid_parameters);
}

void __cdecl free_registry_allocation(void* registry) noexcept {
    singleton_lifetime_free(registry);
}
} // namespace

__declspec(naked) void* __fastcall delete_native_resource_registry_00b1b660(
    void*, const NativeResourceRegistryDeleteBindings*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        call destroy_bound_registry
        test byte ptr [esp + 8], 1
        jz retained
        push esi
        call free_registry_allocation
        add esp, 4
    retained:
        mov eax, esi
        pop esi
        ret 4
    }
}

__declspec(naked) void* __fastcall delete_native_resource_registry_00b1b710(
    void*, const NativeResourceRegistryDeleteBindings*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        call destroy_bound_registry
        test byte ptr [esp + 8], 1
        jz retained
        push esi
        call free_registry_allocation
        add esp, 4
    retained:
        mov eax, esi
        pop esi
        ret 4
    }
}

} // namespace bsp
