#include "bsp/native_shadow_node_array_release.hpp"
#include "bsp/generated_model_lifetime.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_shadow_pointer_vector.hpp"
#include "bsp/render_command_queue.hpp"
#include <cstdint>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow node-array release requires MSVC Win32 raw assembly.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
namespace {
// Explicit source-domain bridge to the existing real B6DFA0 fragment. It
// creates no binding and does not read a native profile/slot before unlink.
void __fastcall release_actual_model(void* actual_node,
    GeneratedModelLifetimeRuntime* lifetimes) {
    if (!lifetimes)
        throw std::logic_error("shadow node release requires the canonical lifetime runtime");
    const auto key = reinterpret_cast<std::uint32_t>(actual_node);
    auto* const reference = dynamic_cast<NativeModelReference*>(
        lifetimes->find_actual_node(key));
    if (!reference)
        throw std::logic_error("shadow node release requires an actual canonical Model reference");
    auto& owner = reference->model_owner();
    // Check host phase before forming an address through possibly ended
    // storage. This validates the existing binding, not native slot18.
    if (owner.phase != NativeModelOwner::Phase::live ||
        reinterpret_cast<std::uint32_t>(&owner.storage.node) != key ||
        &owner.environment.nodes.attachments != lifetimes)
        throw std::logic_error("shadow node release requires the same live actual Model owner");
    unlink_and_release_render_model_00b6dfa0(*reference);
    // No access to owner/reference/storage after a possibly terminal release.
}
} // namespace

__declspec(naked) void __fastcall release_native_shadow_entry_nodes_00ae1c20(
    void*, GeneratedModelLifetimeRuntime*) {
    __asm {
        push edx // Source runtime at entry S-4; original has no public words.
        push ebx // AE1C20
        push edi // AE1C21
        mov ebx, ecx // AE1C22; actual entry
        xor edi, edi // AE1C24; index0
        cmp dword ptr [ebx + 48h], edi // AE1C26; signed initial count
        jle release_resize_zero // AE1C29
        push esi // AE1C2B; only the positive-count path saves ESI
        // AE1C2C: preserve exact four-byte LEA ESP,[ESP] alignment encoding.
        _emit 08dh
        _emit 064h
        _emit 024h
        _emit 000h
    release_current_node:
        mov eax, dword ptr [ebx + 44h] // AE1C30; CURRENT data
        mov ecx, dword ptr [eax + edi*4] // AE1C33; CURRENT node
        test ecx, ecx // AE1C36
        lea esi, [eax + edi*4] // AE1C38; capture slot before branch; keep flags
        jz release_next_node // AE1C3B
        mov edx, dword ptr [esp + 0ch] // Source runtime S-4, ESP=S-16
        call release_actual_model // AE1C3D -> concrete existing B6DFA0
        mov dword ptr [esi], 0 // AE1C42; CAPTURED slot AFTER normal release
    release_next_node:
        add edi, 1 // AE1C48
        cmp edi, dword ptr [ebx + 48h] // AE1C4B; CURRENT signed count
        jl release_current_node // AE1C4E; next iteration reloads data
        pop esi // AE1C50
    release_resize_zero:
        push 0 // AE1C51
        lea ecx, [ebx + 44h] // AE1C53; actual header; EDX is unused
        call resize_native_shadow_pointer_vector_00ae19b0 // AE1C56; full body, RET4
        pop edi // AE1C5B
        pop ebx // AE1C5C
        add esp, 4 // Remove source runtime only.
        _emit 0c3h // AE1C5D; exact no-public-argument RET encoding
    }
}
} // namespace bsp
