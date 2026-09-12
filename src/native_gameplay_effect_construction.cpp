#include "bsp/native_gameplay_effect_construction.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeGuard) == 8 && offsetof(NativeGuard, section_04) == 4);

volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

// Complete native normal constructor operations after its cleanup is armed.
__declspec(naked) void* __fastcall construct_effect_body(void*) {
    __asm {
        push esi
        push edi
        mov edi, ecx
        lea esi, [edi + 4]
        mov ecx, esi
        mov dword ptr [edi], 0x00d0da64
        call allocate_native_int_pointer_tree18_node_0086ac00
        mov dword ptr [esi + 4], eax
        mov byte ptr [eax + 0x15], 1
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax + 4], eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax], eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax + 8], eax
        mov eax, edi
        mov dword ptr [esi + 8], 0
        pop edi
        pop esi
        ret
    }
}
} // namespace

__declspec(naked) void __fastcall reset_native_gameplay_effect_manager_00869c50(
    void*, void* volatile&) {
    __asm {
        mov dword ptr [edx], 0
        mov dword ptr [ecx], 0x00ce3818
        ret
    }
}

__declspec(noinline) void* __fastcall construct_native_gameplay_effect_manager_00870370(
    void* owner, void* volatile& actual_publication_00f87664) {
    try {
        return construct_effect_body(owner);
    } catch (...) {
        reset_native_gameplay_effect_manager_00869c50(owner, actual_publication_00f87664);
        throw;
    }
}

__declspec(noinline) void* get_native_gameplay_effect_manager_004c1650(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_effect_publication_00f87664) {
    void* const initial = actual_effect_publication_00f87664;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_counter(captured_section);
        depth = depth + 1u;
    }
    try {
        if (!actual_effect_publication_00f87664) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x10, 0x10});
            void* result = nullptr;
            try {
                if (allocation) {
                    result = construct_native_gameplay_effect_manager_00870370(
                        allocation, actual_effect_publication_00f87664);
                }
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            // Native state1 ends before publication and registration.
            actual_effect_publication_00f87664 = result;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            void* const current_effect = actual_effect_publication_00f87664;
            register_native_singleton_object_00bd0c30(current_manager, nullptr, current_effect);
        }
        if (captured_section) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return actual_effect_publication_00f87664;
}
} // namespace bsp
