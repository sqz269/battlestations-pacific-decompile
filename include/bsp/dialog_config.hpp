#pragma once

#include "bsp/lua_state_owner.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/panel_palette.hpp"
#include "bsp/panel_publication_types.hpp"
#include "bsp/voice_playback.hpp"

namespace bsp {
// Borrow fields of the SAME panel owner at game+21E4. Character map is the
// existing standard-container projection; palette is an actual initialized
// Win32 tree. This routine updates both maps without clearing prior entries.
struct DialogConfigView {
    PanelCharacterMap& characters_04;
    void* palette_10;
    float& default_pause_30;
    std::uint32_t& state_34;
};
inline DialogConfigView voice_panel_dialog_config(VoicePanelState& panel) noexcept {
    return {panel.characters_04, panel.palette_10, panel.default_pause_30, panel.field_34};
}
struct DialogConfigContext {
    NativeStringStorage& strings;
    const PanelPaletteValueWords& missing_palette_words;
    const SingletonLifetimeCallbacks& validation;
    const bool& crt_sse2_conversion;
};

// Complete0044FA30 sequence: ECX=panel owner, RET. Creates a temporary Lua
// owner, opens base only(mask1), runs DialogGlobals.lua plus VFS overrides,
// reads live tables in native order, sets state0, then closes. The existing
// owner/runtime transport native fatal errors as C++ exceptions; no SEH/ABI
// or interpreter allocation/stack-layout equivalence is claimed.
void load_dialog_config_0044fa30(DialogConfigView, LuaStateOwnerEnvironment,
    LuaScriptRuntime&, DialogConfigContext&);
} // namespace bsp
