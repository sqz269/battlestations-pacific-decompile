#include "bsp/native_platform_construction.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* owner, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(owner) + offset);
}
volatile unsigned char& byte(void* owner, Word offset) noexcept {
    return *reinterpret_cast<volatile unsigned char*>(reinterpret_cast<Word>(owner) + offset);
}
__declspec(noinline) void* __cdecl allocate_text_queue_node_bytes(Word size) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
}
struct CompletedTextQueueMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedTextQueueMessage() noexcept {
        native_legacy_sbo_string_destroy_004072d0(storage);
    }
};
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
void unwind_base(void* owner, NativePlatformConstructionContext& context, bool window) noexcept {
    __try {
        if (window) destroy_native_window_platform_base_00be2b10(owner, context);
        else destroy_native_platform_base_00be2a00(owner, context);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
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
struct PlatformCleanup {
    void* owner;
    NativePlatformConstructionContext& context;
    bool window;
    bool armed = true;
    ~PlatformCleanup() noexcept { if (armed) unwind_base(owner, context, window); }
};
CRITICAL_SECTION* first_section(NativePlatformConstructionContext& context) {
    void* const manager = get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    return reinterpret_cast<CRITICAL_SECTION*>(word(manager, 0x10));
}
} // namespace

void* publish_native_platform_base_00be2960(
    void* actual_owner, NativePlatformConstructionContext& context) {
    ProfileCleanup base{actual_owner}; // state0 BEFORE the profile store.
    word(actual_owner) = 0x00d68604u;
    auto* const section = first_section(context);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    GuardCleanup locked{guard}; // state1 only after Enter/depth succeeds.
    context.actual_platform_0109cf04 = actual_owner;
    void* const manager = get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    void* const current = context.actual_platform_0109cf04;
    register_native_singleton_object_00bd0c30(manager, nullptr, current);
    if (section) {
        word(section, 0x18) = word(section, 0x18) - 1u;
        LeaveCriticalSection(section); // state1 remains armed through Leave.
    }
    locked.armed = false;
    base.armed = false;
    return actual_owner;
}

void destroy_native_platform_base_00be2a00(
    void* actual_owner, NativePlatformConstructionContext& context) {
    word(actual_owner) = 0x00d68604u;
    ProfileCleanup base{actual_owner}; // state0 AFTER the profile store.
    auto* const section = first_section(context);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    GuardCleanup locked{guard};
    void* const manager = get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    void* const current = context.actual_platform_0109cf04;
    unregister_native_singleton_object_00bcfca0(manager, nullptr, current);
    context.actual_platform_0109cf04 = nullptr;
    if (section) {
        word(section, 0x18) = word(section, 0x18) - 1u;
        LeaveCriticalSection(section);
    }
    locked.armed = false;
    destroy_native_generic_singleton_base_00412430(actual_owner);
    base.armed = false;
}

void* construct_native_window_platform_base_00be2ac0(
    void* actual_owner, NativePlatformConstructionContext& context) {
    publish_native_platform_base_00be2960(actual_owner, context);
    // BE2AC8..BE2B07: keep the x87 integer operands and interleaved stores.
    __asm {
        mov esi, actual_owner
        xor eax, eax
        mov dword ptr [esi], 00d68608h
        mov [esi + 4], eax
        mov [esi + 8], eax
        fild dword ptr [esi + 24h]
        mov byte ptr [esi + 0ch], al
        mov byte ptr [esi + 0dh], al
        fidiv dword ptr [esi + 28h]
        mov byte ptr [esi + 14h], al
        mov [esi + 1ch], eax
        mov [esi + 20h], eax
        mov byte ptr [esi + 2ch], al
        mov dword ptr [esi + 18h], 20h
        mov dword ptr [esi + 24h], 280h
        mov dword ptr [esi + 28h], 1e0h
        mov eax, esi
        fstp dword ptr [esi + 10h]
    }
    return actual_owner;
}

void destroy_native_window_platform_base_00be2b10(
    void* actual_owner, NativePlatformConstructionContext& context) {
    word(actual_owner) = 0x00d68608u;
    void* const data = reinterpret_cast<void*>(word(actual_owner, 8));
    PlatformCleanup base{actual_owner, context, false}; // capture precedes state0.
    if (data) {
        const Word size = word(actual_owner, 4) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.strings.actual_published_01090aa8,
            context.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, size,
            context.strings.actual_small_returns_disabled_01090aa4);
    }
    base.armed = false;
    destroy_native_platform_base_00be2a00(actual_owner, context);
}

void* allocate_native_text_queue_sentinel_00bec710() {
    void* const allocated = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, 0x0c});
    // Preserve both native tests, DWORD wrapping, and the untouched payload.
    __asm {
        mov eax, allocated
        test eax, eax
        jz previous_link
        mov [eax], eax
    previous_link:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz links_done
        mov [ecx], eax
    links_done:
    }
    return allocated;
}

