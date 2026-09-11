#pragma once
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// Borrow original runtime storage. Pointer members are immutable; the actual
// synchronization bytes remain live. The readonly DWORD D7A24C is loaded with
// MOVSS for viewport MaxZ, preserving its original bits and FP behavior.
struct NativeRendererViewportClearContext final {
    NativeRendererSynchronizationGlobals* const synchronization_00;
    const volatile std::uint32_t* const viewport_one_04;
    NativeRendererViewportClearContext(NativeRendererSynchronizationGlobals& globals,
        const volatile std::uint32_t& actual_00d7a24c) noexcept
        : synchronization_00(&globals), viewport_one_04(&actual_00d7a24c) {}
};
static_assert(sizeof(NativeRendererViewportClearContext) == 8);
static_assert(offsetof(NativeRendererViewportClearContext, viewport_one_04) == 4);

// Four complete 4-byte raw leaves. ECX actual viewport; EAX address / AL byte;
// RET. No dereference for the three addresses, no bool normalization for AL.
const void* __fastcall native_viewport_origin_00b1f730(const void*) noexcept;
const void* __fastcall native_viewport_size_00b1f740(const void*) noexcept;
const void* __fastcall native_viewport_scissor_rect_00b1f7b0(const void*) noexcept;
std::uint8_t __fastcall native_viewport_scissor_enabled_00b1f7d0(const void*) noexcept;

// Full B26770[290], original ECX renderer / stack viewport / RET4. The new EDX
// context is explicit. Preserve the actual callee viewport argument word until
// after optional guard entry; a callback may mutate that word before its load.
// The renderer's +1904 publication is borrowed and has no ownership action.
void __fastcall bind_native_renderer_viewport_00b26770(void* actual_renderer,
    const NativeRendererViewportClearContext*, void* actual_viewport);

// Full B21430[171], original ECX renderer and six stacked args / RET18h.
// Flags are captured before entry; zero returns without reading context,
// depth, color or device. Nonzero reads depth from its ACTUAL BY-VALUE CALLEE
// ARGUMENT SLOT after guard entry. This is not a camera field or external float
// reference; the caller's source expression has already been evaluated.
// Cleanup arms only after outgoing arguments are prepared, immediately before
// COM Clear. No HRESULT branch; count+1BD4 increments only after normal return.
void __fastcall clear_native_renderer_00b21430(void* actual_renderer,
    const NativeRendererViewportClearContext*, std::uint32_t count,
    const void* rectangles, std::uint32_t flags, const void* color_word,
    float depth, std::uint32_t stencil);
} // namespace bsp
