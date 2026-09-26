#pragma once

#include "bsp/native_render_pointer_arrays.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// Separately placed actual source+08..+1F payload, not a complete owner.
// Default construction begins these lifetimes without initializing bytes.
// The base/profile and separately live actual+04 atomic remain external.
// No owning destructor, second header/count or callable vtable is introduced.
struct NativeTextureSourcePayload {
    std::uint32_t selected_00;
    float rate_04;
    NativeRenderPointerArrayStorage children_08;
    std::uint8_t initialized_14;
    std::byte padding_15[3];
};
static_assert(sizeof(NativeTextureSourcePayload) == 0x18);
static_assert(alignof(NativeTextureSourcePayload) == 4);
static_assert(offsetof(NativeTextureSourcePayload, selected_00) == 0);
static_assert(offsetof(NativeTextureSourcePayload, rate_04) == 4);
static_assert(offsetof(NativeTextureSourcePayload, children_08) == 8);
static_assert(offsetof(NativeTextureSourcePayload, initialized_14) == 0x14);
static_assert(offsetof(NativeTextureSourcePayload, padding_15) == 0x15);
static_assert(std::is_standard_layout_v<NativeTextureSourcePayload>);
static_assert(std::is_trivially_default_constructible_v<NativeTextureSourcePayload>);
static_assert(std::is_trivially_destructible_v<NativeTextureSourcePayload>);

} // namespace bsp
