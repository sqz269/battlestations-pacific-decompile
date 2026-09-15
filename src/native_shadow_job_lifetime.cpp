#include "bsp/native_shadow_job_lifetime.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}

// Actual raw pointer/critical-section representations, including unaligned
// DWORD fields. No typed uint32_t alias or early aggregate snapshot.
__declspec(noinline) std::uint32_t current_word(void* base,
    std::uint32_t offset) noexcept {
    void* const address = at(base, offset);
    std::uint32_t result;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}

__declspec(noinline) void store_word(void* base, std::uint32_t offset,
    std::uint32_t value) noexcept {
    void* const address = at(base, offset);
    __asm {
        mov eax, address
        mov edx, value
        mov dword ptr [eax], edx
    }
}

// Source EDX is already the address of the actual publication cell. Obtain it
// in C++ before entering this body; handwritten assembly assumes no layout
// for the context's C++ reference members. Native flags/store/free schedule.
__declspec(naked) void* __fastcall delete_primary_body(
    void*, void* volatile*, std::uint32_t) noexcept {
    __asm {
        push esi
        mov esi, ecx
        test esi, esi
        je null_primary
        lea eax, [esi + 4]
        jmp secondary_ready
    null_primary:
        xor eax, eax
    secondary_ready:
        test byte ptr [esp + 8], 1 // A8DDEE: flags BEFORE clear/stamp
        mov dword ptr [edx], 0 // A8DDF3: actual E18AD4, unconditional
        mov dword ptr [eax], 00ce3818h // A8DDFD: nullable secondary
        jz retained
        push esi
        call singleton_lifetime_free // A8DE06 / BF65AC
        add esp, 4 // A8DE0B: original installed bytes, listing gap
    retained:
        mov eax, esi
        pop esi
        ret 4
    }
}
} // namespace

void* get_native_shadow_job_00a8e090(NativeShadowJobContext& context) {
    void* const initial = context.actual_job_publication_00e18ad4; // A8E0A5
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0); // A8E0B6
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(
        current_word(first_manager, 0x10)); // A8E0BB
    alignas(4) std::uint32_t guard[2]{0x00ce37fc,
        reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section); // A8E0CF / CE2218
        store_word(section, 0x18, current_word(section, 0x18) + 1u); // A8E0D5
    }
    // A8E0D9: state0 armed only after Enter and the actual counter increment.
    try {
        if (!context.actual_job_publication_00e18ad4) { // A8E0E1
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8}); // A8E0EC / BF681B
            if (allocation) {
                store_word(allocation, 4, 0x00d5b568); // A8E0F8
                store_word(allocation, 0, 0x00d5b570); // A8E0FF
                store_word(allocation, 4, 0x00d5b56c); // A8E105
            }
            context.actual_job_publication_00e18ad4 = allocation; // A8E10C/E113
            void* const current = context.actual_job_publication_00e18ad4; // A8E11D
            void* const secondary = current ? at(current, 4) : nullptr;
            // A8E12D captures even a null secondary before the manager call.
            void* const second_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0); // A8E12E
            register_native_singleton_object_00bd0c30(
                second_manager, nullptr, secondary); // A8E135
        }
        if (section) {
            store_word(section, 0x18, current_word(section, 0x18) - 1u); // A8E13E
            LeaveCriticalSection(section); // A8E143 / CE2210
        }
    } catch (...) {
        // Native CB63F0 -> 411EE0, guard ONLY. Publication/allocation survive.
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_job_publication_00e18ad4; // A8E149, after Leave
}

void* __fastcall delete_native_shadow_job_00a8dde0(void* primary,
    NativeShadowJobContext& context, std::uint32_t flags) noexcept {
    return delete_primary_body(primary,
        &context.actual_job_publication_00e18ad4, flags);
}

__declspec(naked) void* __fastcall delete_native_shadow_job_secondary_00a8ddb0(
    void*, NativeShadowJobContext&, std::uint32_t) noexcept {
    __asm {
        sub ecx, 4
        jmp delete_native_shadow_job_00a8dde0
    }
}
} // namespace bsp
