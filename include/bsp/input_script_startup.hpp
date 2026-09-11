#pragma once

#include "bsp/input_settings.hpp"
#include "bsp/vfs_lua_scripts.hpp"
#include <memory>

namespace bsp {

// Normal-flow composition of006ab6b0 and006a7be0. Owns the settings and the
// persistent ControlPresets interpreter (native this+78h). New C++ interface;
// native singleton registration, container layout and allocator are not implied.
// The VFS/files/runtime/globals must outlive this object, including any DoFile
// calls made through the borrowed persistent state. Access is serialized.
class InputScriptStartup {
public:
    // Construct empty settings, clear the distinct native+4/+5 flags, and load
    // immediately, as006ab6b0 does before returning this. Script/schema failure
    // throws through the existing protected host boundary.
    InputScriptStartup(VfsLuaScriptFiles&, LuaScriptRuntime&, const LuaRuntimeGlobals&);
    ~InputScriptStartup();
    InputScriptStartup(const InputScriptStartup&) = delete;
    InputScriptStartup& operator=(const InputScriptStartup&) = delete;

    // Native+4 is set BEFORE either interpreter opens. Re-entry/repeated calls
    // return without clearing or reloading anything, including native+5.
    void load_data_tables_006a7be0();
    bool data_tables_started() const noexcept { return data_tables_started_; }
    InputSettings& settings() noexcept { return settings_; }
    const InputSettings& settings() const noexcept { return settings_; }
    // Borrowed until this object is destroyed. Callers must release any Lua
    // references before destruction; they must never close this interpreter.
    lua_State* control_presets_lua() noexcept;

private:
    VfsLuaScriptFiles& files_;
    LuaScriptRuntime& runtime_;
    const LuaRuntimeGlobals& globals_;
    bool data_tables_started_{}; // native+4, distinct from runtime_settings_loaded+5
    InputSettings settings_;
    std::unique_ptr<PcStorageLuaOwner> control_presets_;
};

} // namespace bsp
