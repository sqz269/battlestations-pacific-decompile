#include "bsp/native_renderer_capability_nested_arrays.hpp"
#include "bsp/native_renderer_capability_array_reserves.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Capability nested arrays require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uint32_t>(value);
}
void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}
std::uint32_t load(const void* base, std::uint32_t byte_offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(address(base) + byte_offset);
}
void store(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(address(base) + byte_offset) = value;
}
__declspec(noinline) void* __cdecl allocate_nested_array(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
__declspec(noinline) void __cdecl free_nested_array(void* allocation) noexcept {
    singleton_lifetime_free(allocation);
}

// Fixed specialization of CBD310's saved-state reads and its actual RET-only
// 401130 target. This does not reconstruct a generic CRT vector cleanup helper.
struct OuterSavedState {
    volatile std::uint32_t fresh;
    volatile std::uint32_t completed;
    volatile std::uint32_t current;
    volatile std::int32_t state;
};
static_assert(sizeof(OuterSavedState) == 16);
static_assert(offsetof(OuterSavedState, completed) == 4);
static_assert(offsetof(OuterSavedState, current) == 8);
__declspec(naked) void __cdecl finish_failed_header(void*, void*) {
    __asm { ret }
}
__declspec(naked) void __fastcall run_failed_header_action(OuterSavedState*) {
    __asm {
        mov eax, dword ptr [ecx + 4]
        imul eax, eax, 0Ch
        add eax, dword ptr [ecx]
        push eax
        mov ecx, dword ptr [ecx + 8]
        push ecx
        call finish_failed_header
        add esp, 8
        ret
    }
}

__declspec(noinline) void __cdecl reserve_headers_core(
    void* header, volatile std::int32_t* actual_request_slot) {
    OuterSavedState saved;
    saved.state = -1;
    auto capacity = *actual_request_slot;
    if (capacity < 1) {
        *actual_request_slot = 1;
        capacity = *actual_request_slot;
    }
    if (static_cast<std::int32_t>(load(header, 8)) >= capacity) return;
    try {
        void* const allocated = allocate_nested_array(
            static_cast<std::uint32_t>(capacity) * 12u);
        std::uint32_t index = 0;
        const auto initial_count = static_cast<std::int32_t>(load(header, 4));
        saved.fresh = address(allocated);
        saved.completed = index;
        if (initial_count > 0) {
            bool more;
            do {
                const auto fresh = saved.fresh;
                const auto byte_offset = index * 12u;
                const auto child = fresh + byte_offset;
                saved.current = child;
                saved.state = 0;
                if (child != 0) {
                    const auto old_base = load(header, 0);
                    void* const source_child = pointer(old_base + byte_offset);
                    store(pointer(child), 0, 0);
                    store(pointer(child), 4, 0);
                    store(pointer(child), 8, 0);
                    copy_native_capability_records_00b25e60(
                        pointer(child), 0, source_child);
                }
                ++index;
                more = static_cast<std::int32_t>(index) <
                    static_cast<std::int32_t>(load(header, 4));
                saved.state = -1;
                saved.completed = index;
            } while (more);
        }
        index = 0;
        if (static_cast<std::int32_t>(load(header, 4)) > 0) {
            std::uint32_t byte_offset = 0;
            do {
                void* const child = pointer(load(header, 0) + byte_offset);
                resize_native_capability_records_00b23120(child, 0, 0);
                free_nested_array(pointer(load(child, 0)));
                ++index;
                byte_offset += 12u;
            } while (static_cast<std::int32_t>(index) <
                static_cast<std::int32_t>(load(header, 4)));
            capacity = *actual_request_slot;
        }
        free_nested_array(pointer(load(header, 0)));
        const auto fresh = saved.fresh;
        store(header, 0, fresh);
        store(header, 8, static_cast<std::uint32_t>(capacity));
    } catch (...) {
        if (saved.state == 0) run_failed_header_action(&saved);
        throw;
    }
}
} // namespace

__declspec(naked) void __fastcall reserve_native_capability_headers_00b29d40(
    void*, std::uint32_t, std::int32_t) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push ecx
        call reserve_headers_core
        add esp, 8
        ret 4
    }
}

