#pragma once
#include "bsp/gui_resources.hpp"
#include "bsp/gui_widget_frame_runtime.hpp"

namespace bsp {
// AA3910's pointer-device, globals, movement, hit-test and cursor-state body
// remains an explicit required provider. It receives this same resource owner.
// No update callback or successful fallback is supplied for a missing owner.
struct GuiManagerPointerCalls {
    virtual ~GuiManagerPointerCalls() = default;
    virtual void update_pointer_00aa3910(GuiResourceOwner&) = 0;
};
struct GuiManagerFrameServices {
    GuiWidgetFrameRuntime& frames;
    const GuiTimedEntryConstants& timed;
    GuiManagerPointerCalls& pointer;
    const volatile float& hidden_position_00d7a260;
};
// AA7EF0 reads the actual+88/+8C timed-entry list; count is captured once.
// The storage owner is shared with widget AA87B0. No timed entry is advanced.
bool gui_widget_has_live_entries_00aa7ef0(GuiWidgetOwner&,
    const GuiTimedEntryConstants&);
// AA0F70: hide/reset current manager74, then fresh manager78. Slots and the
// negative-position constant are reloaded at the native callback boundaries.
void reset_gui_manager_highlights_00aa0f70(GuiResourceOwner&,
    GuiWidgetOwnerRuntime&, const volatile float& hidden_position_00d7a260);
// AA4F80: ECX manager, delta/blocked-byte in two stack DWORDs, RET8. Publishes
// raw70, optionally updates pointer, resets highlights, copies actual page
// pointers, and dispatches visible/live pages. Clears70 only on normal exit.
// Snapshot allocation/checked-STL errors and original EH/ABI are not ported.
// The same manager, page roots and owners must remain alive through callbacks;
// registry insertions after snapshot do not join the current frame.
void update_gui_manager_00aa4f80(GuiResourceOwner&, float seconds,
    std::uint8_t blocked, const GuiManagerFrameServices&);
} // namespace bsp
