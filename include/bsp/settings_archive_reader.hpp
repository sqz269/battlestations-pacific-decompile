#pragma once
#include "bsp/game_settings.hpp"
#include "bsp/gui_lua_reader.hpp"
#include <cstdint>

namespace bsp {
struct InputSettings;
struct KeyboardRuntimeHost;
struct SettingsArchiveReadHost {
    virtual ~SettingsArchiveReadHost() = default;
    // Mirror00f88980+B0 into0108ff20 and optionally issue the native Lua command.
    virtual void publish_xbox_compatibility_008d44c0(bool) = 0;
    virtual std::uint32_t renderer_shader_model_104_28() = 0;
};
// 008d6dc0: ECX=settings, one reader pointer, RET4. Defaults here differ from
// reset defaults. Uses real Lua fields and keyboard application, no native ABI.
void read_settings_archive_008d6dc0(GameSettingsBlock&, GuiLuaReader&,
    InputSettings&, KeyboardRuntimeHost&, SettingsArchiveReadHost&);
}
