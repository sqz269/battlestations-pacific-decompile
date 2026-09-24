#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct NativeNodeStorage;

// Borrow the same actual string pool and the original current constant words.
// Native values are 0.1f, 1000.0f and 1.0f; preserve their bits/read timing.
struct NativeGuiPageConstructionContext {
    ActualNativeStringPoolStorage& strings;
    const volatile std::uint32_t& actual_00d7a2f0;
    const volatile std::uint32_t& actual_00ce3804;
    const volatile std::uint32_t& actual_one_00d7a24c;
};

// Retained construction metadata, never an owner/refcount or a finished page.
// No destructor performs native cleanup. The caller owns the same raw 124h
// page allocation (128h pool slot) and all outstanding constructor resources.
struct NativeGuiPageConstructionAcquired {
    enum class Phase { not_started, running, awaiting_lua_continuation, failed };
    Phase phase{Phase::not_started};
    void* actual_page{};
    const void* actual_name{};
    NativeNodeStorage* actual_scene_node{};
    std::uint32_t share_scene_slot{};
    std::uint32_t native_site{};
    std::uint32_t continuation{};
    std::int32_t native_eh_state{-1};
    bool base_returned{}, name_returned{}, scene_bound{};
};

// Complete AA6720..AA6734 (21 bytes). Native ECX raw widget, stack raw node,
// RET4. Store actual node at widget+4C, then clear its current flags138 bits0/1
// when nonnull. No retain/release or logical GuiWidgetOwner is involved.
void bind_native_gui_widget_node_00aa6720(void* actual_widget,
    NativeNodeStorage* actual_node) noexcept;

// PARTIAL AC6600: normal prefix AC6600..AC6713 only. Invoke actual AA9390(type1),
// construct page name/defaults, and bind the actual node. On return the frame
// is awaiting_lua_continuation at AC6714 with native EH state1; this is NOT a
// successfully constructed page and must not be published or treated as one.
// Original full ABI: ECX raw page, stack(name,node,share_scene_low_byte), RET0C.
// +E4/+E8, page padding and the pool's +124 ID retain their existing bytes.
//
// Unimplemented continuation AC6714..AC6878 constructs/opens Lua, executes the
// scripts, builds the raw reader, visits properties, propagates the node root
// and tears down temporaries. Native failure from state0 needs full AA9730;
// from state1 it first needs 41DD20(page+100). That raw destructor is still a
// reconstruction frontier, so this prefix RETAINS state/resources on C++ error
// and reports failed. It does not claim native unwind parity or add rollback.
// Do not return its pool slot while name/list/node obligations remain live.
// AA9390's own failure cleanup remains the existing provider's responsibility.
void initialize_native_gui_page_00ac6600_prefix(void* actual_page,
    const void* actual_name_header, NativeNodeStorage* actual_scene_node,
    std::uint32_t share_scene_slot, NativeGuiPageConstructionContext&,
    NativeGuiPageConstructionAcquired&);

} // namespace bsp