__declspec(naked) void __fastcall resize_native_capability_records_00b23120(
    void*, std::uint32_t, std::int32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        cmp edi, dword ptr [esi + 8]
        jle l_b23133
        push edi
        call reserve_native_capability_records_00b22b30
    l_b23133:
        mov eax, dword ptr [esi + 4]
        cmp eax, edi
        jge l_b23165
        lea ecx, [eax + eax*2]
        add ecx, ecx
        mov edx, edi
        push ebx
        add ecx, ecx
        sub edx, eax
        xor ebx, ebx
    l_b23148:
        mov eax, dword ptr [esi]
        add eax, ecx
        je l_b2315c
        mov dword ptr [eax], ebx
        mov byte ptr [eax + 4], bl
        mov byte ptr [eax + 5], bl
        mov byte ptr [eax + 6], bl
        mov byte ptr [eax + 7], bl
    l_b2315c:
        add ecx, 0ch
        sub edx, 1
        jne l_b23148
        pop ebx
    l_b23165:
        cmp edi, dword ptr [esi + 4]
        jge l_b23178
        or eax, 0ffffffffh
        // Preserve original three-byte LEA ECX,[ECX+0].
        _emit 08dh
        _emit 049h
        _emit 000h
    l_b23170:
        add dword ptr [esi + 4], eax
        cmp edi, dword ptr [esi + 4]
        jl l_b23170
    l_b23178:
        mov dword ptr [esi + 4], edi
        pop edi
        pop esi
        ret 4
    }
}

__declspec(naked) void* __fastcall copy_native_capability_records_00b25e60(
    void*, std::uint32_t, void*) {
    __asm {
        push ebx
        push ebp
        push esi
        xor ebp, ebp
        push ebp
        mov esi, ecx
        call resize_native_capability_records_00b23120
        mov ebx, dword ptr [esp + 010h]
        mov eax, dword ptr [ebx + 4]
        push eax
        mov ecx, esi
        call reserve_native_capability_records_00b22b30
        cmp dword ptr [ebx + 4], ebp
        mov dword ptr [esp + 010h], ebp
        jle l_b25edd
        push edi
    l_b25e86:
        mov edi, dword ptr [ebx]
        mov eax, dword ptr [esi + 8]
        add edi, ebp
        cmp dword ptr [esi + 4], eax
        jne l_b25ea6
        add eax, eax
        cmp eax, 1
        jg l_b25e9e
        mov eax, 1
    l_b25e9e:
        push eax
        mov ecx, esi
        call reserve_native_capability_records_00b22b30
    l_b25ea6:
        mov eax, dword ptr [esi + 4]
        mov edx, dword ptr [esi]
        lea ecx, [eax + eax*2]
        lea eax, [edx + ecx*4]
        test eax, eax
        je l_b25ec5
        mov ecx, dword ptr [edi]
        mov dword ptr [eax], ecx
        mov edx, dword ptr [edi + 4]
        mov dword ptr [eax + 4], edx
        mov ecx, dword ptr [edi + 8]
        mov dword ptr [eax + 8], ecx
    l_b25ec5:
        mov eax, dword ptr [esp + 014h]
        add dword ptr [esi + 4], 1
        add eax, 1
        add ebp, 0ch
        cmp eax, dword ptr [ebx + 4]
        mov dword ptr [esp + 014h], eax
        jl l_b25e86
        pop edi
    l_b25edd:
        mov eax, esi
        pop esi
        pop ebp
        pop ebx
        ret 4
    }
}

__declspec(naked) void __fastcall resize_native_capability_headers_00b2ae20(
    void*, std::uint32_t, std::int32_t) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8]
        push esi
        mov esi, ecx
        cmp ebx, dword ptr [esi + 8]
        push edi
        jle l_b2ae34
        push ebx
        call reserve_native_capability_headers_00b29d40
    l_b2ae34:
        mov eax, dword ptr [esi + 4]
        cmp eax, ebx
        jge l_b2ae5e
        lea ecx, [eax + eax*2]
        add ecx, ecx
        mov edx, ebx
        add ecx, ecx
        sub edx, eax
    l_b2ae46:
        mov eax, dword ptr [esi]
        add eax, ecx
        je l_b2ae56
        xor edi, edi
        mov dword ptr [eax], edi
        mov dword ptr [eax + 4], edi
        mov dword ptr [eax + 8], edi
    l_b2ae56:
        add ecx, 0ch
        sub edx, 1
        jne l_b2ae46
    l_b2ae5e:
        cmp ebx, dword ptr [esi + 4]
        jge l_b2ae8b
    l_b2ae63:
        add dword ptr [esi + 4], -1
        mov eax, dword ptr [esi + 4]
        mov ecx, dword ptr [esi]
        lea eax, [eax + eax*2]
        lea edi, [ecx + eax*4]
        push 0
        mov ecx, edi
        call resize_native_capability_records_00b23120
        mov edx, dword ptr [edi]
        push edx
        call free_nested_array
        add esp, 4
        cmp ebx, dword ptr [esi + 4]
        jl l_b2ae63
    l_b2ae8b:
        pop edi
        mov dword ptr [esi + 4], ebx
        pop esi
        pop ebx
        ret 4
    }
}

} // namespace bsp
