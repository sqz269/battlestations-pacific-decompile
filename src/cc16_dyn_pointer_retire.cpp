#include "bsp/cc16_dyn_pointer_retire.hpp"

#include "bsp/native_game_physics_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered pointer-list schedule requires MSVC Win32.
#endif

namespace bsp::cc16_dyn_pointer_retire {
namespace {
static_assert(sizeof(void*) == 4);
using Context = DynPointerRetireContext;

void* __cdecl allocate_bridge(const Context* context, std::uint32_t bytes) {
    return context->allocators.operator_new_00bf55be(bytes);
}
void __cdecl storage_free_bridge(const Context* context, void* storage) {
    context->allocators.storage_free_00bf6989(storage);
}
void __cdecl object_free_bridge(const Context* context, void* object) {
    context->allocators.object_free_00bf65ac(object);
}
void __cdecl destroy_bridge(const Context* context, void* object) {
    // The existing function name is retained; no native class is inferred from
    // it. All services and construction bindings come from the actual caller.
    destroy_native_dyn_world_00c4dc60(
        object, context->destruction_memory, context->lifetime_calls,
        context->lifetime_progress, context->body_creation);
}

// All shims receive stack(context, original argument). Cdecl allocator shims
// remove only the inserted context; the native caller still removes its arg.
__declspec(naked) void allocate_shim() {
    __asm {
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call allocate_bridge
        add esp, 8
        ret 4
    }
}
__declspec(naked) void storage_free_shim() {
    __asm {
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call storage_free_bridge
        add esp, 8
        ret 4
    }
}
__declspec(naked) void object_free_shim() {
    __asm {
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call object_free_bridge
        add esp, 8
        ret 4
    }
}
// Native C4DC60 consumes its one stack argument (RET4). Remove that argument
// as well as the inserted context; the following BF65AC has a separate PUSH.
__declspec(naked) void destroy_shim() {
    __asm {
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call destroy_bridge
        add esp, 8
        ret 8
    }
}

// Private ABI: ESI raw list, EDX search pointer, one extra Context* stack arg.
// Context offsets account for the ACTUAL native pushes at each call. Every
// original instruction remains address-marked; only calls/context/RET adapt.
__declspec(naked) void __cdecl kernel() {
    __asm {
        mov eax, dword ptr [esi]             // 00c55df0
        mov ecx, dword ptr [esi + 4]         // 00c55df2
        lea ecx, [eax + ecx*4]               // 00c55df5
        cmp eax, ecx                        // 00c55df8
        jz l_00c55e0b                        // 00c55dfa
        lea esp, [esp]                      // 00c55dfc
    l_00c55e00:
        cmp dword ptr [eax], edx             // 00c55e00
        jz l_00c55e0b                        // 00c55e02
        add eax, 4                          // 00c55e04
        cmp eax, ecx                        // 00c55e07
        jnz l_00c55e00                       // 00c55e09
    l_00c55e0b:
        mov edx, dword ptr [ecx - 4]         // 00c55e0b
        push ebp                            // 00c55e0e
        mov ebp, dword ptr [eax]             // 00c55e0f
        mov dword ptr [eax], edx             // 00c55e11
        mov eax, dword ptr [esi + 4]         // 00c55e13
        push edi                            // 00c55e16
        lea edi, [eax - 1]                   // 00c55e17
        cmp edi, eax                        // 00c55e1a
        jbe l_00c55e87                       // 00c55e1c
        cmp edi, dword ptr [esi + 8]         // 00c55e1e
        jbe l_00c55e6a                       // 00c55e21
        lea eax, [edi*4]                     // 00c55e23
        push ebx                            // 00c55e2a
        push eax                            // 00c55e2b
        mov dword ptr [esi + 8], edi         // 00c55e2c
        push dword ptr [esp + 14h]           // Borrow actual Context*.
        call allocate_shim                  // 00c55e2f
        mov ebx, eax                        // 00c55e34
        xor eax, eax                        // 00c55e36
        add esp, 4                          // 00c55e38
        cmp dword ptr [esi + 4], eax         // 00c55e3b
        jbe l_00c55e58                       // 00c55e3e
        mov ecx, ebx                        // 00c55e40
    l_00c55e42:
        test ecx, ecx                       // 00c55e42
        jz l_00c55e4d                        // 00c55e44
        mov edx, dword ptr [esi]             // 00c55e46
        mov edx, dword ptr [edx + eax*4]     // 00c55e48
        mov dword ptr [ecx], edx             // 00c55e4b
    l_00c55e4d:
        add eax, 1                          // 00c55e4d
        add ecx, 4                          // 00c55e50
        cmp eax, dword ptr [esi + 4]         // 00c55e53
        jb l_00c55e42                        // 00c55e56
    l_00c55e58:
        mov eax, dword ptr [esi]             // 00c55e58
        test eax, eax                       // 00c55e5a
        jz l_00c55e67                        // 00c55e5c
        push eax                            // 00c55e5e
        push dword ptr [esp + 14h]           // Borrow actual Context*.
        call storage_free_shim              // 00c55e5f
        add esp, 4                          // 00c55e64
    l_00c55e67:
        mov dword ptr [esi], ebx             // 00c55e67
        pop ebx                             // 00c55e69
    l_00c55e6a:
        mov eax, dword ptr [esi + 4]         // 00c55e6a
        cmp eax, edi                        // 00c55e6d
        jae l_00c55e87                       // 00c55e6f
    l_00c55e71:
        mov ecx, dword ptr [esi]             // 00c55e71
        lea ecx, [ecx + eax*4]               // 00c55e73
        test ecx, ecx                       // 00c55e76
        jz l_00c55e80                        // 00c55e78
        mov dword ptr [ecx], 0               // 00c55e7a
    l_00c55e80:
        add eax, 1                          // 00c55e80
        cmp eax, edi                        // 00c55e83
        jb l_00c55e71                        // 00c55e85
    l_00c55e87:
        test ebp, ebp                       // 00c55e87
        mov dword ptr [esi + 4], edi         // 00c55e89
        jz l_00c55e9d                        // 00c55e8c
        push ebp                            // 00c55e8e
        push dword ptr [esp + 10h]           // Borrow actual Context*.
        call destroy_shim                   // 00c55e8f
        push ebp                            // 00c55e94
        push dword ptr [esp + 10h]           // Borrow actual Context*.
        call object_free_shim               // 00c55e95
        add esp, 4                          // 00c55e9a
    l_00c55e9d:
        pop edi                             // 00c55e9d
        pop ebp                             // 00c55e9e
        ret 4                               // 00c55e9f
    }
}
}  // namespace

// Keep parameter addresses fixed while preparing the custom register/stack
// call; suppress interposed Debug runtime-check calls. Restore both settings.
#pragma runtime_checks("", off)
#pragma optimize("y", off)
void __cdecl remove_and_destroy_00c55df0(
    void* list, void* search_pointer, const DynPointerRetireContext& context) {
    __asm {
        mov esi, list
        mov edx, search_pointer
        push context
        call kernel
    }
}
#pragma optimize("", on)
#pragma runtime_checks("", restore)

}  // namespace bsp::cc16_dyn_pointer_retire
