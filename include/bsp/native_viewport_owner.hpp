#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cstddef>
#include <cstdint>

#include "bsp/native_viewport_registry.hpp"

namespace bsp {
class D3D9StateCache;

// The actual native +08..+33 fields, including constructor-unwritten scissor
// bytes. Depth words are preserved by the owner/setters, but 00B26770 uses
// hardcoded depth 0..1 and does not read them.
struct NativeViewportFields {
    DWORD x, y, width, height;
    std::uint32_t depth_min_bits, depth_max_bits;
    std::uint8_t scissor_enabled;
    std::uint8_t preserved_21[3];
    RECT scissor;
};
struct NativeViewportOwner final {
    volatile std::uint32_t native_vtable_00;
    volatile long references_04;
    NativeViewportFields fields_08;
};
inline constexpr std::size_t kNativeViewportBytes = 0x34;
inline constexpr std::uint32_t kNativeViewportVtable = 0x00d5e5f8;
inline constexpr std::uint32_t kNativeViewportBaseVtable = 0x00ceb130;

// A stable caller-owned companion; references are to this SAME raw owner.
// Keep the view alive while a renderer borrows its address. It neither retains
// nor frees the owner, and must not be rebound or used after owner destruction.
struct NativeViewportView final {
    explicit NativeViewportView(NativeViewportOwner&) noexcept;
    NativeViewportView(const NativeViewportView&) = delete;
    NativeViewportView& operator=(const NativeViewportView&) = delete;
    NativeViewportOwner& owner;
    const DWORD& x;
    const DWORD& y;
    const DWORD& width;
    const DWORD& height;
    const std::uint8_t& scissor_enabled;
    const RECT& scissor;
};

// Native concrete renderer vtable00D5F0A8+30 -> 00B1FF60 returns renderer+1A14.
// These references project result+0C/+10 (actual renderer+1A20/+1A24). They
// are distinct storage from D3DPRESENT_PARAMETERS at renderer+1A28.
struct NativeViewportRendererParameters {
    const DWORD& width_0c;
    const DWORD& height_10;
};
class NativeViewportRendererAccess {
public:
    virtual ~NativeViewportRendererAccess() = default;
    // Resolve the exact captured renderer and its CURRENT virtual+30 binding
    // on every call. Return references to its actual parameter fields. A
    // callback may replace the shared renderer publication or mutate fields.
    // Throw an explicit binding error for unsupported/unbound renderers; never
    // synthesize dimensions, use device present sizes, or cache a prior result.
    virtual NativeViewportRendererParameters parameters_00b1ff60(
        D3D9StateCache& captured_renderer) = 0;
};
struct NativeViewportEnvironment {
    D3D9StateCache* const volatile& renderer_00f8d394;
    NativeViewportRendererAccess& renderer_access;
    const volatile std::uint32_t& one_bits_00d7a24c;
};

// New C++ ABI for native ECX=storage, EAX=same, RET. Requires aligned writable
// raw storage without a live owner. Preserves allocation preimages at +20..33
// until the successful final byte20=0. Initializes count1, origin0, sizes640/480,
// min-depth +0, max-depth live00D7A24C, then width and height through TWO separate
// renderer calls with a fresh global00F8D394 load for EACH. Exception unwinding
// restores only base vtable, as native funclet00CBCCB0. No automatic free here.
NativeViewportOwner* initialize_native_viewport_owner_00b1f850(
    void* storage, NativeViewportEnvironment&);
// Shared ordinary CRT allocation, 34h bytes; no zero initialization. Frees the
// allocation on constructor failure (the native caller's new-expression role).
NativeViewportOwner* allocate_native_viewport_owner(NativeViewportEnvironment&);
// Prepared host association only: validate the admission before native allocation,
// then register the successful owner without allocation before returning it.
// Takes the token before native callbacks. Constructor failure cancels its record;
// the caller must forget that cancelled storage after host quiescence.
NativeViewportOwner* allocate_native_viewport_owner(NativeViewportEnvironment&,
    NativeViewportRegistry::Admission&&);

// Concrete D5E5F8 profile only. These operate on the actual +04 count and
// BD30E0 -> B1F8F0(flag1) final-zero path, without an auxiliary reference count.
void retain_native_viewport_owner(NativeViewportOwner&) noexcept;
void release_native_viewport_owner(NativeViewportOwner&) noexcept;
void invoke_native_viewport_deleting_destructor_00bd30e0(NativeViewportOwner*) noexcept;
// Native ECX=owner, stack flags, EAX=original address, RET4. Set concrete then
// base vtable; free through the shared CRT iff flags&1. Flags0 ends the owner
// lifetime but leaves storage. Returned address may already be freed.
NativeViewportOwner* delete_native_viewport_owner_00b1f8f0(
    NativeViewportOwner*, std::uint32_t flags) noexcept;

// Native thiscall, stack pointer to two raw DWORDs, RET4. Forward read/store
// pairs preserve overlapping-source propagation; no pair snapshot/memmove.
void set_native_viewport_origin_00b1f920(NativeViewportOwner&, const void* xy) noexcept;
void set_native_viewport_dimensions_00b1f940(NativeViewportOwner&, const void* wh) noexcept;
// Raw float32 bits preserve MOVSS signaling NaNs and negative zero exactly.
// Native thiscall, one stack float word, RET4; no clamp or numeric conversion.
void set_native_viewport_min_depth_00b1f750(NativeViewportOwner&, std::uint32_t) noexcept;
void set_native_viewport_max_depth_00b1f760(NativeViewportOwner&, std::uint32_t) noexcept;

} // namespace bsp
