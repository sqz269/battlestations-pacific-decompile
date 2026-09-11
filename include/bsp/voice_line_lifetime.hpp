#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/voice_subtitles.hpp"

namespace bsp {

class VoiceLineLifetimeHost {
public:
    virtual ~VoiceLineLifetimeHost() = default;
    // Widget virtual +4, ECX=widget, flag on stack. The actual widget factory
    // owns its complete deleting destructor; releasing scene nodes alone is
    // insufficient. Native passes 1 and clears line+14 only after this returns.
    virtual void delete_widget_vslot_04(GuiWidgetTransform&, std::uint32_t flag) = 0;
    // Data accessor: resolve [[00E198C4]+A4] at this exact point, after widget
    // destruction. Do not return the manager captured before the callback.
    virtual VoicePlaybackManager& current_voice_manager_00e198c4_a4() = 0;
    // Native00BF65AC is the correctly named CRT _free thunk. Release the
    // matching allocation of this projected C++ VoiceLine, without rerunning
    // destroy_voice_line_005b9160. Its clip vector is already empty.
    virtual void free_line_storage_00bf65ac(VoiceLine*) noexcept = 0;
};

// Full005B9160..005B91DA, native ECX=line, RET. Resets native-vtable metadata,
// deletes the widget and marks the CURRENT manager dirty only when line+14
// was initially nonnull, then releases clip-vector storage. Borrowed records
// are not destroyed. Widget and manager calls may mutate line fields.
void destroy_voice_line_005b9160(VoiceLine&, VoiceLineLifetimeHost&);

// Full005B9B80..005B9B9D, native ECX=line, flags stack, RET4, EAX=original
// this even when freed. The correct CG_scalar_deleting_dtor identity is kept.
VoiceLine* scalar_delete_voice_line_005b9b80(VoiceLine*, std::uint8_t flags,
    VoiceLineLifetimeHost&);

// Full005BAF30..005BB01E, native ECX=fresh38h line, reader* stack, RET4,
// EAX=this. Canonical Lua serializer reads Handle "shortcut" then String
// "text", without defaults; subtitle construction always runs, then slot=-1.
// The native shortcut destination is seeded with the address of its stack
// field pair. This Win32 projection preserves that rule using its own pair's
// address. If conversion declines the value, those address bits survive.
// Caller supplies fresh projected line storage and real handle resolution.
// Other fields, including clip_index_1c, are not assigned constructor defaults.
VoiceLine& deserialize_voice_line_005baf30(VoiceLine&, GuiLuaReader&,
    GuiLuaHandleResolver&, VoiceSubtitleContext&, NativeStringStorage&);

} // namespace bsp
