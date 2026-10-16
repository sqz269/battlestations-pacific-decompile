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

namespace bsp {
// Whole 00AD26D0..00AD26D8: MOV AX,[ECX+ECh]; RET (8 bytes/two starts).
// Borrow a live DWORD-aligned actual FrameBox; return its current WORD bits.
// No semantic GuiFrameBox overlay or signed conversion is used. Win32 fastcall
// carries the one argument in ECX and returns the bits in AX. Original address
// placement, caller adoption and runtime execution are not established here.
std::uint16_t __fastcall native_gui_frame_box_current_state_bits_00ad26d0(
    const void* actual_frame_box) noexcept;
} // namespace bsp

namespace bsp {
// Whole AD2750,69B/24 starts: Native ECX receiver, signed WORD stack index,
// EAX raw 2Ch record address, RET4. This two-argument C++ interface is cdecl.
// Borrow live aligned actual storage through any Source CRT handler. Capture
// F4 first; null skips F8. Otherwise compare the sign-extended index UNSIGNED
// against signed wrapped(F8-F4)/44. The genuine BF6713 Source gateway may
// throw/terminate/return; after a return, reload CURRENT F4 once and form the
// saved index's wrapping 32-bit record address without another range check.
// No record dereference, ownership, typed overlay or range_error substitution.
// The SDK handler and value-argument boundary do not prove Native private CRT,
// stack/fault timing, register ABI, original placement or caller adoption.
void* native_gui_frame_box_state_record_00ad2750(
    const void* actual_frame_box, std::int16_t index);
} // namespace bsp