void* construct_native_win32_platform_00becda0(
    void* actual_owner, NativePlatformConstructionContext& context) {
    construct_native_window_platform_base_00be2ac0(actual_owner, context);
    PlatformCleanup base{actual_owner, context, true};
    word(actual_owner) = 0x00d68cc4u;
    void* const sentinel = allocate_native_text_queue_sentinel_00bec710();
    word(actual_owner, 0x178) = reinterpret_cast<Word>(sentinel);
    word(actual_owner, 0x17c) = 0;
    byte(actual_owner, 0x0c) = 0;
    byte(actual_owner, 0x0d) = 0;
    word(actual_owner, 0x30) = 0;
    byte(actual_owner, 0x40) = 0;
    byte(actual_owner, 0x41) = 0;
    byte(actual_owner, 0x42) = 0;
    byte(actual_owner, 0x43) = 0;
    byte(actual_owner, 0x44) = 0;
    word(actual_owner, 0x48) = 0;
    byte(actual_owner, 0x170) = 0;
    byte(actual_owner, 0x181) = 0;
    byte(actual_owner, 0x180) = 0;
    base.armed = false;
    return actual_owner;
}

__declspec(naked) void __fastcall destroy_native_text_queue_00bec730(
    void*, void*) noexcept {
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esi + 4]
        mov eax, dword ptr [ecx]
        mov dword ptr [ecx], ecx
        mov ecx, dword ptr [esi + 4]
        mov dword ptr [ecx + 4], ecx
        cmp eax, dword ptr [esi + 4]
        mov dword ptr [esi + 8], 0
        je free_sentinel
        push edi
        // Original 3-byte alignment LEA ECX,[ECX+0].
        _emit 08dh
        _emit 049h
        _emit 000h
    free_node:
        mov edi, dword ptr [eax]
        push eax
        call singleton_lifetime_free
        add esp, 4
        cmp edi, dword ptr [esi + 4]
        mov eax, edi
        jne free_node
        pop edi
    free_sentinel:
        mov eax, dword ptr [esi + 4]
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
        pop esi
        ret
    }
}

void destroy_native_win32_platform_00beca00(
    void* actual_owner, NativePlatformConstructionContext& context) {
    word(actual_owner) = 0x00d68cc4u;
    (void)SystemParametersInfoA(0x11, 1, nullptr, 0);
    destroy_native_text_queue_00bec730(
        reinterpret_cast<void*>(reinterpret_cast<Word>(actual_owner) + 0x174u),
        nullptr);
    destroy_native_window_platform_base_00be2b10(actual_owner, context);
}

void* delete_native_win32_platform_00bece30(
    void* actual_owner, std::uint32_t flags,
    NativePlatformConstructionContext& context) {
    destroy_native_win32_platform_00beca00(actual_owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(actual_owner);
    return actual_owner;
}

__declspec(naked) void* __stdcall create_native_text_queue_node_00bec7b0(
    void*, void*, const void*) {
    __asm {
        push 0ch
        call allocate_text_queue_node_bytes
        add esp, 4
        test eax, eax
        jz previous_field
        mov ecx, dword ptr [esp + 4]
        mov dword ptr [eax], ecx
    previous_field:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz payload_field
        mov edx, dword ptr [esp + 8]
        mov dword ptr [ecx], edx
    payload_field:
        lea ecx, [eax + 8]
        test ecx, ecx
        jz done
        mov edx, dword ptr [esp + 0ch]
        mov dx, word ptr [edx]
        mov word ptr [ecx], dx
    done:
        ret 0ch
    }
}

std::uint32_t __fastcall increase_native_text_queue_count_00bed290(
    void* actual_queue, void*, std::uint32_t increment) {
    const Word captured = word(actual_queue, 8);
    if (Word{0x7fffffffu} - captured < increment) {
        NativeLegacySboStringStorage message;
        message.capacity_18 = 15;
        message.length_14 = 0;
        message.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(
            message, "list<T> too long", 16);
        // CC74A0 destroys the completed message only after state0 is armed.
        const CompletedTextQueueMessage completed{message};
        throw NativeAliasListLengthError{message};
    }
    const Word updated = captured + increment;
    word(actual_queue, 8) = updated;
    return updated;
}

__declspec(naked) void __fastcall append_native_text_queue_00bed370(
    void*, void*, const void*) {
    __asm {
        mov eax, dword ptr [esp + 4]
        push ebx
        push esi
        push edi
        mov esi, ecx
        mov edi, dword ptr [esi + 4]
        mov ecx, dword ptr [edi + 4]
        push eax
        push ecx
        push edi
        mov ecx, esi
        call create_native_text_queue_node_00bec7b0
        push 1
        mov ecx, esi
        mov ebx, eax
        call increase_native_text_queue_count_00bed290
        mov dword ptr [edi + 4], ebx
        mov edx, dword ptr [ebx + 4]
        pop edi
        pop esi
        mov dword ptr [edx], ebx
        pop ebx
        ret 4
    }
}
} // namespace bsp
