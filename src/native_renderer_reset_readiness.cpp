#include "bsp/native_renderer_reset_readiness.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer reset readiness requires MSVC Win32 assembly.
#endif

namespace bsp {
namespace {
// Original B49294/B493C4 contain addresses of these four local case labels.
// These are link-time address relocations; no runtime table initializer or
// replacement pool policy participates in the reconstructed bodies.
const unsigned char* const index_pool_targets[] = {
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_index_buffer_00b49180) + 0x1a,
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_index_buffer_00b49180) + 0x24,
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_index_buffer_00b49180) + 0x2b,
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_index_buffer_00b49180) + 0x32,
};
const unsigned char* const vertex_pool_targets[] = {
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_vertex_buffer_00b492b0) + 0x1a,
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_vertex_buffer_00b492b0) + 0x24,
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_vertex_buffer_00b492b0) + 0x2b,
    reinterpret_cast<const unsigned char*>(&recreate_native_physical_vertex_buffer_00b492b0) + 0x32,
};

// Volatile raw 32-bit accesses retain external mutation boundaries. Native
// object addresses, not semantic D3D9BufferBinding companion objects, are used.
__forceinline std::uint32_t word(const void* p, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uint32_t>(p) + offset);
}
__forceinline unsigned char byte(const void* p, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(
        reinterpret_cast<std::uint32_t>(p) + offset);
}
__forceinline const volatile std::uint32_t* profile(std::uint32_t identity,
    const NativeRendererResetReadinessProfiles& profiles) noexcept {
    switch (identity) {
    case 0x00d61e58: return profiles.pooled_index_00d61e58;
    case 0x00d61e7c: return profiles.pooled_vertex_00d61e7c;
    default: __assume(0); // Explicit two-profile source domain, not native check.
    }
}
__forceinline void invoke(std::uint32_t captured_target, void* wrapper,
    void* device) {
    switch (captured_target) {
    case 0x00b49180:
        recreate_native_physical_index_buffer_00b49180(wrapper, nullptr, device);
        return;
    case 0x00b492b0:
        recreate_native_physical_vertex_buffer_00b492b0(wrapper, nullptr, device);
        return;
    default: __assume(0); // The borrowed immutable original selectors only.
    }
}
} // namespace

// Full B1FD90[70], original ECX renderer / RET. This source interface adds the
// fixed profile context in EDX. B2AEB0's B2B067..B2B11A producer calls pooled
// B4BBB0/B4BB60 and stores the resulting wrappers at +1974/+1978. Private
// D61E10/D61E34 and other virtual implementations remain outside this domain.
void __fastcall restore_native_renderer_dynamic_buffers_00b1fd90(
    void* renderer, const NativeRendererResetReadinessProfiles& profiles) {
    if (byte(renderer, 0x1d8c) != 0) return;
    if (byte(renderer, 0x1d8a) != 0) return;
    void* wrapper = reinterpret_cast<void*>(word(renderer, 0x1974));
    void* device = reinterpret_cast<void*>(word(renderer, 0x1a10));
    *reinterpret_cast<volatile unsigned char*>(
        reinterpret_cast<std::uint32_t>(renderer) + 0x1d8c) = 1;
    const volatile std::uint32_t* table = profile(word(wrapper, 0), profiles);
    std::uint32_t target = table[8];
    invoke(target, wrapper, device);
    wrapper = reinterpret_cast<void*>(word(renderer, 0x1978));
    table = profile(word(wrapper, 0), profiles);
    device = reinterpret_cast<void*>(word(renderer, 0x1a10));
    target = table[8];
    invoke(target, wrapper, device);
}

// Full 00B49180[276]. ECX raw wrapper, stacked device/output cell, RET4.
// HRESULT is deliberately ignored; every COM call and slot reload is native.
__declspec(naked) void __fastcall recreate_native_physical_index_buffer_00b49180(
    void*, void*, void*) {
    __asm {
        push esi
        push edi
        mov edi, ecx
        mov ecx, dword ptr [edi + 014h]
        mov eax, ecx
        and eax, 0fh
        xor edx, edx
        cmp eax, 3
        ja short index_00b491b9
        jmp dword ptr [eax*4 + index_pool_targets]
        xor esi, esi
        or ecx, 010000h
        jmp short index_00b491bd
        mov esi, 1
        jmp short index_00b491bd
        mov esi, 2
        jmp short index_00b491bd
        mov esi, 3
        jmp short index_00b491bd
    index_00b491b9:
        mov esi, dword ptr [esp + 0ch]
    index_00b491bd:
        test cl, 010h
        je short index_00b491c7
        mov edx, 1
    index_00b491c7:
        mov eax, ecx
        and eax, 0f00h
        cmp eax, 0300h
        ja short index_00b491f7
        je short index_00b491f2
        cmp eax, 0100h
        je short index_00b491ed
        cmp eax, 0200h
        jne short index_00b49213
        or edx, 04000h
        jmp short index_00b49213
    index_00b491ed:
        or edx, 2
        jmp short index_00b49213
    index_00b491f2:
        or edx, 040h
        jmp short index_00b49213
    index_00b491f7:
        cmp eax, 0400h
        je short index_00b4920d
        cmp eax, 0500h
        jne short index_00b49213
        or edx, 080h
        jmp short index_00b49213
    index_00b4920d:
        or edx, 0100h
    index_00b49213:
        mov eax, ecx
        and eax, 0f000h
        cmp eax, 01000h
        jne short index_00b49227
        or edx, 0200h
    index_00b49227:
        and ecx, 0f0000h
        cmp ecx, 010000h
        jne short index_00b49238
        or edx, 8
    index_00b49238:
        mov eax, dword ptr [esp + 0ch]
        mov ecx, dword ptr [eax]
        push ebx
        push 0
        lea ebx, [esp + 014h]
        push ebx
        push esi
        push 065h
        push edx
        mov edx, dword ptr [edi + 018h]
        push edx
        push eax
        mov eax, dword ptr [ecx + 06ch]
        call eax
        mov esi, dword ptr [edi + 028h]
        mov eax, dword ptr [esp + 010h]
        cmp esi, eax
        pop ebx
        je short index_00b49283
        test eax, eax
        mov dword ptr [edi + 028h], eax
        je short index_00b49273
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 4]
        push eax
        call edx
        mov eax, dword ptr [esp + 0ch]
    index_00b49273:
        test esi, esi
        je short index_00b49283
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [eax + 8]
        push esi
        call ecx
        mov eax, dword ptr [esp + 0ch]
    index_00b49283:
        test eax, eax
        pop edi
        pop esi
        je short index_00b49291
        mov edx, dword ptr [eax]
        push eax
        mov eax, dword ptr [edx + 8]
        call eax
    index_00b49291:
        ret 4
    }
}

