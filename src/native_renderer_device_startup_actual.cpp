#include "bsp/native_renderer_device_startup_actual.hpp"
#include "bsp/native_renderer_reset_process.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_renderer_resource_release.hpp"
#include "bsp/native_renderer_default_surfaces.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_stream_frequency.hpp"
#include "bsp/native_renderer_reset_readiness.hpp"
#include "bsp/native_renderer_gamma.hpp"
#include "bsp/native_physical_pool_acquire.hpp"
#include "bsp/native_physical_buffer_owner.hpp"
#include <d3d9.h>
#include <mmsystem.h>
#include <cstddef>
#include <cstring>
#include <exception>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer startup requires MSVC Win32.
#endif
#pragma comment(lib, "winmm.lib")
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(D3DCAPS9) == 0x130);
static_assert(offsetof(D3DCAPS9, DevCaps) == 0x1c);
static_assert(offsetof(D3DCAPS9, VertexShaderVersion) == 0xc4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, [eax + edx] }
    __asm { mov value, eax }
    return value;
}
Word byte(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, byte ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
Word halfword(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { movzx eax, word ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov [eax + edx], ecx }
}
void put_byte(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov byte ptr [eax + edx], cl }
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(void* base, Word byte_offset) noexcept {
    return pointer(reinterpret_cast<Word>(base) + byte_offset);
}
struct ConstructorCleanup {
    void* volatile& slot;
    const NativePhysicalBufferOwnerContext& context;
    bool vertex, armed = true;
    ~ConstructorCleanup() noexcept {
        if (armed) {
            if (vertex) return_native_vertex_buffer_slot_00b49950(slot, context);
            else return_native_index_buffer_slot_00b49940(slot, context);
        }
    }
};
struct AspectGammaAccess {
    const volatile NativeRendererDeviceStartupSlots* slots;
    const volatile float* unsigned_bias;
    const volatile double* limit;
    const volatile Word* renderer_profile;
    const NativeRendererGammaContext* gamma;
};
// Keep the native x87 schedule and its actual outgoing float word together.
__declspec(naked) void __fastcall aspect_and_gamma(void*, const AspectGammaAccess*) {
    __asm {
        push ebx
        push esi
        push edi
        mov esi, ecx
        mov ebx, edx
        mov edi, [ebx]
        fild dword ptr [edi + 8]
        mov eax, [edi + 8]
        test eax, eax
        mov byte ptr [esi + 1d8ch], 1
        jge width_unsigned
        mov edx, [ebx + 4]
        fadd dword ptr [edx]
    width_unsigned:
        mov ecx, [edi + 0ch]
        fild dword ptr [edi + 0ch]
        test ecx, ecx
        jge height_unsigned
        mov edx, [ebx + 4]
        fadd dword ptr [edx]
    height_unsigned:
        fdivp st(1), st(0)
        mov edx, [ebx + 8]
        fld qword ptr [edx]
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        jbe ordinary_aspect
        mov eax, 1
        jmp aspect_ready
    ordinary_aspect:
        xor eax, eax
    aspect_ready:
        mov edx, [esi]
        cmp edx, 0d5f0a8h
        jne unsupported
        mov edx, [ebx + 0ch]
        fldz
        push ecx
        fstp dword ptr [esp]
        mov byte ptr [esi + 1a14h], al
        mov eax, [edx + 0f0h]
        cmp eax, 0b21960h
        jne unsupported
        lea eax, [esp]
        push dword ptr [ebx + 10h]
        push eax
        push esi
        call set_native_renderer_gamma_00b21960
        add esp, 10h
        pop edi
        pop esi
        pop ebx
        ret
    unsupported:
        _emit 0x0f
        _emit 0x0b
    }
}
} // namespace
void return_native_index_buffer_slot_00b49940(void* slot,
    const NativePhysicalBufferOwnerContext& context) {
    return_native_physical_buffer_slot_00b49500(context.actual_index_pool_0108fda8, slot);
}
void return_native_vertex_buffer_slot_00b49950(void* slot,
    const NativePhysicalBufferOwnerContext& context) {
    return_native_physical_buffer_slot_00b49500(context.actual_vertex_pool_0108fde0, slot);
}
void initialize_native_renderer_device_00b2aeb0(void* renderer,
    const volatile NativeRendererDeviceStartupSlots& slots,
    NativeRendererDeviceStartupContext& context) {
    auto& recreation = context.reset.recreation;
    auto& globals = context.default_surfaces.synchronization;
    void* const parameters = at(renderer, 0x1a28);
    std::memset(parameters, 0, 0x38);
    const Word format = word(&slots, 0x10);
    put(renderer, 0x1a34, word(&slots, 0x14));
    const Word window = word(&slots);
    put(renderer, 0x1a30, format);
    const Word multisample = word(&slots, 0x18);
    const Word height = word(&slots, 0x0c);
    put(renderer, 0x1a44, window);
    const Word width = word(&slots, 8);
    put(renderer, 0x1a38, multisample);
    const Word depth_format = word(&slots, 0x1c);
    put(renderer, 0x1a50, depth_format);
    put(renderer, 0x1a58, word(&slots, 0x24));
    void* const caps_d3d = pointer(word(renderer, 0x1990));
    put(renderer, 0x1a28, width);
    put(renderer, 0x1a4c, depth_format != 0);
    const Word interval = ((word(&slots, 0x20) << 31) ^ 0xffffffffu) & 0x80000000u;
    put(renderer, 0x1a5c, interval);
    D3DCAPS9 caps; // Original uninitialized stack output; ignored HRESULT.
    put(renderer, 0x1a2c, height);
    put(renderer, 0x1a3c, 0);
    put(renderer, 0x1a40, 1);
    put(renderer, 0x1a48, 1);
    put(renderer, 0x1a54, 2);
    put(renderer, 0x1a20, width);
    put(renderer, 0x1a24, height);
    using Caps = HRESULT (WINAPI*)(void*, UINT, D3DDEVTYPE, D3DCAPS9*);
    const auto get_caps = reinterpret_cast<Caps>(word(pointer(word(caps_d3d)), 0x38));
    (void)get_caps(caps_d3d, 0, D3DDEVTYPE_HAL, &caps);
    Word behavior = 0x20;
    if ((word(&caps, 0x1c) & 0x10000u) && halfword(&caps, 0xc4) >= 0x101u)
        behavior = 0x40;
    void* const create_d3d = pointer(word(renderer, 0x1990));
    using Create = HRESULT (WINAPI*)(void*, UINT, D3DDEVTYPE, HWND, DWORD,
        D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
    const auto create = reinterpret_cast<Create>(word(pointer(word(create_d3d)), 0x40));
    (void)create(create_d3d, 0, D3DDEVTYPE_HAL, reinterpret_cast<HWND>(word(&slots)),
        behavior | 4u, static_cast<D3DPRESENT_PARAMETERS*>(parameters),
        static_cast<IDirect3DDevice9**>(at(renderer, 0x1a10)));
    const Word windowed = byte(&slots, 4) == 0;
    put_byte(renderer, 0x1d8a, 0);
    put_byte(renderer, 0x1d8b, 1);
    put(renderer, 0x1a48, windowed);
    context.actual_render_thread_0108d4c8 = GetCurrentThreadId();
    (void)timeBeginPeriod(1);
    set_native_renderer_render_state_00b24460(renderer, 0xa1, word(&slots, 0x18) != 0, globals);
    capture_native_renderer_default_surfaces_00b238d0(renderer, &context.default_surfaces);
    initialize_native_renderer_default_states_00b26170(renderer, globals);
    for (Word stream = 0; stream < 4; ++stream)
        set_native_renderer_stream_frequency_00b24a40(renderer, stream, 1, globals);
    void* volatile raw_slot = acquire_native_vertex_buffer_slot_00b4b360(context.physical);
    void* owner;
    {
        ConstructorCleanup cleanup{raw_slot, context.physical, true};
        owner = raw_slot ? construct_native_physical_vertex_buffer_00b4bbb0(raw_slot) : nullptr;
        cleanup.armed = false;
    }
    put(renderer, 0x1974, reinterpret_cast<Word>(owner));
    void* device = pointer(word(renderer, 0x1a10));
    Word volatile vertex_output = 0;
    using Vertex = HRESULT (WINAPI*)(void*, UINT, DWORD, DWORD, D3DPOOL, void*, HANDLE*);
    const auto create_vertex = reinterpret_cast<Vertex>(word(pointer(word(device)), 0x68));
    (void)create_vertex(device, 0x1000000, 0x208, 0, D3DPOOL_DEFAULT,
        const_cast<Word*>(&vertex_output), nullptr);
    owner = pointer(word(renderer, 0x1974));
    const Word vertex_identity = word(owner);
    __assume(vertex_identity == 0x00d61e7cu);
    const Word captured_vertex = word(&vertex_output);
    const Word vertex_target = word(recreation.physical_profiles.pooled_vertex_00d61e7c, 0x14);
    __assume(vertex_target == 0x00b4c370u);
    attach_native_physical_vertex_buffer_00b4c370(owner,
        reinterpret_cast<IDirect3DVertexBuffer9*>(captured_vertex), 0x1000, 0x1000000, context.physical);
    using Release = ULONG (WINAPI*)(void*);
    void* temporary = pointer(word(&vertex_output));
    auto release = reinterpret_cast<Release>(word(pointer(word(temporary)), 8));
    (void)release(temporary);
    raw_slot = acquire_native_index_buffer_slot_00b4b350(context.physical);
    {
        ConstructorCleanup cleanup{raw_slot, context.physical, false};
        owner = raw_slot ? construct_native_physical_index_buffer_00b4bb60(raw_slot) : nullptr;
        cleanup.armed = false;
    }
    put(renderer, 0x1978, reinterpret_cast<Word>(owner));
    device = pointer(word(renderer, 0x1a10));
    Word volatile index_output = 0;
    using Index = HRESULT (WINAPI*)(void*, UINT, DWORD, D3DFORMAT, D3DPOOL, void*, HANDLE*);
    const auto create_index = reinterpret_cast<Index>(word(pointer(word(device)), 0x6c));
    (void)create_index(device, 0x100000, 0x208, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
        const_cast<Word*>(&index_output), nullptr);
    owner = pointer(word(renderer, 0x1978));
    const Word index_identity = word(owner);
    __assume(index_identity == 0x00d61e58u);
    const Word captured_index = word(&index_output);
    const Word index_target = word(recreation.physical_profiles.pooled_index_00d61e58, 0x14);
    __assume(index_target == 0x00b4c250u);
    attach_native_physical_index_buffer_00b4c250(owner,
        reinterpret_cast<IDirect3DIndexBuffer9*>(captured_index), 0x1000, 0x100000, context.physical);
    temporary = pointer(word(&index_output));
    release = reinterpret_cast<Release>(word(pointer(word(temporary)), 8));
    (void)release(temporary);
    const AspectGammaAccess gamma{&slots, &context.unsigned_bias_00ce3978,
        &context.widescreen_limit_00cf5750,
        recreation.release.actual_bindings.actual_renderer_profile_00d5f0a8, &recreation.gamma};
    aspect_and_gamma(renderer, &gamma);
    context.reset.actual_pending_0108d4b8 = 1; // EBX=1 at B2AF41; no later EBX/BL write.
    process_native_renderer_device_reset_00b2abd0(renderer, context.reset);
}
} // namespace bsp
