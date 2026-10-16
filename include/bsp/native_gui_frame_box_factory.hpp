#pragma once
#include <cstdint>

namespace bsp {
// Borrowed live cells from the SAME genuine native scalar/provider domain.
// No values are supplied implicitly, copied into a default profile, or owned.
struct NativeGuiFrameBoxDefaultBindings {
    const volatile std::uint32_t& actual_one_00d7a24c;
    const volatile std::uint32_t& actual_frame_x_00d5d11c;
    const volatile std::uint32_t& actual_frame_y_00d5d10c;
};

// Whole AD2600: native ECX actual destination, EAX same destination, RET.
// This is a new Source binding API, not a binary-compatible native entry.
// Caller owns fresh DWORD-aligned storage covering +00..+11B and keeps the
// actual scalar cells alive, disjoint from that storage, and externally
// serialized with this operation. Bindings metadata stays live/stable,
// disjoint from destination/callee scratch. Uses real AA9390(type18); its
// existing allocation/failure contract applies. No extra cleanup, slot return, or
// replay/retirement policy is introduced if that base constructor throws.
// X is captured after the base and before the derived vptr; Y only after
// both X stores. Values are copied as DWORD bits, including signaling NaNs.
// Preserves +EE..+F3 (including vector allocator/proxy), +101..+103, every
// unwritten base byte, and any caller-owned hidden pool word at +11C.
void* construct_native_gui_frame_box_default_00ad2600(
    void* actual_destination, NativeGuiFrameBoxDefaultBindings& bindings);
} // namespace bsp
