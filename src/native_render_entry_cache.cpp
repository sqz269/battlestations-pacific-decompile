#include "bsp/native_render_entry_cache.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(p) + offset);
}
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
struct NativeGuard { Word profile; CRITICAL_SECTION* section; };
static_assert(sizeof(void*) == 4 && sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(NativeGuard& guard) noexcept {
    __try { destroy_native_singleton_guard_00411ee0(&guard); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
void unwind_base(void* owner, NativeRenderEntryCacheContext& context) noexcept {
    __try { destroy_native_render_entry_cache_base_00bec630(owner, context); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct ProfileCleanup {
    void* owner;
    bool armed = true;
    ~ProfileCleanup() noexcept {
        if (armed) destroy_native_generic_singleton_base_00412430(owner);
    }
};
struct GuardCleanup {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard); }
};
struct BaseCleanup {
    void* owner;
    NativeRenderEntryCacheContext& context;
    bool armed = true;
    ~BaseCleanup() noexcept { if (armed) unwind_base(owner, context); }
};
struct HeaderCleanup {
    void* header;
    bool armed = true;
    ~HeaderCleanup() noexcept {
        if (armed) destroy_native_render_entry_array_00bec240(header);
    }
};
struct AllocationCleanup {
    void* allocation;
    bool armed = true;
    ~AllocationCleanup() noexcept { if (armed) singleton_lifetime_free(allocation); }
};
CRITICAL_SECTION* first_section(NativeRenderEntryCacheContext& context) {
    void* const manager = get_native_singleton_manager_00415350(context.actual_manager_01090aa0);
    return reinterpret_cast<CRITICAL_SECTION*>(word(manager, 0x10));
}
} // namespace

__declspec(naked) void* __fastcall initialize_native_render_entry_00bebf00(
    void*, const volatile std::uint32_t*) {
    __asm {
        xorps xmm0, xmm0
        mov eax, ecx
        xor ecx, ecx
        movss dword ptr [eax], xmm0
        movss dword ptr [eax + 14h], xmm0
        movss xmm0, dword ptr [edx]
        mov [eax + 8], ecx
        mov [eax + 0ch], ecx
        mov [eax + 10h], ecx
        movss dword ptr [eax + 18h], xmm0
        ret
    }
}

void destroy_native_render_entry_array_00bec240(void* actual_header) noexcept {
    singleton_lifetime_free(reinterpret_cast<void*>(word(actual_header)));
}

void resize_native_render_entry_array_00bec3e0(void* actual_header,
    Word requested, const volatile Word& actual_one_00d7a24c) {
    if (word(actual_header, 4) == requested) return;
    void* const previous = reinterpret_cast<void*>(word(actual_header));
    word(actual_header, 4) = 0;
    word(actual_header, 8) = 0;
    singleton_lifetime_free(previous);
    if (requested != 0) {
        const std::uint64_t product = static_cast<std::uint64_t>(requested) * 0x28u;
        const Word bytes = product > 0xffffffffu ? 0xffffffffu : static_cast<Word>(product);
        void* const allocation = singleton_lifetime_allocate(
            {SingletonAllocationKind::pointer_slots, bytes, bytes});
        AllocationCleanup cleanup{allocation}; // FH3 state0 captures allocation.
        if (allocation) {
            Word cursor = reinterpret_cast<Word>(allocation);
            // Specialize403560: SUB count,1; JS exit, then construct/advance/SUB/JNS.
            Word remaining = requested - 1u;
            while ((remaining & 0x80000000u) == 0) {
                initialize_native_render_entry_00bebf00(
                    reinterpret_cast<void*>(cursor), &actual_one_00d7a24c);
                cursor += 0x28u;
                --remaining;
            }
        }
        word(actual_header) = reinterpret_cast<Word>(allocation);
        word(actual_header, 8) = requested;
        cleanup.armed = false;
        return;
    }
    word(actual_header, 8) = requested;
}

void* publish_native_render_entry_cache_base_00bec590(
    void* actual_owner, NativeRenderEntryCacheContext& context) {
    ProfileCleanup base{actual_owner}; // Native state0 before profile.
    word(actual_owner) = 0x00d68cbcu;
    auto* const section = first_section(context);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    GuardCleanup guard_cleanup{guard};
    context.actual_cache_0108fe88 = actual_owner;
    void* const manager = get_native_singleton_manager_00415350(context.actual_manager_01090aa0);
    void* const current = context.actual_cache_0108fe88;
    register_native_singleton_object_00bd0c30(manager, nullptr, current);
    if (section) {
        word(section, 0x18) = word(section, 0x18) - 1u;
        LeaveCriticalSection(section); // state1 remains armed through Leave.
    }
    guard_cleanup.armed = false;
    base.armed = false;
    return actual_owner;
}

void destroy_native_render_entry_cache_base_00bec630(
    void* actual_owner, NativeRenderEntryCacheContext& context) {
    word(actual_owner) = 0x00d68cbcu;
    ProfileCleanup base{actual_owner}; // Native state0 after profile.
    auto* const section = first_section(context);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    GuardCleanup guard_cleanup{guard};
    void* const manager = get_native_singleton_manager_00415350(context.actual_manager_01090aa0);
    void* const current = context.actual_cache_0108fe88;
    unregister_native_singleton_object_00bcfca0(manager, nullptr, current);
    context.actual_cache_0108fe88 = nullptr;
    if (section) {
        word(section, 0x18) = word(section, 0x18) - 1u;
        LeaveCriticalSection(section);
    }
    guard_cleanup.armed = false;
    destroy_native_generic_singleton_base_00412430(actual_owner);
    base.armed = false;
}

void* delete_native_render_entry_cache_base_00bec6f0(void* actual_owner,
    Word flags, NativeRenderEntryCacheContext& context) {
    destroy_native_render_entry_cache_base_00bec630(actual_owner, context);
    if (flags & 1u) singleton_lifetime_free(actual_owner);
    return actual_owner;
}

void* construct_native_render_entry_cache_00bec870(
    void* actual_owner, NativeRenderEntryCacheContext& context) {
    publish_native_render_entry_cache_base_00bec590(actual_owner, context);
    word(actual_owner) = 0x00d68cc0u;
    const Word old_count = word(actual_owner, 8);
    BaseCleanup base{actual_owner, context}; // Native state0 after old-count capture.
    word(actual_owner, 4) = 0;
    if (old_count != 0) {
        void* const current_data = reinterpret_cast<void*>(word(actual_owner, 4));
        word(actual_owner, 8) = 0;
        word(actual_owner, 0x0c) = 0;
        singleton_lifetime_free(current_data);
        word(actual_owner, 0x0c) = 0;
    }
    HeaderCleanup array{at(actual_owner, 4)}; // state2 -> free header -> state0 base.
    auto* const section = create_native_tracked_critical_section_00bd1860();
    word(actual_owner, 0x10) = reinterpret_cast<Word>(section);
    array.armed = false;
    base.armed = false;
    return actual_owner;
}

void destroy_native_render_entry_cache_00bec8e0(
    void* actual_owner, NativeRenderEntryCacheContext& context) {
    word(actual_owner) = 0x00d68cc0u;
    release_native_tracked_critical_section_0041cc80(
        reinterpret_cast<TrackedCriticalSection**>(at(actual_owner, 0x10)));
    singleton_lifetime_free(reinterpret_cast<void*>(word(actual_owner, 4)));
    destroy_native_render_entry_cache_base_00bec630(actual_owner, context);
}

void* delete_native_render_entry_cache_00bec910(void* actual_owner,
    Word flags, NativeRenderEntryCacheContext& context) {
    destroy_native_render_entry_cache_00bec8e0(actual_owner, context);
    if (flags & 1u) singleton_lifetime_free(actual_owner);
    return actual_owner;
}

void initialize_native_window_render_entry_cache_00bed1e8_fragment(
    NativeRenderEntryCacheContext& context) {
    void* const owner = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x14, 0x14});
    AllocationCleanup cleanup{owner};
    if (owner) construct_native_render_entry_cache_00bec870(owner, context);
    void* const current = context.actual_cache_0108fe88;
    void* const header = at(current, 4);
    cleanup.armed = false;
    resize_native_render_entry_array_00bec3e0(header, 10000, context.actual_one_00d7a24c);
}
} // namespace bsp
