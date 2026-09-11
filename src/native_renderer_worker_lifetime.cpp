#include "bsp/native_renderer_worker_lifetime.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Renderer worker lifetime reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(TrackedCriticalSection) == 0x1c);
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);
static_assert(sizeof(NativeRendererWorkerLifetimeContext) == 4);

// Called with the original pushed 1Ch. Preserve the existing allocator's
// current malloc/new-handler/retry/throw service, with no enlarged projection.
__declspec(noinline) void* __cdecl allocate_worker_lock(std::uint32_t bytes) {
    return singleton_lifetime_allocate({
        SingletonAllocationKind::critical_section, bytes, bytes});
}

void destroy_header_prefix(void* headers, std::uint32_t completed,
    NativeRendererWorkerLifetimeContext* context) noexcept {
    auto* const first = static_cast<unsigned char*>(headers);
    while (completed != 0) {
        --completed;
        destroy_native_string_header_0041dd20(first + completed * 8u,
            *context->strings);
    }
}

// The two helpers specialize exactly five eight-byte headers. The completed
// prefix advances only after the element constructor returns. Native hardware
// faults and the compiler's SEH filter/terminate machinery are not this ABI.
__declspec(noinline) void initialize_five_worker_headers(void* headers,
    NativeRendererWorkerLifetimeContext* context) {
    std::uint32_t completed = 0;
    try {
        while (completed != 5) {
            initialize_native_string_header_00415270(
                static_cast<unsigned char*>(headers) + completed * 8u);
            ++completed;
        }
    } catch (...) {
        destroy_header_prefix(headers, completed, context);
        throw;
    }
}

__declspec(noinline) void __cdecl destroy_five_worker_headers(void* headers,
    NativeRendererWorkerLifetimeContext* context) noexcept {
    destroy_header_prefix(headers, 5, context);
}

template<class T> void store(void* owner, std::uint32_t offset, T value) {
    *reinterpret_cast<volatile T*>(
        static_cast<unsigned char*>(owner) + offset) = value;
}
} // namespace

__declspec(naked) void* __fastcall initialize_native_string_header_00415270(void*) {
    __asm {
        mov eax, ecx
        mov dword ptr [eax], 0
        mov dword ptr [eax + 4], 0
        ret
    }
}

__declspec(naked) TrackedCriticalSection*
create_native_tracked_critical_section_00bd1860() {
    __asm {
        push esi
        push 1Ch
        call allocate_worker_lock
        mov esi, eax
        add esp, 4
        test esi, esi
        jz empty_lock
        push esi
        call dword ptr [InitializeCriticalSection]
        mov dword ptr [esi + 18h], 0
        mov eax, esi
        pop esi
        ret
    empty_lock:
        xor eax, eax
        pop esi
        ret
    }
}

void* __fastcall construct_native_renderer_worker_00b5e270(void* owner,
    NativeRendererWorkerLifetimeContext* context) {
    auto* const headers = static_cast<unsigned char*>(owner) + 0x14;
    initialize_five_worker_headers(headers, context);
    store<std::uint32_t>(owner, 0x3c, 0);
    store<std::uint32_t>(owner, 0x40, 0);
    store<unsigned char>(owner, 0x44, 0);
    store<unsigned char>(owner, 0x45, 0);
    store<std::uint32_t>(owner, 0x50, 0);
    // Original outer EH state becomes zero only here, at B5E2B5.
    try {
        store<std::uint32_t>(owner, 0x00, 0);
        store<std::uint32_t>(owner, 0x04, 0);
        store<std::uint32_t>(owner, 0x08, 0);
        store<std::uint32_t>(owner, 0x0c, 0);
        store<std::uint32_t>(owner, 0x10, 0);
        store<TrackedCriticalSection*>(owner, 0x48,
            create_native_tracked_critical_section_00bd1860());
        store<TrackedCriticalSection*>(owner, 0x4c,
            create_native_tracked_critical_section_00bd1860());
    } catch (...) {
        destroy_five_worker_headers(headers, context);
        throw;
    }
    return owner;
}

__declspec(naked) void __fastcall stop_native_renderer_worker_00b5e050(void*) {
    __asm {
        push esi
        mov esi, ecx
        cmp dword ptr [esi + 50h], 0
        jz stop_return
        push ebx
        push ebp
        mov ebp, dword ptr [EnterCriticalSection]
        push edi
        mov edi, dword ptr [esi + 4Ch]
        push edi
        call ebp
        add dword ptr [edi + 18h], 1
        mov eax, dword ptr [esi + 4Ch]
        mov byte ptr [esi + 44h], 1
        add dword ptr [eax + 18h], -1
        push eax
        call dword ptr [LeaveCriticalSection]
        mov edi, edi
    wait_for_done:
        push 0Ah
        call dword ptr [Sleep]
        mov edi, dword ptr [esi + 4Ch]
        push edi
        call ebp
        add dword ptr [edi + 18h], 1
        mov eax, dword ptr [esi + 4Ch]
        mov bl, byte ptr [esi + 45h]
        add dword ptr [eax + 18h], -1
        push eax
        call dword ptr [LeaveCriticalSection]
        test bl, bl
        jz wait_for_done
        mov eax, dword ptr [esi + 50h]
        push eax
        call dword ptr [CloseHandle]
        pop edi
        pop ebp
        pop ebx
    stop_return:
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall destroy_native_renderer_worker_00b5e2f0(
    void*, NativeRendererWorkerLifetimeContext*) {
    __asm {
        push edx // fixed host context, outside the original four-register frame
        push ebx
        push ebp
        push esi
        push edi
        mov ebx, ecx
        call stop_native_renderer_worker_00b5e050
        mov esi, dword ptr [ebx + 48h]
        mov edi, dword ptr [LeaveCriticalSection]
        or ebp, -1
        test esi, esi
        jz second_lock
        cmp dword ptr [esi + 18h], 0
        jle delete_first
    leave_first:
        add dword ptr [esi + 18h], ebp
        push esi
        call edi
        cmp dword ptr [esi + 18h], 0
        jg leave_first
    delete_first:
        push esi
        call dword ptr [DeleteCriticalSection]
        push esi
        call singleton_lifetime_free
        add esp, 4
    second_lock:
        mov esi, dword ptr [ebx + 4Ch]
        test esi, esi
        jz destroy_headers
        cmp dword ptr [esi + 18h], 0
        jle delete_second
        lea ebx, [ebx]
    leave_second:
        add dword ptr [esi + 18h], ebp
        push esi
        call edi
        cmp dword ptr [esi + 18h], 0
        jg leave_second
    delete_second:
        push esi
        call dword ptr [DeleteCriticalSection]
        push esi
        call singleton_lifetime_free
        add esp, 4
    destroy_headers:
        mov eax, dword ptr [esp + 10h]
        push eax
        add ebx, 14h
        push ebx
        call destroy_five_worker_headers
        add esp, 8
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 4
        ret
    }
}
}
