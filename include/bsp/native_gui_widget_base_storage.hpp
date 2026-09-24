#pragma once
#include "bsp/native_gui_text_identity.hpp"

namespace bsp {

// Fresh, DWORD-aligned native widget storage must cover at least +00..+E3.
// The EC-byte base region in derived producers also contains unwritten bytes;
// this routine does not initialize them or create a logical GuiWidgetOwner.
// The borrowed D7A24C word is loaded separately before and after allocation.
NativeGuiTextIdentityPrefix* construct_native_gui_widget_base_00aa9390(
    void* raw_storage, std::uint32_t type,
    const volatile std::uint32_t& actual_one_00d7a24c);

// Same 0Ch allocation and two self-link writes as existing A4C4A0. The last
// DWORD is untouched. The receiver passed by AA9390 is unused by A9B720.
void* allocate_native_gui_widget_list_sentinel_00a9b720();

// Complete AA6E10: stamp D5C104, tail BD30F0 (stamp CEB130). Neither the
// reference count nor any storage allocation is released. Used by the
// constructor's state-zero native unwind and by the source allocation catch.
void __fastcall destroy_native_gui_ref_base_00aa6e10(void*) noexcept;

} // namespace bsp
