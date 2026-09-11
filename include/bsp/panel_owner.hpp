#pragma once

#include "bsp/panel_palette.hpp"
#include "bsp/voice_playback.hpp"

namespace bsp {

// Unaddressed allocation adapter. Use a fresh, placement-constructed canonical
// owner; copy the five explicitly captured native allocation words before the
// recovered constructor. The owner must not move while its palette is alive.
void prepare_panel_owner_allocation(VoicePanelState&,
    const PanelOwnerAllocationWords&) noexcept;

//0044AB50: incoming ECX unused, no arguments, RET, EAX=24h allocation.
// Initialize only links0/4/8 and color20=1/isnil21=0. Payload and padding
// retain allocator bytes. Uses existing concrete CRT/new-handler transport.
void* allocate_panel_palette_sentinel_0044ab50();

//0044AC90: ECX=tree (forwarded, otherwise unused), node* stack, RET4.
// Full actual native subtree clear: right, capture left, free node, loop left.
void destroy_panel_palette_subtree_0044ac90(void* actual_tree, void* node) noexcept;

// Reached begin..end branches ONLY of the native RET14 range helpers.
//00452040 constructs both checked iterators from the same current owner/head,
// so validation and arbitrary range paths cannot be entered before teardown.
// Standard character/queue projections preserve payload/key order and the
// separately exposed queued count; they are not native tree-layout replacements.
void clear_panel_queue_full_range_004517e0(VoicePanelState&, NativeStringStorage&);
void clear_panel_palette_full_range_0044e150(void* actual_tree) noexcept;
void clear_panel_characters_full_range_0044f0a0(PanelCharacterMap&, NativeStringStorage&);

//00452660: ECX=fresh38h native owner, EAX=this, RET. Standard maps are already
// constructed by the host C++ allocation; actual palette construction is here.
// Preserves allocator words, default_pause_30 and field_34 supplied above.
VoicePanelState& construct_panel_owner_00452660(VoicePanelState&);
//00452040: ECX=owner, RET. Current string, queue, palette, character keys.
// Current NativeString header remains untouched after its storage is released.
void destroy_panel_owner_00452040(VoicePanelState&, NativeStringStorage&);
//00452720: ECX=owner, flags word stack, RET4. Always destroys owned content;
// bit0 additionally ends the host C++ lifetime and calls concrete CRT free.
// Return is the original address, even when it was freed. A deleting owner
// must be placement-constructed in singleton_lifetime_allocate storage.
VoicePanelState* scalar_delete_panel_owner_00452720(VoicePanelState*,
    std::uint32_t flags, NativeStringStorage&);

} // namespace bsp