// Full 00B492B0[276]. ECX raw wrapper, stacked device/output cell, RET4.
// HRESULT is deliberately ignored; every COM call and slot reload is native.
__declspec(naked) void __fastcall recreate_native_physical_vertex_buffer_00b492b0(
    void*, void*, void*) {
    __asm {
        push esi
        push edi
        mov edi, ecx
        mov ecx, dword ptr [edi + 014h]
        mov eax, ecx
        and eax, 0fh
        xor edx, edx
        cmp eax, 3
        ja short vertex_00b492e9
        jmp dword ptr [eax*4 + vertex_pool_targets]
        xor esi, esi
        or ecx, 010000h
        jmp short vertex_00b492ed
        mov esi, 1
        jmp short vertex_00b492ed
        mov esi, 2
        jmp short vertex_00b492ed
        mov esi, 3
        jmp short vertex_00b492ed
    vertex_00b492e9:
        mov esi, dword ptr [esp + 0ch]
    vertex_00b492ed:
        test cl, 010h
        je short vertex_00b492f7
        mov edx, 1
    vertex_00b492f7:
        mov eax, ecx
        and eax, 0f00h
        cmp eax, 0300h
        ja short vertex_00b49327
        je short vertex_00b49322
        cmp eax, 0100h
        je short vertex_00b4931d
        cmp eax, 0200h
        jne short vertex_00b49343
        or edx, 04000h
        jmp short vertex_00b49343
    vertex_00b4931d:
        or edx, 2
        jmp short vertex_00b49343
    vertex_00b49322:
        or edx, 040h
        jmp short vertex_00b49343
    vertex_00b49327:
        cmp eax, 0400h
        je short vertex_00b4933d
        cmp eax, 0500h
        jne short vertex_00b49343
        or edx, 080h
        jmp short vertex_00b49343
    vertex_00b4933d:
        or edx, 0100h
    vertex_00b49343:
        mov eax, ecx
        and eax, 0f000h
        cmp eax, 01000h
        jne short vertex_00b49357
        or edx, 0200h
    vertex_00b49357:
        and ecx, 0f0000h
        cmp ecx, 010000h
        jne short vertex_00b49368
        or edx, 8
    vertex_00b49368:
        mov eax, dword ptr [esp + 0ch]
        mov ecx, dword ptr [eax]
        push ebx
        push 0
        lea ebx, [esp + 014h]
        push ebx
        push esi
        push 0
        push edx
        mov edx, dword ptr [edi + 018h]
        push edx
        push eax
        mov eax, dword ptr [ecx + 068h]
        call eax
        mov esi, dword ptr [edi + 028h]
        mov eax, dword ptr [esp + 010h]
        cmp esi, eax
        pop ebx
        je short vertex_00b493b3
        test eax, eax
        mov dword ptr [edi + 028h], eax
        je short vertex_00b493a3
        mov ecx, dword ptr [eax]
        mov edx, dword ptr [ecx + 4]
        push eax
        call edx
        mov eax, dword ptr [esp + 0ch]
    vertex_00b493a3:
        test esi, esi
        je short vertex_00b493b3
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [eax + 8]
        push esi
        call ecx
        mov eax, dword ptr [esp + 0ch]
    vertex_00b493b3:
        test eax, eax
        pop edi
        pop esi
        je short vertex_00b493c1
        mov edx, dword ptr [eax]
        push eax
        mov eax, dword ptr [edx + 8]
        call eax
    vertex_00b493c1:
        ret 4
    }
}

// Full B20C50[31]. Only the GetFocus import operand is relocated.
__declspec(naked) std::uint32_t __fastcall native_platform_has_focus_00b20c50(
    const void*) noexcept {
    __asm {
        push esi
        mov esi, ecx
        cmp byte ptr [esi + 041h], 0
        je short focus_00b20c6b
        call GetFocus
        cmp eax, dword ptr [esi + 030h]
        jne short focus_00b20c6b
        mov eax, 1
        pop esi
        ret 
    focus_00b20c6b:
        xor eax, eax
        pop esi
        ret 
    }
}

} // namespace bsp
